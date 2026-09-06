#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

/* 1MB 平坦内存（计划书 §3.2）：单块连续数组，小端序 */
#define MEM_SIZE (1u << 20)

/* 内存布局：文本段从 0 起装载；数据段、栈区为 W2+ 的 .bin 程序预留 */
#define TEXT_BASE 0x00000000u
#define DATA_BASE 0x00010000u
#define STACK_LOW  0x00080000u
#define STACK_TOP  0x00100000u   /* SP 复位值，向下生长 */

typedef struct {
    uint8_t data[MEM_SIZE];
} memory_t;

/* 清零整块内存（程序每次运行从干净状态开始） */
void mem_init(memory_t *m);

/* 按字节/半字/字读写。
 * 契约（T2 实现时必须遵守）：
 * 1) 小端：低地址放低字节，mem[addr] 是最低 8 位；
 * 2) 32 位访问要求 addr % 4 == 0，16 位要求 addr % 2 == 0；
 * 3) addr 越界（addr + 宽度 > MEM_SIZE）同样致命；
 * 4) 非对齐以 EXIT_MISALIGNED 终止，越界以 EXIT_OOB 终止。 */
uint8_t  mem_read8 (const memory_t *m, uint32_t addr);
uint16_t mem_read16(const memory_t *m, uint32_t addr);
uint32_t mem_read32(const memory_t *m, uint32_t addr);
void mem_write8 (memory_t *m, uint32_t addr, uint8_t  v);
void mem_write16(memory_t *m, uint32_t addr, uint16_t v);
void mem_write32(memory_t *m, uint32_t addr, uint32_t v);

#endif
