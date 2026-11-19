#!/usr/bin/env python3
"""Repair arduino-cli tool installs under proot.

Under proot's link2symlink extension (Termux/proot distros), hard links are
emulated with .l2s symlinks. arduino-cli extracts tool archives using hard
links between a temp staging dir and the install dir; when the temp dir is
cleaned up the installed binaries become dangling symlinks.

This script re-extracts the cached tool tarballs from
~/.arduino15/staging/packages, dereferencing hardlinks into plain files, and
rebuilds any tool directory that contains broken symlinks.

Usage: python3 tools/fix_arduino15_links.py
"""
import os
import sys
import tarfile

arduino15 = os.path.expanduser("~/.arduino15")
packages = os.path.join(arduino15, "packages")
staging = os.path.join(arduino15, "staging", "packages")


def broken_tool_dirs():
    """Find .../tools/<name>/<version> dirs containing dangling symlinks."""
    broken = {}
    for root, dirs, files in os.walk(packages):
        rel = os.path.relpath(root, packages).split(os.sep)
        if len(rel) < 4 or rel[1] != "tools":
            continue
        for f in files + dirs:
            p = os.path.join(root, f)
            if os.path.islink(p) and not os.path.exists(p):
                broken[(rel[2], rel[3])] = os.path.join(packages, *rel[:4])
                break
    return broken


def tar_members(tf):
    tops = set()
    for m in tf.getmembers():
        name = m.name.removeprefix("./")
        tops.add(name.split("/")[0])
    return tops


def extract_to(tf, dest, strip_top):
    count = 0
    for m in tf.getmembers():
        name = m.name.removeprefix("./")
        if strip_top:
            top, _, rest = name.partition("/")
            if not rest:
                continue  # the stripped top-level directory itself
            name = rest
        if not name:
            continue
        path = os.path.join(dest, name)
        if m.isdir():
            os.makedirs(path, exist_ok=True)
        elif m.issym():
            os.makedirs(os.path.dirname(path), exist_ok=True)
            if os.path.lexists(path):
                os.remove(path)
            os.symlink(m.linkname, path)
        elif m.islnk() or m.isfile():
            os.makedirs(os.path.dirname(path), exist_ok=True)
            if os.path.lexists(path):
                os.remove(path)
            f = tf.extractfile(m)
            if f is None:
                continue
            with open(path, "wb") as out:
                out.write(f.read())
            os.chmod(path, m.mode & 0o7777)
            count += 1
    return count


def main():
    broken = broken_tool_dirs()
    if not broken:
        print("No broken tool installs found.")
        return 0

    tarballs = sorted(os.listdir(staging))
    for (name, version), dirpath in sorted(broken.items()):
        match = None
        for tb in tarballs:
            if tb.startswith(f"{name}-{version}") or tb.startswith(f"{name}_{version}") \
               or tb.startswith(f"{name}-{version}".replace(".", "_")):
                match = tb
                break
        if not match:
            print(f"NO CACHED TARBALL for {name} {version}; reinstall with arduino-cli core/tools", file=sys.stderr)
            continue
        src = os.path.join(staging, match)
        print(f"Re-extracting {match} -> {dirpath}")
        # proot re-creates l2s indirection for dangling links; remove the
        # dangling ones first so plain files can replace them.
        for root, dirs, files in os.walk(dirpath):
            for f in files + dirs:
                p = os.path.join(root, f)
                if os.path.islink(p) and not os.path.exists(p):
                    try:
                        os.remove(p)
                    except OSError:
                        pass
        with tarfile.open(src) as tf:
            tops = tar_members(tf)
            strip_top = len(tops) == 1
            n = extract_to(tf, dirpath, strip_top)
        print(f"  wrote {n} files")
    return 0


if __name__ == "__main__":
    sys.exit(main())
