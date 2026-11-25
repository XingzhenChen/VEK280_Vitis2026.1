#!/bin/bash
# SPDX-License-Identifier: MIT
set -euo pipefail
build=$1
prebuilt=$YOCTO_ARTIFACTS/amd-cortexa72-common_vek280_qemu_prebuilt
destination=$build/qemu
wic=edf-platform-disk-image-amd-cortexa72-common.rootfs.wic.qemu-sd

test -s "$build/package/BOOT.bin"
mkdir -p "$destination"
for input in "$prebuilt"/*; do
    case "$input" in *.wic.qemu-sd) continue ;; esac
    cp -a --reflink=auto "$input" "$destination/"
done
# Replace local symlinks before writing to keep the Yocto artifacts untouched.
rm -f "$destination/$wic" "$destination/BOOT-versal-vek280-multidomain.bin" "$destination/boot.bin"
cp --reflink=auto --sparse=always "$YOCTO_ARTIFACTS/amd-cortexa72-common_edf-platform-disk-image/$wic" "$destination/$wic"
cp "$build/package/BOOT.bin" "$destination/BOOT-versal-vek280-multidomain.bin"
ln -s BOOT-versal-vek280-multidomain.bin "$destination/boot.bin"
for input in "$prebuilt"/*.wic.qemu-sd; do
    name=$(basename "$input")
    if [ "$name" != "$wic" ]; then
        ln -sfn "$wic" "$destination/$name"
    fi
done

unset LD_LIBRARY_PATH
set +u
source "$XILINX_VITIS/data/emulation/qemu/comp/qemu_edf/environment-setup-x86_64-amdedfsdk-linux"
set -u
wic cp --sector-size=512 "$destination/boot.bin" "$destination/$wic:1/"
wic cp --sector-size=512 "$build/runtime.tar" "$destination/$wic:2/"
