/*
Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: X11
*/
#include "top.h"

void s2mm(Word* mem, Axis& stream, int words) {
#pragma HLS INLINE off
    for (int i = 0; i < words; ++i) {
#pragma HLS PIPELINE II=1
        mem[i] = stream.read().data;
    }
}
