#!/bin/sh
#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#
# Print the Bp* method signatures exported by the stock private HAL interface
# libraries. Point BLOBS at an unpacked vendor image, e.g.
#   BLOBS=/home/hema/Workspace/e5/vendor tools/dump_hal_api.sh

BLOBS="${BLOBS:-/home/hema/Workspace/e5/vendor}"

find "$BLOBS" -type f \( -name 'vendor.sprd.hardware*@*.so' \
     -o -name 'vendor.sprd.hardware*-V1-ndk.so' \
     -o -name 'vendor.unisoc.hardware*-V1-ndk.so' \) \
     -path '*/lib64/*' | sort -u | while read -r lib; do
    printf '### %s\n' "$(basename "$lib")"
    nm -D --defined-only "$lib" 2>/dev/null | c++filt | \
        sed -nE 's/^.*::Bp(Hw)?([A-Za-z0-9_]+)::([A-Za-z0-9_]+)\((.*)$/\2::\3(\4/p' | \
        grep -vE '::(Bp|~|_hidl_|getInterfaceVersion|getInterfaceHash)' | sort -u
done
