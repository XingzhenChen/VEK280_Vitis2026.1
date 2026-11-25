# Copyright (C) 2026, Advanced Micro Devices, Inc. All rights reserved.
SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c
.DEFAULT_GOAL := all
.DELETE_ON_ERROR:

ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
TARGET ?= hw_emu
BUILD := $(ROOT)/build/$(TARGET)
PLATFORM := $(XILINX_VITIS)/base_platforms/vek280_base/vek280_base.xpfm
export YOCTO_ARTIFACTS := /home/xingzhen/YOCTO_ARTIFACTS
SDK := $(YOCTO_ARTIFACTS)/amd-cortexa72-common_meta-edf-app-sdk/sdk
SYSROOT := $(SDK)/sysroots/cortexa72-cortexa53-amd-linux
CXX := $(SDK)/sysroots/x86_64-amdedfsdk-linux/usr/bin/aarch64-amd-linux/aarch64-amd-linux-g++
KERNEL := $(BUILD)/top.xo
PL_SOURCES := $(wildcard $(ROOT)/src/pl/*.cpp)

.PHONY: all aie kernels host link sim package run clean
all: package
aie: $(BUILD)/libadf.a
kernels: $(KERNEL)
host: $(BUILD)/aieml_system
link: $(BUILD)/aieml.xsa

$(BUILD):
	mkdir -p "$@"

$(BUILD)/data: | $(BUILD)
	ln -s "$(ROOT)/data" "$@"

$(BUILD)/libadf.a: $(wildcard $(ROOT)/src/aie/*) $(ROOT)/src/config.h $(ROOT)/Makefile | $(BUILD)/data
	cd "$(BUILD)" && v++ --compile --mode aie --platform "$(PLATFORM)" \
		--include "$(ROOT)/src/aie" --aie.workdir Work \
		--aie.output-archive "$@" "$(ROOT)/src/aie/graph.cpp"

$(KERNEL): $(PL_SOURCES) $(wildcard $(ROOT)/src/pl/*.h $(ROOT)/src/pl/*.hpp) $(ROOT)/src/config.h $(ROOT)/Makefile | $(BUILD)
	cd "$(BUILD)" && v++ --compile -t $(TARGET) --platform "$(PLATFORM)" \
		--temp_dir _x/top --report_dir reports/top -k top $(PL_SOURCES) -o "$@"

$(BUILD)/aieml_system: $(ROOT)/src/host/host.cpp $(ROOT)/src/config.h $(ROOT)/Makefile | $(BUILD)
	$(CXX) --sysroot="$(SYSROOT)" -std=c++17 -O2 -Wall -Wextra \
		-I"$(SYSROOT)/usr/include/xrt" "$<" -L"$(SYSROOT)/usr/lib" \
		-lxrt_coreutil -lpthread -lrt -ldl -o "$@"

$(BUILD)/aieml.xsa: $(KERNEL) $(BUILD)/libadf.a $(ROOT)/system.cfg $(ROOT)/Makefile
	cd "$(BUILD)" && v++ --link -t $(TARGET) --platform "$(PLATFORM)" \
		--temp_dir _x/link --config "$(ROOT)/system.cfg" \
		"$(KERNEL)" "$(BUILD)/libadf.a" -o "$@"

sim: $(BUILD)/libadf.a
	cd "$(BUILD)" && aiesimulator --pkg-dir Work --output-dir sim

$(BUILD)/krnl_aieml.xclbin: $(BUILD)/aieml.xsa $(BUILD)/libadf.a $(ROOT)/Makefile
	cd "$(BUILD)" && v++ --package -t $(TARGET) --platform "$(PLATFORM)" \
		--package.boot_mode sd --package.no_image --package.defer_aie_run \
		--package.out_dir package "$(BUILD)/aieml.xsa" "$(BUILD)/libadf.a" -o "$@"

package: $(BUILD)/krnl_aieml.xclbin $(BUILD)/aieml_system
	{ printf '/dts-v1/; /plugin/;\n'; cat "$(BUILD)/_x/package/updated_zocl.dtsi"; } > "$(BUILD)/vek280.dts"
	dtc -@ -I dts -O dtb -o "$(BUILD)/vek280.dtbo" "$(BUILD)/vek280.dts"
	mkdir -p "$(BUILD)/runtime"
	cp "$(BUILD)/aieml_system" "$(BUILD)/krnl_aieml.xclbin" "$(BUILD)/vek280.dtbo" "$(BUILD)/runtime/"
	cp "$(BUILD)"/package/*.pdi "$(BUILD)/runtime/krnl_aieml.pdi"
	cp -r "$(ROOT)/data" "$(BUILD)/runtime/"
	install -m 755 "$(ROOT)/scripts/run.sh" "$(BUILD)/runtime/run.sh"

run: package
	@test "$(TARGET)" = hw_emu
	cd "$(BUILD)" && emconfigutil --platform "$(PLATFORM)" --od runtime --nd 1
	printf 'AIE_PKG_DIR=%s/Work\n' "$(BUILD)" > "$(BUILD)/aiesim_options.txt"
	tar -cf "$(BUILD)/runtime.tar" -C "$(BUILD)/runtime" .
	bash "$(ROOT)/scripts/prepare_qemu.sh" "$(BUILD)"
	cd "$(BUILD)" && ./package/launch_hw_emu.sh \
		-qemu-config "$(BUILD)/qemu/combined.qemuboot.conf" \
		-login amd-edf -password amd-edf -aie-sim-options "$(BUILD)/aiesim_options.txt" \
		-run-app 'mkdir -p /tmp/vek280 && tar -xf /runtime.tar -C /tmp/vek280 && cd /tmp/vek280 && XCL_EMULATION_MODE=hw_emu ./run.sh' \
		2>&1 | tee run.log

clean:
	rm -rf -- "$(ROOT)/build"
