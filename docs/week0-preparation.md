# Week 0 准备清单（09.06–09.09）

> 目标：09.10 进入 W1 时，环境零障碍、C 语言达到项目门槛、知道每周该读什么。
> 总预算：3 天 × 1–2h/天。**按"刚好够用"设计，不要超时学前置知识**——剩余知识在项目里边做边学。

---

## A. 环境准备（第 1 天，约 1–2h，命令可直接复制）

### A1. 系统工具与 RISC-V 工具链（需要输密码，亲自跑）

```bash
sudo apt update
sudo apt install -y gcc-riscv64-unknown-elf gdb
# 验证（两条都有版本号输出即成功）：
riscv64-unknown-elf-gcc --version
gcc --version
```

> 装不上不要恋战：把报错原文记下来。这是计划书里的 **F1 决策点**，W8 之前都有兜底方案，不阻塞任何事。

### A2. Git 与 GitHub（约 30–60 分钟）

1. GitHub → New repository → 名字 `riscv-sim` → **Public** → **不勾选** README（本地已有文件）。
2. 配置身份。邮箱用 GitHub 的 noreply 地址（GitHub → Settings → Emails，形如 `12345+用户名@users.noreply.github.com`，**不要用真实邮箱**）：

```bash
git config --global user.name "你的GitHub用户名"
git config --global user.email "12345+用户名@users.noreply.github.com"
```

3. SSH 免密（一次性）：

```bash
ssh-keygen -t ed25519 -C "riscv-sim"    # 一路回车
cat ~/.ssh/id_ed25519.pub               # 复制输出内容
```

GitHub → Settings → SSH and GPG keys → New SSH key → 粘贴。
验证：`ssh -T git@github.com` 出现 "successfully authenticated" 即可。

4. 把现有工程目录变成仓库，并让**计划书成为第一个 commit**（commit 历史从第一天就讲工程故事）：

```bash
cd ~/桌面/RISC-V
git init -b main
git add docs/
git commit -m "docs: freeze project plan v1.0 + week0 preparation"
git remote add origin git@github.com:你的用户名/riscv-sim.git
git push -u origin main
```

### A3. Python 绘图环境（5 分钟）

```bash
sudo apt install -y python3.12-venv    # venv 依赖的系统包（需要密码，清单初版漏了它）
rm -rf ~/venvs/plot                    # 若之前建到一半失败，先清掉半成品
python3 -m venv ~/venvs/plot           # 创建隔离的 Python 虚拟环境
~/venvs/plot/bin/pip install matplotlib -i https://pypi.tuna.tsinghua.edu.cn/simple
```

验证：`~/venvs/plot/bin/python -c "import matplotlib; print(matplotlib.__version__)"` 能打印版本号即可。

### A4. 编辑器（10 分钟）

推荐 VSCode（图形化调试器对新手最友好）：`sudo snap install code --classic`，再装扩展 **C/C++**。习惯 vim 的话 vim 完全够用。

---

## B. C 语言自测 5 题（第 2 天，约 1–2h）—— 本项目的真正门槛

本项目 70% 的代码只用到 C 的一个小子集：位运算、结构体、数组、指针。下面 5 题能**独立**做出来就达标；做不出来按题目末尾提示补对应知识点，**不要通读 C 语言教材**。

**T1（位提取）** `uint32_t inst = 0x00A00093;`，写表达式取出第 11~7 位（共 5 位）。
达标线：`(inst >> 7) & 0x1F`。不会 → 复习 `>>`、`&`、掩码（`0x1F` 就是二进制 11111）。

**T2（拆编码）** RV32I 的 I 型格式：`imm[31:20] | rs1[19:15] | funct3[14:12] | rd[11:7] | opcode[6:0]`。
手工拆解 `0x00A00093` 的每一段（它是 `addi x1, x0, 10`）。
达标线：imm=10、rs1=0、funct3=0、rd=1、opcode=0x13。不会 → 对照 P&H 图 2.3 或 RISC-V 卡片，把五种格式（R/I/S/B/U/J）抄一遍。

**T3（符号扩展）** `(int32_t)inst >> 20` 和 `(uint32_t)inst >> 20` 有什么区别？为什么译码立即数必须用有符号类型？
不会 → 复习有符号/无符号、算术移位与逻辑移位。

**T4（结构体）** 声明一个流水线寄存器结构体 `id_ex`：包含 rd、rs1、rs2 三个 5 位字段，一个 32 位立即数，一个 `bool reg_write` 控制信号。
不会 → 复习 struct 定义与 typedef。

**T5（数组当内存）** `uint8_t mem[1<<20];`，写出"从地址 addr 读一个 32 位小端字"的代码（假设 addr 已对齐）。
达标线：

```c
uint32_t v = (uint32_t)mem[addr]
           | ((uint32_t)mem[addr+1] << 8)
           | ((uint32_t)mem[addr+2] << 16)
           | ((uint32_t)mem[addr+3] << 24);
```

不会 → 复习数组与小端序。`memcpy(&v, &mem[addr], 4)` 也算对。

T1–T5 全对 → C 准备完成，后面边做边学。

---

## C. 体系结构知识：只读三样，按周读（第 2–3 天穿插）

| 期限 | 材料 | 读什么 | 为什么 |
|---|---|---|---|
| W1 开始前 | P&H《计算机组成与设计：硬件/软件接口》RISC-V 版，第 2 章 | 2.1–2.5 + 图 2.3（指令格式） | W1 写译码器的字典 |
| W3 之前 | 同书 第 4 章 4.5 节 | 流水线概念 + 那组流水线时空图 | W3 写流水线骨架前过一遍 |
| W7 之前 | 同书 第 5 章 5.1–5.2 | 直接映射 cache、写策略 | W7 写 cache 前再读 |

- RISC-V Unprivileged Spec **只当字典查**（字段定义最权威），不要通读。
- 中文资源二选一：B 站体系结构公开课，或"一生一芯"(ysyx) 讲义前几章——比教材口语化。

---

## D. 开工检查点（09.09 晚自检）

- [ ] `riscv64-unknown-elf-gcc --version` 有输出
- [ ] GitHub 有 public 的 `riscv-sim` 仓库，`git push` 成功过（计划书已是第一个 commit）
- [ ] `git log` 里没有真实姓名/邮箱
- [ ] T1–T5 全部独立做对
- [ ] P&H 2.1–2.5 读完，五种指令格式能默画

全部打勾 → 09.10 按计划书 §8 的 W1 清单开工。

---

## E. 协作方式（和 AI 助手）

1. **你写代码，我 review**——学习发生在你手上；每完成一小块贴给我看，我指出问题但不代写。
2. **卡住超过 30 分钟就发我**：代码 + 报错/现象 + 你已经试过什么，不要独自耗着。
3. **M1/M2/M3 三个里程碑**我帮你做整体检查：正确性、代码结构、报告素材。
4. 你说"**开工**"，我先把 W1 骨架（Makefile + 目录 + 空模块）搭好，你从第一个 hex loader 开始填肉。
