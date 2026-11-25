/*
Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
*/
#pragma once
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

using Word = ap_int<128>;
using Axis = hls::stream<ap_axis<128, 0, 0, 0>>;

void mm2s(const Word* mem, Axis& first, Axis& second, int words, int packet_words);
void s2mm(Word* mem, Axis& stream, int words);

extern "C" void top(const Word* a, const Word* b, Word* c32, Word* c16,
                    Axis& a1, Axis& b1, Axis& a2, Axis& b2,
                    Axis& out32, Axis& out16);
