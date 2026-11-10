#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Check ELF dependency presence in an unpacked Android product (including APEXes).

This checks DT_NEEDED recursively and ELF bitness/machine, not linker namespace
allowlists, symbol/struct ABI compatibility, dlopen targets, or runtime data.
Pass both 32-bit and 64-bit HAL implementations as roots to cover both clients.
The input must contain extracted APEX directories (PRODUCT_OUT/apex) or
flattened APEXes (system/apex); compressed .apex files alone are not inspected.
"""

import argparse
from collections import deque
from functools import lru_cache
import json
from pathlib import Path
import re
import subprocess


class DependencyChecker:
    def __init__(self, root, readelf="readelf"):
        self.root = Path(root).resolve()
        self.readelf = readelf

    def local_path(self, relative):
        """Resolve Android absolute symlinks inside the image, not on the host."""
        parts = list(Path(relative).parts)
        if not parts or Path(relative).is_absolute() or ".." in parts:
            raise ValueError("expected an image-relative path: " + relative)
        current = self.root
        links = 0
        while parts:
            current /= parts.pop(0)
            if current.is_symlink():
                links += 1
                if links > 40:
                    raise ValueError("symlink loop: " + relative)
                target = current.readlink()
                if target.is_absolute():
                    current = self.root
                    target = Path(*target.parts[1:])
                else:
                    current = current.parent
                parts = list(target.parts) + parts
            # Collapse .. from relative symlinks, while staying in the image.
            if current.name == "..":
                current = current.parent.parent
            if not current.is_relative_to(self.root):
                raise ValueError("symlink escapes image: " + relative)
        return current

    @lru_cache(maxsize=None)
    def elf(self, relative):
        path = self.local_path(relative)
        try:
            with path.open("rb") as stream:
                header = stream.read(20)
        except (OSError, ValueError):
            return None
        if len(header) < 20 or header[:4] != b"\x7fELF":
            return None
        endian = "little" if header[5] == 1 else "big"
        machine = int.from_bytes(header[18:20], endian)
        result = subprocess.run([self.readelf, "-d", str(path)],
                                check=True, capture_output=True, text=True)
        return (header[4], machine), re.findall(r"\(NEEDED\).*?\[(.*?)\]", result.stdout)

    def directories(self, arch):
        lib = "lib64" if arch[0] == 2 else "lib"
        # Presence candidates only: system libraries are not necessarily
        # visible from the vendor namespace. Only VNDK/runtime APEXes apply.
        dirs = [f"{part}/{lib}" for part in ("odm", "vendor")]
        for base in ("apex", "system/apex", "system_ext/apex", "vendor/apex"):
            dirs += [f"{base}/{p.name}/{lib}" for p in sorted(
                (self.root / base).glob("com.android.vndk.*")) if p.is_dir()]
        dirs += [f"{base}/com.android.runtime/{lib}/bionic"
                 for base in ("apex", "system/apex")]
        dirs.append(f"system/{lib}")
        return dirs

    def resolve(self, name, arch):
        for directory in self.directories(arch):
            relative = f"{directory}/{name}"
            info = self.elf(relative)
            if info is not None and info[0] == arch:
                return relative
        return None

    def check(self, seeds):
        queue = deque(seeds)
        visited = set()
        elf_files = 0
        missing = []
        edges = []
        while queue:
            relative = queue.popleft()
            if relative in visited:
                continue
            visited.add(relative)
            info = self.elf(relative)
            if info is None:
                missing.append({"consumer": relative, "needed": "<missing/non-ELF root>"})
                continue
            elf_files += 1
            arch, needed = info
            for name in needed:
                provider = self.resolve(name, arch)
                edges.append({"consumer": relative, "needed": name, "provider": provider})
                if provider:
                    queue.append(provider)
                else:
                    missing.append({"consumer": relative, "needed": name})
        return {"root": str(self.root), "elf_files": elf_files,
                "edges": edges, "missing": missing}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--readelf", default="readelf")
    parser.add_argument("--json", type=Path, help="write a machine-readable report")
    parser.add_argument("files", nargs="+", help="image-relative ELF paths")
    args = parser.parse_args()
    if not args.root.is_dir():
        parser.error("image root does not exist")
    try:
        report = DependencyChecker(args.root, args.readelf).check(args.files)
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        parser.exit(2, f"dependency scan failed: {error}\n")
    if args.json:
        args.json.write_text(json.dumps(report, indent=2) + "\n")
    for entry in report["missing"]:
        print(f"MISSING {entry['consumer']} -> {entry['needed']}")
    print(f"Checked {report['elf_files']} ELF files; {len(report['missing'])} missing dependencies")
    print("Presence check only; runtime linker namespace and ABI checks still required.")
    return int(bool(report["missing"]))


if __name__ == "__main__":
    raise SystemExit(main())
