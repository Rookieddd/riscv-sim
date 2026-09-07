# Week 1 策划执行书 —— 工程骨架与 ISA 执行链路打通

| 项 | 内容 |
|---|---|
| 名义周期 | 09.10–09.16（实际滚动推进，时间节点只作里程碑坐标） |
| 里程碑坐标 | M1（V1 全功能）的第 1/5 段：从 0 到"第一条指令跑起来" |
| 状态 | 待开工 |
| 本文件地位 | **周书模板定版**：W2–W12 沿用本八栏目结构，按"工程迭代记录"形式书写 |

---

## ① 上周小结

**核实过的完成项**（详见 [week0 日志](../../../../RISCV-test/weekly/week0-log.md)）：

- 环境：riscv-gcc 13.2.0 / gdb 15.1 / matplotlib 3.11.1 / VSCode 全部就绪
- 仓库：`github.com/Rookieddd/riscv-sim`（public），首提交 `2646b31`，3 份文档入库
- 能力：C 自测 T1–T5 全部通过且已编译运行，`extract_bits` 通用掩码函数可直接升级为产品代码
- 风险表更新：R1（工具链装不上）解除，F1 决策点闭环

**遗留带入本周**：

1. `docs/week0-preparation.md` 的 A3 修正尚未提交（本周 commit #1 一并带入）。
2. 附加题 `0xFFF00093` 拆解未做——**并入本周 T4 热身**，它就是 I 型立即数译码本身。

**能力基线**：会位提取与掩码 → 本周目标是从"能做题"跨到"能写产品代码"。

## ② 本周定位

打通 **文件 → 内存 → 译码 → 执行 → 停机** 的完整最小链路，并让两条架构原则从第一天落进代码：

1. **golden.c 是 ISA 执行语义的家**（Q3 决议）：单周期参考实现先行存在，W3 的流水线只是它的第二个客户。
2. **isa.c 一份语义两处复用**：译码与执行只写一遍，杜绝 golden 与流水线语义漂移（计划书 §3.5）。

本周完成前，项目没有"能算数的程序"；本周完成后，一切后续机制（流水线、冒险、Cache）都叠加在这条链路上。

## ③ 功能目标（本周收口时"能跑什么"）

| # | 功能 | 验收一句话 |
|---|---|---|
| F1 | 零警告构建 | `make` 通过 `-Wall -Wextra -Werror`，产出 `build/sim` |
| F2 | 执行 demo 程序 | `./build/sim tests/demo.hex` 正确执行 addi/add/ecall 并报告停机与寄存器值 |
| F3 | 逐周期 trace | `--trace N` 每周期打印 pc、原始指令、译码结果 |
| F4 | 周期保险 | `--max-cycle N` 到限强制停机（默认 10,000,000） |
| F5 | 防御式输入 | 文件不存在 / 非法 hex 行 → 带行号的明确报错 + 约定退出码 |
| F6 | 译码覆盖 R/I/S 三型 12 条 + ECALL | 未知指令 → 报"暂不支持"并指明 opcode |

**范围声明（对计划书的解释性收窄）**：BEQ/BNE/JAL/JALR 的译码（B/J 型拼位）与全部访存/控制流**执行语义**属 W2；本周执行语义覆盖 R 型 5 条 + I 型算术 4 条 + ECALL。依据 D11 渐进原则，W1 先把"一条指令的一生"完整走通。

## ④ 操作目标（任务块）

> 协作模式（Q2 决议）：T1 骨架由助手搭建，T3 loader 由助手示范，其余你写。每块含：产出 / 涉及文件 / 知识锚点 / 预估时长。

### T1 骨架搭建（助手执行，你审查）

- **产出**：目录结构、`Makefile`（`sim` + `debug` 两个目标，见 D9 扩展）、`.gitignore`、各 `.h` 的接口声明与注释、commit #1
- **你要做**：通读头文件，能回答"每个模块对外提供什么"（T6 概念题会问）
- 预估：0.5h

### T2 memory.c（你写的第一行产品代码）

- **产出**：`memory_t`（1MB `uint8_t` 数组）+ `mem_read8/16/32`、`mem_write8/16/32`
- **硬性约定**：小端拼字必须逐字节移位组合（你 T5 的答案原样升级）；`read32/write32` 校验 `addr % 4 == 0`，违规走退出码 5
- **知识锚点**：Week0-T5；布局图（text 0x0 / data 0x10000 / stack 0x80000–0x100000）默画一遍
- 预估：1–1.5h

### T3 loader.c（助手示范 + 你合卷重写）

- **产出**：`load_hex(path, mem)`：逐行读入 8 位十六进制 → 顺序写入 0x0 起始内存；错误带行号
- **形式**：助手提交示范实现 → 你**合上示例独立重写一遍** → 与示范 diff，差异点写进 commit message
- **知识锚点**：`fgets`/`strtol`/防御式解析（非法字符、超 1MB、空行容忍）
- 预估：1.5h（1h 重写 + 0.5h 对照）

### T4 isa.c 译码器（本周最大块，你写）

- **产出**：`decoded_t` 结构体（raw/opcode/rd/rs1/rs2/funct3/funct7/imm/fmt）+ `isa_decode()`，覆盖 R/I/S 三型 + ECALL
- **硬性约定**：imm 字段**必须在本层完成符号扩展**（存放的已是带符号值）；`fmt` 枚举让上层不必再关心拼位规则
- **知识锚点**：Week0-T1/T2 直接延伸；附加题 `0xFFF00093` 先做（预期 imm = -1）；P&H 图 2.3
- 预估：2–3h

### T5 golden.c 执行器（你写）

- **产出**：`cpu_t { pc, xreg[32], halted }` + `golden_step()`：取指 → 译码 → 执行（R 型 5 条 + ADDI/ANDI/ORI/XORI + ECALL）→ pc += 4；ECALL 置 halted
- **硬性约定**：对 x0 的写入一律忽略（写回层统一拦截）；未知 opcode 返回错误而非静默跳过
- **知识锚点**：单周期 CPI=1 的含义；为什么执行语义住在 golden 而不是 main
- 预估：1.5–2h

### T6 main.c CLI + demo 验证（你写）

- **产出**：参数解析（输入文件、`--trace N`、`--max-cycle N`）、trace 行格式 `cycle N: pc=... inst=... 译码摘要`、退出码表（0 正常停机 / 2 用法错 / 3 装载错 / 4 非法指令 / 5 对齐违规）、`tests/demo.hex`
- **demo 编码先自己验算再对答案**：`addi x1,x0,10`、`add x2,x1,x1`、`ecall` 三条的 hex 值推导写进提交说明；预期终态 x1=10、x2=20、pc=0xC
- 预估：1–1.5h

## ⑤ 自检准则（三层全绿 = W1 收口）

**A. 命令层（逐条执行，记录实际输出）**

| # | 命令 | 预期 |
|---|---|---|
| A1 | `make` | 无任何警告输出，生成 `build/sim` |
| A2 | `./build/sim tests/demo.hex` | `Halted: a0=0, pc=0x0000000C` 且 x1=10、x2=20，`echo $?` 为 0 |
| A3 | `./build/sim tests/demo.hex --trace 10` | 打印 3 个周期后停机，每行含 pc 与译码摘要 |
| A4 | `./build/sim nofile.hex; echo $?` | 报错含文件名，退出码 3 |
| A5 | demo.hex 中插入一行 `zzzzzzzz` 再运行 | 报错含行号，退出码 3 |
| A6 | `make debug && ./build/debug/sim tests/demo.hex` | sanitizer 静默通过（0 报告） |

**B. 代码层（能向别人指着代码讲）**

- B1 五个 opcode 常量默写：R=0x33、I=0x13、S=0x23、SYSTEM=0x73
- B2 imm 为什么在译码层就符号扩展（用 0xFFF00093 → -1 举例）
- B3 x0 写忽略发生在哪一行，为什么不放在译码层
- B4 `mem_write32` 对齐检查删掉后，什么样的程序会悄悄出错？（现在答"还不确定的场景"，W2 分支跳转后回来看）

**C. 概念层（复试向，口头 30 秒/题）**

- C1 为什么译码/执行语义必须 golden 与流水线共享同一份 isa.c？
- C2 单周期 CPI=1 意味着什么？它为什么反而"慢"（对比五级流水线的理想 CPI）？
- C3 loader 为什么必须防御式解析？信任输入的最坏后果是什么？
- C4 1MB 内存布局中 data 为什么不紧贴 text 结尾？（给 W2 的 .bin 装载留的对齐余量）

## ⑥ 提交计划

| # | message | 内容 | 谁提交 |
|---|---|---|---|
| 1 | `chore: project skeleton (Makefile + src layout)` | 骨架 + week0 文档修正带入 | 助手 |
| 2 | `feat(memory): 1MB little-endian memory with alignment checks` | T2 | 你 |
| 3 | `feat(loader): hex text loader` | T3（附重写对照说明） | 你 |
| 4 | `feat(isa): RV32I R/I/S decoder with sign-extended immediates` | T4 | 你 |
| 5 | `feat(golden): single-cycle executor (arith + ecall)` | T5 | 你 |
| 6 | `feat(main): CLI, trace, exit codes + demo program` | T6 | 你 |
| 7 | `docs: week1 execution book` | 本文件 | 你 |

纪律：单 commit diff 以 ≤200 行为宜；message 用类型前缀；push 前 `make` 必须零警告。

## ⑦ 问题预案（本周三大卡点 + 自救路径）

1. **Makefile 报 `missing separator`** → 十有八九是规则行前丢了 Tab。骨架模板可整体照抄，只改文件名。
2. **译码位段抽出错值** → 用 Week0-T2 的打印法对 `0x00A00093` 做逐字段断言；先查掩码宽度（`0x1F` 是 5 位，不是 `0x1FF`）。
3. **段错误** → `make debug`（AddressSanitizer + UBSan 版本）跑一遍，报错行号基本直指病灶；再 `gdb --args ./build/debug/sim ...` 配 `bt` 看调用栈。

自救总则：卡住超过 30 分钟 → 把"代码 + 完整报错 + 已尝试的三个动作"发给助手，不要独自耗着。

## ⑧ 协作点

- **触发词"开工"**：助手执行 T1（骨架 + loader 示范 + commit #1），交付后你从 T2 开始。
- **每完成一个任务块**：push 后请助手 review，按"正确性 / 规范 / 可讲解性"三栏给意见。
- **T3 重写对照**与 **demo 编码验算**是本周两处必须诚实执行的环节——它们是 W1 最重要的学习事件。
- W1 收口后：助手做周验收（⑤全绿确认），随后生成 Week 2 执行书。

---

*模板使用说明（W2–W12 沿用）：八栏目顺序与内涵不变；③ 的功能数、④ 的任务块数、⑤ 的三层结构按周内容伸缩；每周末在状态栏更新进度并在 ① 写下一段历史。*
