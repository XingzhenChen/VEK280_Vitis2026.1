# VEK280 最小工程

Vitis 2026.1 / EDF，Yocto 路径固定为 `/home/xingzhen/YOCTO_ARTIFACTS`。

```bash
source env.sh
make aie           # 编译 AIE
make kernels       # 编译 PL
make host          # 编译 AArch64 host
make sim           # AIE 仿真
make link          # 链接 XSA
make               # 编译并打包 hw_emu
make run           # QEMU 硬件仿真
make TARGET=hw     # 编译并打包硬件版本
make clean         # 删除整个 build/，保留源码和输入数据
```

直接修改 `src/`；矩阵尺寸和迭代次数在 `src/config.h`，PL / AIE 连接在 `system.cfg`。Makefile 不传递算法参数。

HLS 只综合一个 `top`：`src/pl/top.cpp` 定义接口及 DATAFLOW，`mm2s.cpp` 和 `s2mm.cpp` 是内部子模块。Makefile 将 `src/pl/` 下全部 `.cpp` 编译成一个 `top.xo`，链接时只实例化 `top_1`，host 只启动一次该核。新增子模块文件放在同一目录即可。

`data/` 包含当前 64×64 矩阵、4 次迭代的输入和参考结果。更换算法或尺寸时自行修改数据。AIE 仿真输出位于 `build/hw_emu/sim/data/`。

板端运行文件在 `build/hw/runtime/`，复制整个目录到 VEK280 后执行 `sudo ./run.sh`。`make TARGET=hw` 生成运行文件包，不烧写 SD 卡。

源码已纳入本工程，构建不复制外部示例。基线源自 AMD Vitis AIE-ML system design 示例和 VEK280 EDF 教程，许可证见 `LICENSES/`。
