#ifndef GOLDEN_H
#define GOLDEN_H

#include <stdint.h>
#include <stdbool.h>
#include "memory.h"

/* 单周期参考模型（计划书 §3.5）：只保证 ISA 正确性，无流水线、无 cache。
 * 架构原则：golden 与 W3 的 pipeline 共用同一份 isa.c 语义。 */
typedef struct {
    uint32_t pc;
    int32_t  xreg[32];
    bool     halted;
} cpu_t;

/* 复位：pc=0、寄存器全零、SP(x2)=STACK_TOP（计划书 §3.2） */
void cpu_reset(cpu_t *cpu);

/* 执行一条指令。返回 0 成功；非 0 = 非法/未支持指令（main 映射退出码 4）。
 * 停机契约：ECALL 且 a0(x10)==0 → cpu->halted = true；
 *          a0 != 0 → stderr 打印并 exit(EXIT_INTERNAL)（riscv-tests 风格）。 */
int golden_step(cpu_t *cpu, memory_t *mem);

#endif
