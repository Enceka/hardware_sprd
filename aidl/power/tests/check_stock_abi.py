#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
"""Compare the recovered AIDL transaction order/flags with E5's OEM proxy."""
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[5]
POWER = ROOT / 'hardware/sprd/aidl/power'
BLOB = ROOT / 'vendor/rongyue/e5/proprietary/vendor/lib64/vendor.unisoc.hardware.power-V1-ndk.so'
OBJDUMP = ROOT / 'prebuilts/clang/host/linux-x86/clang-r450784d/bin/llvm-objdump'
source = (POWER / 'compat/vendor/unisoc/hardware/power/IVendorPower.aidl').read_text()
methods = re.findall(r'^\s*(?:(oneway)\s+)?(?:void|int)\s+(\w+)\(', source, re.M)
expected = {name: (i, bool(oneway)) for i, (oneway, name) in enumerate(methods, 1)}
assert len(expected) == 9
assembly = subprocess.check_output([str(OBJDUMP), '-d', '--demangle', str(BLOB)], text=True)
found = {}
for block in re.split(r'(?=^[0-9a-f]+ <)', assembly, flags=re.M):
    header = block.split('\n', 1)[0]
    match = re.search(r'::BpVendorPower::(\w+)\(', header)
    if not match or match[1] not in expected:
        continue
    before, separator, _ = block.partition('<AIBinder_transact@plt>')
    assert separator, header
    code = int(re.findall(r'\bmov\s+w1, #(\d+)', before)[-1])
    flags = int(re.findall(r'\bmov\s+w4, #(\d+)', before)[-1])
    for value in re.findall(r'\bmovk\s+w4, #(\d+), lsl #16', before):
        flags = (flags & 0xffff) | (int(value) << 16)
    found[match[1]] = (code, bool(flags & 1))
assert found == expected, (found, expected)
hashes = set(re.findall(rb'(?<![0-9a-f])[0-9a-f]{40}(?![0-9a-f])', BLOB.read_bytes()))
assert b'67a6288e6ac1c48aa2d0f1c882d182d74ad6f61b' in hashes
assert '67a6288e6ac1c48aa2d0f1c882d182d74ad6f61b' in (POWER / 'VendorPower.cpp').read_text()
print('PASS: all nine OEM transaction IDs, oneway flags and V1 interface hash')
