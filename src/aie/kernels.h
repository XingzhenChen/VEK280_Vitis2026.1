/*
Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
*/
#pragma once
#include <adf.h>
#include "aie_api/aie.hpp"
#include "../config.h"

template<typename ITYPE, typename OTYPE, int SHIFT_RESULT>
void ClassicMatMult(
    adf::input_buffer<ITYPE, adf::extents<adf::inherited_extent, adf::inherited_extent>>& __restrict inA,
    adf::input_buffer<ITYPE, adf::extents<adf::inherited_extent, adf::inherited_extent>>& __restrict inB,
    adf::output_buffer<OTYPE, adf::extents<adf::inherited_extent, adf::inherited_extent>>& __restrict outC);
