# riscv-sim 项目实施规划（v1.0 冻结版）

> 冻结日期：2026-09-06 ｜ 启动：2026-09-10
> 性质：纯自学项目，研究生复试加分项
> 代码 + 实验 + 报告冻结线：**2026-11-30**；Stretch 窗口：2027-01-01 ~ 01-15；12 月完全让位考研初试

---

## 1. 可行性结论

**可行，批准立项。** 依据：

- 无技术未知区：设计直接映射 P&H《计算机组成与设计》流水线（第 4 章）与存储层次（第 5 章），全部机制教科书可查。
- 工作量：核心代码约 2,500 行 C + 测试用例 + 5 个 benchmark + 绘图脚本 + 5 章报告。
- 预算：11.5 周 × 10–15h ≈ **115–170h**；需求估算约 120–140h，缓冲 15–25%。
- 两类风险各有兜底：正确性风险由 golden model + 精确周期数断言兜底；进度风险由冻结线 + 降级阶梯（§4）兜底。

---

## 2. 冻结决策表（v1.0）

| 模块 | 决策 |
|---|---|
| ISA | 两阶段：V1 核心 16 条 → V2 共 38 条（= RV32I 基础集去掉 FENCE/EBREAK） |
| Loader | hex 文本（单测）+ RISC-V 工具链 .bin（benchmark）；不做 ELF 解析 |
| Pipeline | 五级 IF→ID→EX→MEM→WB，四个流水寄存器 |
| Branch | EX 级解析，Predict-Not-Taken，taken 冲刷 IF/ID 与 ID/EX，罚 2 拍 |
| Hazard | 转发 + load-use stall + flush（转发网络见 §3.3） |
| Cache | 直接映射 L1 D-cache，write-through + write-allocate，blocking（miss 整机冻结），参数全部走 CLI |
| Memory | 1MB 平坦、小端；布局见 §3.2；非对齐访存报错 |
| Halt | ECALL 停机（a0=0 正常 / 非 0 报错退出）+ MAX_CYCLE=10M 保险 |
| 正确性 | 单周期 golden model（仅 ISA 语义，无流水无 cache）逐条提交 diff |
| 测试 | tests/ 回归 + `make test`，小用例断言寄存器终态 + 精确周期数 |
| 统计 | CPI = Total Cycles / IC，四项分解；同时输出 IPC（§3.6） |
| 输出 | `--trace N` ASCII 流水线图 + `--csv` 导出 + Python/Matplotlib 出图；不做 GUI |
| 构建 | Makefile、C17、GCC 13.3、模块化多文件、零第三方 C 依赖 |
| 仓库 | GitHub **public** 从第一天开始，小步提交，无任何个人信息 |
| 报告 | 中文 Markdown，5 章结构（§7） |

**明确不做（v1 防蔓延清单）**：ELF 解析器、I-Cache、TLB/虚存、L2、N 路相联 + LRU、动态分支预测器、ID 级分支解析、GUI、mini-assembler（降为第三阶段）、多核 / 中断 / CSR。

---

## 3. 系统设计基线

### 3.1 指令集两阶段

- **V1（16 条，证明 CPU 正确）**：ADD SUB AND OR XOR ｜ ADDI ANDI ORI XORI ｜ LW SW ｜ BEQ BNE ｜ JAL JALR ｜ ECALL
- **V2（+22 条 → 38 条）**：SLL SRL SRA SLLI SRLI SRAI ｜ SLT SLTU SLTI SLTIU ｜ LUI AUIPC ｜ BLT BGE BLTU BGEU ｜ LB LBU LH LHU SB SH
- 复试话术：V2 完成后即为**完整 RV32I 基础整数指令集（除 FENCE/EBREAK）**。

### 3.2 内存布局（D3 修订版）

```
0x00000000   text
0x00010000   data
0x00080000   ─┐
             │  stack（向下生长）
0x00100000   ─┘  SP 复位值 = 0x00100000
```

1MB 平坦内存、小端序；PC 复位到 0；模拟器复位时将 x2(SP) 初始化为 0x00100000；非对齐 LW/SW/LH/SH 直接报错（RV32I 自然对齐要求）。

### 3.3 流水线与冒险

- 转发源两级：EX/MEM.alu、MEM/WB.alu、MEM/WB.load；转发用点四处：
  1. ALU 操作数 A/B（EX 级）
  2. 分支比较器操作数 A/B（EX 级）
  3. JALR 目标地址基址（EX 级）
  4. SW 的 store-data（EX 级读入流水、MEM 级使用）
- load-use：load 结果只在 WB 级可用后转发，后一条相关指令 stall 1 拍（即使开了 forwarding）。
- **每条转发路径 + load-use + 分支冲刷各配一个独立单测用例**，断言精确周期数。
- 统计口径：data stall（非 load-use）与 load-use stall 分开计数，便于归因。

### 3.4 Cache

- Line 结构 `{valid, tag, data[block]}`；地址划分 tag/index/offset 由容量与块大小参数推导。
- CLI：`--cache-size B`、`--block-size B`、`--miss-penalty C`、`--no-cache`（完美内存模式，供 E1 对照）。
- 命中 1 拍；miss 整机冻结 penalty 拍。报告声明：**阻塞式 Cache（Blocking Cache）模型**，不建模 MSHR / 非阻塞多请求；write-through 命中时写内存不额外计时。
- 命中/缺失统计按 load / store 分列。

### 3.5 Golden Model（质量底座）

- 单周期 ISA 语义，**无流水线、无 cache**——只保证 ISA 正确性，不复制第二套复杂系统。
- 与流水线**共享同一份 isa.c 执行语义**，杜绝两套语义漂移。
- 每条指令提交时 diff PC + x1..x31（x0 恒零由译码/写回层单独保证：对 x0 的写一律忽略）；halt 时及首次不一致时全量 diff 内存，并 dump 五个流水级现场。
- golden 的访存绕过 cache，不污染命中率统计。

### 3.6 统计口径（D1 修订版）

```
CPI = Total Cycles / Instruction Count
IPC = Instruction Count / Total Cycles
Total Cycles = Ideal (IC + 4 流水填充) + Data Stall + Branch Flush + Cache Stall
```

输出行：instructions、cycles、cpi、ipc、data_stall_cycles、branch_flush_cycles、cache_stall_cycles、各类事件数、cache hit/miss（load/store 分列）。CSV 每行 = 一次运行（一个配置点）。

### 3.7 CLI 约定

```
./sim prog.hex|prog.bin [--trace N] [--stats] [--csv FILE]
  [--cache-size B] [--block-size B] [--miss-penalty C]
  [--no-forwarding] [--no-cache] [--single-cycle] [--max-cycle N]
```

### 3.8 工程结构

```
riscv-sim/
├── src/        isa · pipeline · hazard · cache · memory · loader · golden · stats · main（各 .c/.h）
├── tests/      arithmetic/ hazard/ branch/ cache/   （hex + expected：寄存器终态 + 精确周期数）
├── bench/      B1–B5 源码 + crt0.S + link.ld
├── scripts/    plot.py · run_experiments.sh
├── Makefile    目标：sim / test / bench / exp
└── README.md
```

### 3.9 Benchmark 编译链（W8 落地）

```
bench/xxx.c + crt0.S → riscv-gcc -march=rv32i -mabi=ilp32 -nostdlib -O0/-O2 -T link.ld → ELF
  → objcopy -O binary → .bin → ./sim
```

link.ld：.text→0x0、.data→0x10000；crt0.S 设 gp（可选）、调 main、尾接 ECALL；SP 由模拟器复位值提供。

---

## 4. 逐周里程碑（2026-09-10 → 11-30）

| 周 | 日期 | 内容 | 出口标准（可验证） |
|---|---|---|---|
| W1 | 09.10–09.16 | 建仓（public）、Makefile 骨架、hex loader + 1MB 内存、V1 译码器、`--trace`。**F1 决策点：装 riscv-gcc** | `addi/add/ecall` hex 程序跑通 |
| W2 | 09.17–09.23 | 单周期 golden model + 逐条 diff + tests/ 框架（expected 文件） | V1 全部用例在 `--single-cycle` 下 `make test` 绿 |
| W3 | 09.24–09.30 | 五级流水线骨架（无冒险处理）、逐周期推进、trace 输出 | 无相关用例的周期数与手算一致 |
| W4 | 10.01–10.07 | 转发网络（全部用点）+ load-use stall + 统计计数 | 冒险用例全绿且含精确周期数；`--no-forwarding` 对比可跑 |
| W5 | 10.08–10.14 | 分支 EX 解析 + flush + JAL/JALR + 分支统计 | **M1：V1 全功能，golden 全绿** |
| W6 | 10.15–10.21 | V2 指令扩展 + 对应单测 | V2 全绿 |
| W7 | 10.22–10.28 | D-cache（DM、WT+WA、冻结 penalty、CLI 参数）+ cache 统计 | **M2：功能全部完成**，cache 单测绿 |
| W8 | 10.29–11.04 | benchmark 工程：crt0/link.ld、B1–B5、-O0/-O2、CSV 定型（F1 失败则此处启用 mini-assembler 兜底） | 5 个 benchmark 全部跑通且 golden 绿 |
| W9 | 11.05–11.11 | 执行实验 E1–E6、plot.py 出图、数据对账（手算小程序验证 CPI 公式） | 全部图表产出且过 sanity check |
| W10 | 11.12–11.18 | 报告初稿（5 章）、架构图、CPI 分解表 | 5 章草稿成文 |
| W11 | 11.19–11.25 | 报告定稿、README、复试问答一页纸、repo 清理 | **M3：交付物齐** |
| — | 11.26–11.30 | 缓冲 | — |
| — | 12 月 | **冻结，全力初试** | — |
| — | 01.01–01.15 | Stretch：mini-assembler、N 路 + LRU、ID 级解析对比、ELF32、I-Cache | 可选 |

**降级阶梯（硬约束，冻结线不动）**：

1. 10.14 M1 未达成 → W6 的 V2 只加 benchmark 最小依赖六条（SLLI SRLI SRAI LUI BLT BGE），其余进 stretch。
2. 10.28 M2 未达成 → 砍 E3（块大小扫描）、E4（penalty 扫描），保 E1/E2/E5/E6。
3. 11.18 初稿未成 → 报告按"设计章 + 实验章优先，背景/总结从简"压缩。
4. 任何未竟项一律滑入 1 月 stretch，**绝不挤占 12 月**。

---

## 5. 实验矩阵

| 编号 | 实验 | 变量 / 固定量 | Benchmark | 产出图 |
|---|---|---|---|---|
| E1 | Forwarding on/off | `-O0`；关转发 | B1 B3 B4 | CPI 分解柱状图（含 load-use 单列） |
| E2 | Cache 容量扫描 | 64B→64KB 步进 ×2；block=16B，penalty=20 | B1 B2 B5 | Miss Rate–容量曲线（对数横轴） |
| E3 | 块大小扫描 | 4/8/16/32/64B；容量 4KB | B1 B2 B5 | Miss Rate–块大小曲线 |
| E4 | Miss penalty 扫描 | 10/20/50；容量 4KB，block 16B | 全部 | CPI–penalty 线性图 |
| E5 | 编译器对照 | -O0 vs -O2 | B1 B3 B4 | 指令数 + CPI 对比（编译器消冒险/减指令数） |
| E6 | 矩阵循环序 | i-j-k vs k-j-i，跑一遍容量扫描 | B2 | 循环序 × 容量 Miss Rate 对比（招牌图） |

每个实验一个 CSV + 一张图；`scripts/run_experiments.sh` 一键复现全部数据。

---

## 6. 风险与兜底

| # | 风险 | 等级 | 兜底 |
|---|---|---|---|
| R1 | riscv-gcc 装不上（apt 需密码、源需 update） | 低 | ① xPack 预编译工具链 tarball（免 sudo）；② mini-assembler 兜底（scope 见 D5），F1 于 W1 末决策 |
| R2 | 冒险边角 bug（五条转发路径、load-use 撞分支） | 中 | golden 逐条 diff + 精确周期数断言 + 每条路径独立用例；W4 整周预留给冒险 |
| R3 | 时间被初试复习 / 其他计划侵蚀 | 中 | 冻结线 + 降级阶梯为硬约束；Ubuntu 学习计划降级为本项目"顺带学"（git/make/gdb/apt/SSH 全覆盖） |
| R4 | 统计口径漂移导致实验对不上账 | 低 | CPI 公式先写进 stats.h 注释；W9 用手算小程序对账 |
| R5 | matplotlib 安装（PEP 668 限制） | 低 | `python3 -m venv` + pip 清华镜像；实在不行实验期再装，不阻塞主线 |

---

## 7. 报告结构（D6 采纳）与复试资产

**5 章**：① 项目背景（ISA→Pipeline→Cache→Performance 的动机链）② 系统设计（架构总图）③ Pipeline 实现（流水寄存器 / Forwarding / Stall / Flush）④ Cache 实现（映射 / 地址划分 / Hit-Miss / 写策略）⑤ 实验（图 1 forwarding–CPI、图 2 branch 频率–CPI、图 3 cache size–miss rate、图 4 矩阵循环序–cache）。

**附录 A：复试高频问答清单**（写进报告，练到能脱口而出）：

1. 为什么 load-use 有 forwarding 还要 stall 1 拍？（load 数据 MEM 末才可用，EX 级来不及）
2. 为什么 taken 罚 2 拍？如果 ID 级解析呢？（对比 stretch 实验）
3. Write-through vs write-back 的取舍？你的模拟器为什么选 WT？
4. 直接映射为什么会有冲突 miss？组相联怎么缓解？
5. 为什么 i-j-k 和 k-j-i 的矩阵乘 miss rate 差这么多？
6. -O2 为什么指令数更少 CPI 也变了？编译器和微体系谁影响大？
7. 你的模拟器怎么证明自己是对的？（golden model 方法论——本项目最亮的工程点）
8. CPI 四项分解里每项在真实 CPU 里对应什么？
9. 阻塞式 cache 和真实 CPU 的非阻塞 cache 差在哪？（MSHR）
10. 下一步怎么做会更快？（动态分支预测、返回地址栈、写缓冲、I-Cache）

**电梯稿**（30 秒版）：我用 C 实现了一个周期精确的 RV32I 五级流水线模拟器，完整实现了转发、流水线暂停、分支冲刷和可配置的直接映射 D-Cache；用单周期参考模型逐条指令自动对照保证正确性，并用精确周期数做回归断言；最后用 5 个 benchmark 做了 forwarding、Cache 参数和矩阵循环序的定量实验，CPI 可以手算对账。

---

## 8. W1 行动清单（09.10 当天起）

1. `mkdir riscv-sim && git init`；GitHub 建 public 仓库（README 先占坑）；`git config user.email` 用 GitHub noreply 地址，全仓库不出现姓名/学校/邮箱。
2. **你亲自执行（需要密码）**：`sudo apt update && sudo apt install -y gcc-riscv64-unknown-elf` —— F1 决策点，装不上记录现象，W8 前定 fallback。
3. 绘图环境：`python3 -m venv ~/venvs/plot && ~/venvs/plot/bin/pip install matplotlib -i https://pypi.tuna.tsinghua.edu.cn/simple`。
4. Makefile 骨架 + src/ 九个空模块 + .gitignore；首提交：`project skeleton: Makefile + module layout`。
5. 实现 hex loader + memory.c（含对齐检查）+ isa.c 译码起步（V1 的 16 条）。
6. 周末自检：`./sim tests/arithmetic/first.hex --trace 20` 能看到取指—译码—执行。

---

*本文件为 v1.0 冻结版。执行中如需变更，先改这份文档、写清理由，再改代码——文档即规格。*
