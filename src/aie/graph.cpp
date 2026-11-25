/*
Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: X11
*/
#include "MultGraph.h"

TestMatMult GMatMult;

#if defined(__AIESIM__) || defined(__X86SIM__)
int main() {
    GMatMult.init();
    GMatMult.run(NITERATIONS);
    GMatMult.end();
    return 0;
}
#endif
