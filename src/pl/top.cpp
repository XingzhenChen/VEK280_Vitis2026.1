/*
Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: X11
*/
#include "top.h"
#include "../config.h"

extern "C" void top(const Word* a, const Word* b, Word* c32, Word* c16,
                    Axis& a1, Axis& b1, Axis& a2, Axis& b2,
                    Axis& out32, Axis& out16) {
#pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem_a
#pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem_b
#pragma HLS INTERFACE m_axi port=c32 offset=slave bundle=gmem_c32
#pragma HLS INTERFACE m_axi port=c16 offset=slave bundle=gmem_c16
#pragma HLS INTERFACE s_axilite port=a bundle=control
#pragma HLS INTERFACE s_axilite port=b bundle=control
#pragma HLS INTERFACE s_axilite port=c32 bundle=control
#pragma HLS INTERFACE s_axilite port=c16 bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control
#pragma HLS INTERFACE axis port=a1
#pragma HLS INTERFACE axis port=b1
#pragma HLS INTERFACE axis port=a2
#pragma HLS INTERFACE axis port=b2
#pragma HLS INTERFACE axis port=out32
#pragma HLS INTERFACE axis port=out16
#pragma HLS DATAFLOW
    mm2s(a, a1, a2, NITERATIONS * A_ROWS * A_COLS / 16, A_ROWS * A_COLS / 16);
    mm2s(b, b1, b2, NITERATIONS * B_ROWS * B_COLS / 16, B_ROWS * B_COLS / 16);
    s2mm(c32, out32, NITERATIONS * C_ROWS * C_COLS / 4);
    s2mm(c16, out16, NITERATIONS * C_ROWS * C_COLS / 8);
}
