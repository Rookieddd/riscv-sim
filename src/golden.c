#include "golden.h"
#include "isa.h"
#include "sim.h"
#include <stdio.h>
#include <stdlib.h>

void cpu_reset(cpu_t *cpu)
{
    cpu->pc = 0;
    for (int i = 0; i < 32; i++)
        cpu->xreg[i] = 0;
    cpu->xreg[2] = (int32_t)STACK_TOP;   /* x2 = SP，栈向下生长 */
    cpu->halted = false;
}

/* ================= TODO(T5)：由你实现 =================
 * week1.md ④-T5。一条指令的一生（单周期）：
 *
 *   1) 取指：uint32_t inst = mem_read32(mem, cpu->pc);
 *   2) 译码：decoded_t d; isa_decode(inst, &d) != ISA_OK → return -1;
 *   3) 执行：R 型 5 条 + ADDI/ANDI/ORI/XORI + ECALL；
            add x2, x1, x1    →  x2 = x1 + x1      funct3=0, funct7=0x00
            sub x3, x1, x2    →  x3 = x1 - x2      funct3=0, funct7=0x20  ← 注意！
            and x4, x1, x2    →  x4 = x1 & x2      funct3=7
            or  x5, x1, x2    →  x5 = x1 | x2      funct3=6
            xor x6, x1, x2    →  x6 = x1 ^ x2      funct3=4
 *      LW/SW/JALR/BEQ/BNE/JAL 本周已可译码但执行语义属 W2，
 *      遇到时打印 "sim: 指令将在 W2 支持" 并 return -1;
 *   4) 写回：结果写入 xreg[rd]，rd == 0 时丢弃（x0 恒零的唯一拦截点）;
 *   5) 顺序推进：cpu->pc += 4（本周没有跳转）;
 *   6) ECALL：x10 == 0 → cpu->halted = true;
 *             x10 != 0 → fprintf(stderr, ...) 并 exit(EXIT_INTERNAL)。
 *
 * 陷阱提示：ADD/SUB 用 int32_t 运算（有符号溢出本模拟器不处理，但
 * 乘除法尚未支持，无需关心）；AND/OR/XOR 位宽用 int32_t 保存即可。
 */
int golden_step(cpu_t *cpu, memory_t *mem)
{
    /* ① 取指：pc 指向的 4 字节就是指令本身 */
    uint32_t inst = mem_read32(mem, cpu->pc);

    /* ② 译码：译码器不认识 → 立即上报，绝不硬猜 */
    decoded_t d;
    if (isa_decode(inst, &d) != ISA_OK)
        return -1;

    /* ③ 执行：两层分发（上一条消息的 if/switch 放这里） */
    int32_t result = 0;
    bool   writeback = true;
    /*
    add x2, x1, x1    →  x2 = x1 + x1      funct3=0, funct7=0x00
    sub x3, x1, x2    →  x3 = x1 - x2      funct3=0, funct7=0x20  ← 注意！
    and x4, x1, x2    →  x4 = x1 & x2      funct3=7
    or  x5, x1, x2    →  x5 = x1 | x2      funct3=6
    xor x6, x1, x2    →  x6 = x1 ^ x2      funct3=4
    */
    if (d.opcode == OP_R){
        int32_t a = cpu->xreg[d.rs1];
        int32_t b = cpu->xreg[d.rs2];
        switch (d.funct3)
        {
        case 0: if(d.funct7 == 0x00) result = a + b;
                else result = a - b;
            break;
        case 4: result = a ^ b;
            break;
        case 6: result = a | b;
            break;
        case 7: result = a & b;
            break;
        default:
            return -1;
        }
    }
    else if (d.opcode == OP_I){
        int32_t a = cpu->xreg[d.rs1];
        int32_t b = d.imm;
        switch (d.funct3)
        {
        case 0: result = a + b;
            break;
        case 4: result = a ^ b;
            break;
        case 6: result = a | b;
            break;
        case 7: result = a & b;
            break;
        default:
            return -1;
        }
    }
    else if(d.opcode == OP_SYSTEM){        /* isa.c 已保证：能到这的必是 ECALL */
        if (cpu->xreg[10] == 0) {
            cpu->halted = true;              /* 正常停机：举旗，让主循环收尾 */
        } else {
            fprintf(stderr, "sim: 程序经 ECALL 报告错误 a0=%d (pc=0x%08X)\n",
                    cpu->xreg[10], cpu->pc);
            exit(EXIT_INTERNAL);             /* 错误路径：立即死在错误现场 */
        }
        writeback = false;                   /* ECALL 不产生写回值 */
    }
    else return -1;
    /* ④ 写回：全项目唯一的 x0 拦截点 */
    if (writeback && d.rd != 0)
        cpu->xreg[d.rd] = result;

    /* ⑤ 推进 */
    cpu->pc += 4;
    return 0;
}