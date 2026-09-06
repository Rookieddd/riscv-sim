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
    (void)cpu;
    (void)mem;
    return -1;
}
