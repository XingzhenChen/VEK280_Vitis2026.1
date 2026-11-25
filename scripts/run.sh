#!/bin/bash
# Copyright (C) 2026, Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
export PATH="$PATH:/sbin:/usr/sbin"
export XILINX_XRT=/usr
if [ "${XCL_EMULATION_MODE:-}" = hw_emu ]; then
    fpgautil -o vek280.dtbo
else
    fpgautil -b krnl_aieml.pdi -o vek280.dtbo
fi
exec ./aieml_system krnl_aieml.xclbin
