/*
Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: X11
*/
#include "kernels.h"

template<typename ITYPE, typename OTYPE, int SHIFT_RESULT>
void ClassicMatMult(
    adf::input_buffer<ITYPE, adf::extents<adf::inherited_extent, adf::inherited_extent>>& __restrict inA,
    adf::input_buffer<ITYPE, adf::extents<adf::inherited_extent, adf::inherited_extent>>& __restrict inB,
    adf::output_buffer<OTYPE, adf::extents<adf::inherited_extent, adf::inherited_extent>>& __restrict outC) {
    aie::set_rounding(aie::rounding_mode::floor);
    aie::set_saturation(aie::saturation_mode::saturate);
    auto pA = aie::begin_vector<ATILES_ROWS * ATILES_COLS>(inA);
    auto pB = aie::begin_vector<BTILES_ROWS * BTILES_COLS>(inB);
    auto pC = aie::begin_vector<CTILES_ROWS * CTILES_COLS>(outC);
    aie::mmul<ATILES_ROWS, ATILES_COLS, CTILES_COLS, ITYPE, ITYPE, acc32> tile;

    for (int row = 0; row < C_ROWS / CTILES_ROWS; ++row) {
        for (int col = 0; col < C_COLS / CTILES_COLS; ++col)
        chess_prepare_for_pipelining
        {
            tile.mul(*pA++, *pB++);
            for (int k = 1; k < A_COLS / ATILES_COLS; ++k)
            chess_flatten_loop
            {
                tile.mac(*pA++, *pB++);
            }
            *pC++ = tile.template to_vector<OTYPE>(SHIFT_RESULT);
            pA -= A_COLS / ATILES_COLS;
        }
        pA += A_COLS / ATILES_COLS;
        pB -= (B_ROWS / BTILES_ROWS) * (B_COLS / BTILES_COLS);
    }
}
