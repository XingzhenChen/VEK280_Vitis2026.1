# Minimal VEK280 Project

Vitis 2026.1 / EDF. Yocto artifacts are located at `/home/xingzhen/YOCTO_ARTIFACTS`.

```bash
source env.sh
make aie           # Compile the AIE graph
make kernels       # Compile the HLS top
make host          # Compile the AArch64 host
make sim           # Run AIE simulation
make link          # Link the XSA
make               # Build and package hw_emu
make run           # Run hardware emulation in QEMU
make TARGET=hw     # Build and package for hardware
make clean         # Remove build/ and preserve sources and input data
```

Edit `src/` directly. Matrix dimensions and the iteration count are defined in `src/config.h`; PL/AIE connections are defined in `system.cfg`. The Makefile does not pass algorithm parameters.

HLS synthesizes a single `top`. Its interfaces and DATAFLOW region are defined in `src/pl/top.cpp`; `mm2s.cpp` and `s2mm.cpp` implement internal submodules. The Makefile compiles all `.cpp` files in `src/pl/` into one `top.xo`. Linking creates only `top_1`, and the host launches that kernel once. Add new submodule files to the same directory.

`data/` contains inputs and reference results for 64x64 matrices and four iterations. Update these files when changing the algorithm or dimensions. AIE simulation outputs are written to `build/hw_emu/sim/data/`.

Hardware runtime files are placed in `build/hw/runtime/`. Copy the entire directory to the VEK280 and run `sudo ./run.sh` from that directory. `make TARGET=hw` generates the runtime package; it does not flash an SD card.

All design sources are included in this project; the build does not copy external examples. The baseline is derived from the AMD Vitis AIE-ML system design example and the VEK280 EDF tutorial.
