# riscv-sim 立项书

| 项 | 内容 |
|---|---|
| 项目名称 | 基于 C 语言的 RV32I 五级流水线处理器模拟器及 Cache 性能分析 |
| 项目代号 | riscv-sim |
| 文档版本 | 立项书 v1.0（与计划书 v1.0 同步冻结，2026-09-06） |
| 项目周期 | 2026-09-10 ~ 2026-11-30（12 月冻结；stretch 窗口 2027-01-01 ~ 01-15） |
| 关联文档 | [riscv-sim-plan.md](./riscv-sim-plan.md) 实施规划 ｜ [week0-preparation.md](./week0-preparation.md) 准备清单 |

---

## 1. 项目概述

### 1.1 一句话定位

用 C 语言从零实现一个**周期精确（Cycle-Accurate）**的 RV32I 五级流水线 CPU 模拟器：内置单周期参考模型（Golden Model）逐条指令自动对照以保证正确性，配套直接映射 L1 D-Cache 模型与完整的性能实验体系，最终产出可复现的实验数据与实验报告。

### 1.2 立项背景与目的

- **学习目的**：把《计算机组成与设计》（P&H，RISC-V 版）第 2、4、5 章的 ISA、流水线、存储层次从"读过"变成"做过"，理解 ISA 与硬件实现、冒险成因、Cache 与程序性能的关系。
- **工程目的**：完整经历"规格冻结 → 模块化实现 → 自动化验证 → 定量实验 → 报告撰写"的工程闭环，public 仓库的小步提交历史本身就是过程证明。
- **应用目的**：形成一份可用于研究生复试的体系结构项目材料：代码、数据、图表与问答储备。

### 1.3 范围边界

**v1 范围内**：五级流水线（IF→ID→EX→MEM→WB）、数据冒险（转发 + load-use stall）、控制冒险（EX 级解析 + Predict-Not-Taken + 冲刷）、直接映射 L1 D-Cache（WT + WA，阻塞式 miss）、性能统计（CPI/IPC 四项分解）、hex/bin 双格式加载、CSV 导出与 Matplotlib 绘图。

**明确不做（防蔓延清单）**：ELF 解析器、I-Cache、TLB/虚存、L2、N 路相联 + LRU、动态分支预测器、ID 级分支解析、GUI、mini-assembler（降为第三阶段）、多核 / 中断 / CSR。以上均列入 2027 年 1 月 stretch 窗口。

---

## 2. 实现平台

| 类别 | 选型 | 说明 |
|---|---|---|
| 操作系统 | Ubuntu 24.04 LTS（x86_64） | 本机真机环境；方案同样兼容 WSL2 |
| 开发语言 | C（C17 标准） | 仅依赖 libc，**零第三方 C 库依赖** |
| 编译器 | GCC 13.3.0 | `-Wall -Wextra -O2`，警告零容忍 |
| 构建系统 | Makefile | 目标：`sim` / `test` / `bench` / `exp` |
| 调试 | GDB + VSCode（C/C++ 扩展） | 图形化调试器 + `--trace` 文本 trace 双手段 |
| 版本管理 | Git + GitHub（public 仓库） | 小步提交；noreply 邮箱，仓库无个人信息 |
| 交叉工具链 | `gcc-riscv64-unknown-elf`（Ubuntu 官方源） | 编译 C benchmark；`objcopy -O binary` 产出 .bin；装不上走兜底（计划书 R1） |
| 数据分析 | Python 3 + venv + Matplotlib | 读 CSV 出图；实验脚本一键复现 |
| 运行形态 | 单机命令行程序 `./sim` | 模拟器本身为宿主机原生程序，非解释执行 |

---

## 3. 文件架构

```
riscv-sim/
├── docs/                  # 立项书、计划书 v1.0、Week 0 准备清单、（后期）实验报告
├── src/                   # 模拟器源码（各模块 .c/.h 成对）
│   ├── isa.c/.h           # 译码 + 执行语义（流水线与 golden 共享，杜绝两套语义）
│   ├── pipeline.c/.h      # 五级流水线寄存器、逐周期推进、分支解析与冲刷
│   ├── hazard.c/.h        # 转发网络、load-use 检测与 stall、PC/IF_ID 冻结
│   ├── cache.c/.h         # 直接映射 D-Cache、地址划分、hit/miss、WT+WA、miss 冻结
│   ├── memory.c/.h        # 1MB 平坦小端内存、非对齐检查
│   ├── loader.c/.h        # hex 文本 / .bin 装载、复位状态（PC=0、SP=0x00100000）
│   ├── golden.c/.h        # 单周期参考模型、逐条 diff、内存全量 diff
│   ├── stats.c/.h         # 周期/冒险/Cache 计数、CPI/IPC、CSV 输出
│   └── main.c             # CLI 解析、主循环、ECALL 停机、MAX_CYCLE 保险、trace 输出
├── tests/                 # 回归用例（hex + expected：寄存器终态 + 精确周期数）
│   ├── arithmetic/        # 算术与逻辑
│   ├── hazard/            # 每条转发路径、load-use、冲刷各配独立用例
│   ├── branch/            # 分支 / 跳转 / 冲刷
│   └── cache/             # 命中 / 缺失 / 写分配
├── bench/                 # B1–B5 benchmark 源码 + crt0.S + link.ld
├── scripts/
│   ├── plot.py            # CSV → PNG 图表
│   └── run_experiments.sh # 一键复现全部实验数据
├── Makefile
└── README.md
```

**输入格式**：hex 文本（每行一条 32 位指令，用于单元测试）与真实工具链产出的 .bin（用于 benchmark）；统一从 0x0 装载。

**内存布局**：text @ 0x00000000 ｜ data @ 0x00010000 ｜ stack 0x00080000–0x00100000（向下生长，SP 复位值 0x00100000）。

---

## 4. 技术栈

| 层次 | 涉及技术 |
|---|---|
| 语言与底层 | C17 位运算与掩码、结构体建模流水线寄存器、`uint8_t` 数组模拟内存、小端序读写、符号扩展 |
| ISA 与编码 | RV32I 五种指令格式（R/I/S/B/U/J）、opcode/funct3/funct7 译码、立即数提取、hex/bin 双格式加载、ECALL 停机约定 |
| 微体系结构 | 周期精确状态机、四级流水线寄存器、转发网络（ALU/分支比较/JALR 基址/store-data 四类用点）、load-use stall、分支冲刷（罚 2 拍） |
| 存储层次 | 直接映射 Cache、tag/index/offset 地址划分、write-through + write-allocate、阻塞式 miss（整机冻结 penalty 拍）、命中统计按 load/store 分列 |
| 验证方法 | 单周期参考模型逐条 diff（PC + x1..x31）、halt/异常时全量内存 diff、精确周期数回归断言、CPI 手算对账 |
| 数据与可视化 | CSV 逐配置点输出、Matplotlib 曲线/柱状图、实验矩阵脚本化复现 |
| 工程实践 | Makefile 多目标构建、Git 小步提交、GDB 调试、公共仓库过程记录 |

---

## 5. 预期成果

### 5.1 软件成果

单一可执行程序 `./sim`，支持 11 个 CLI 参数（`--trace / --stats / --csv / --cache-size / --block-size / --miss-penalty / --no-forwarding / --no-cache / --single-cycle / --max-cycle` 及输入文件）；指令集覆盖 **V1 核心 16 条 → V2 共 38 条**（即 RV32I 基础整数集除去 FENCE/EBREAK）；核心代码约 **2,500 行 C**。

### 5.2 测试成果

回归测试集 **≥ 30 个用例**，覆盖算术/访存/分支/冒险/Cache 五类；其中每条转发路径、load-use、分支冲刷均有独立用例，小用例断言**精确周期数**；`make test` 一键全绿，且作为跑 benchmark 的前置门禁。

### 5.3 实验成果

实验矩阵 **E1–E6** 共六组，每组产出 CSV + 图表：E1 转发开关对 CPI 的影响；E2 Cache 容量扫描（64B–64KB）对 Miss Rate；E3 块大小扫描（4–64B）；E4 Miss Penalty 扫描（10/20/50）；E5 编译器 -O0/-O2 对照；E6 矩阵乘循环序（i-j-k vs k-j-i）对 Cache 行为的影响。基于 **B1–B5** 五个 benchmark（数组求和、矩阵乘、冒泡排序、递推链、跨步拷贝），数据由 `scripts/run_experiments.sh` 一键复现。

### 5.4 文档成果

五章实验报告（背景 / 系统设计 / Pipeline 实现 / Cache 实现 / 实验，含架构总图与 CPI 分解表）、README（无个人信息）、复试问答一页纸（10 个高频问题 + 30 秒电梯稿）。

---

## 6. 检验目标（验收标准）

### 6.1 分级里程碑验收

| 里程碑 | 日期 | 验收内容 |
|---|---|---|
| M1 | 2026-10-14 | V1 十六条指令全功能：流水线 + 转发 + stall + 冲刷，golden 全绿，冒险用例精确周期数通过 |
| M2 | 2026-10-28 | V2 扩展 + D-Cache 全部完成：`make test` 全绿，CLI 全参数可用 |
| M3 | 2026-11-25 | 交付物齐全：E1–E6 图表 + 五章报告 + README + 问答页，仓库过程记录完整 |

冻结线 2026-11-30 不动；未竟项按计划书降级阶梯滑入 1 月 stretch，不挤占 12 月。

### 6.2 量化验收指标

| # | 指标 | 目标值 | 验证方式 |
|---|---|---|---|
| 1 | 指令集覆盖 | 38 条（V2 后） | `make test` 全绿 |
| 2 | 功能正确性 | Golden Model 逐条 diff **零不一致**（全部用例 + 全部 benchmark） | 自动断言 + 退出码 |
| 3 | 时序精确性 | 精确周期数断言 100% 通过；至少 1 个 CPI 手算对账用例（手算 = 模拟器输出） | expected 文件 + 对账小程序 |
| 4 | 回归规模 | ≥ 30 用例，五类覆盖，含全部转发路径独立用例 | `make test` |
| 5 | Benchmark | 5 个全部跑通且 golden 绿 | `make bench` |
| 6 | 实验完整性 | E1–E6 全部产出 CSV + PNG，一键复现 | `bash scripts/run_experiments.sh` |
| 7 | 文档 | 五章报告 + 架构图 + 问答 10 题 | M3 整体检查 |
| 8 | 工程规范 | public 仓库、小步提交（预估 ≥ 40 commits）、无个人信息、`make` 一键构建 | 仓库检查 |

### 6.3 门禁规则（一票否决项）

- Golden diff 未全绿时，**禁止**运行 benchmark 采集实验数据（由 `make` 目标依赖强制）。
- 任何精确周期数断言失败，必须先归因修复再继续新功能开发。
- 实验数据与 CPI 分解公式对不上账时，数据不出报告。

---

## 7. 进度与风险（摘要）

里程碑与逐周排期见计划书 §4；五项风险（工具链、冒险边角 bug、时间侵蚀、口径漂移、绘图环境）及兜底方案见计划书 §6。核心约束一句话：**冻结线不动，宁可砍范围，绝不延 12 月。**
