/*
Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
*/
#include "top.h"

void mm2s(const Word* mem, Axis& first, Axis& second, int words, int packet_words) {
#pragma HLS INLINE off
    int packet_word = 0;
    for (int i = 0; i < words; ++i) {
#pragma HLS PIPELINE II=1
        ap_axis<128, 0, 0, 0> word;
        word.data = mem[i];
        word.keep = -1;
        word.strb = -1;
        word.last = (packet_word == packet_words - 1);
        first.write(word);
        second.write(word);
        packet_word = word.last ? 0 : packet_word + 1;
    }
}
