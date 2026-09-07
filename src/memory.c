#include "memory.h"
#include "sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void mem_abort(uint32_t addr, int bytes, int code, const char *why)
{
    fprintf(stderr, "memory: 非法访问 addr=0x%08X (%d 字节): %s\n",
            addr, bytes, why);
    exit(code);
}

void mem_init(memory_t *m)
{
    memset(m->data, 0, sizeof m->data);
}

/*
 * 1) 小端拼字：mem[addr] 是最低字节；
        示例：32位整数 0x12345678 的存储方式
        大端：低地址→高地址 [12] [34] [56] [78] 
        小端：低地址→高地址 [78] [56] [34] [12]
 * 2) read32/write32 先查 addr % 4 == 0，read16/write16 先查 addr % 2 == 0；
 * 3) 越界判断：addr + 宽度 > MEM_SIZE 即越界（注意 uint32 回绕，用
 *    addr > MEM_SIZE - width 的写法更稳）；
 * 4) 违规统一调 mem_abort()：非对齐传 EXIT_MISALIGNED，越界传 EXIT_OOB。
 */

uint8_t mem_read8(const memory_t *m, uint32_t addr)
{
    if (addr >= MEM_SIZE) {
        mem_abort(addr, 1, EXIT_OOB, "越界访问");
    }
    return m->data[addr];
}

uint16_t mem_read16(const memory_t *m, uint32_t addr)
{
    if (addr % 2 != 0)
        mem_abort(addr, 2, EXIT_MISALIGNED, "数据未对齐");
    if (addr > MEM_SIZE - 2)
        mem_abort(addr, 2, EXIT_OOB, "越界访问");
    return (uint16_t)((uint32_t)m->data[addr] |
                     ((uint32_t)m->data[addr + 1] << 8));
}

uint32_t mem_read32(const memory_t *m, uint32_t addr)
{
    if (addr % 4 != 0)
        mem_abort(addr, 4, EXIT_MISALIGNED, "数据未对齐");
    if (addr > MEM_SIZE - 4)
        mem_abort(addr, 4, EXIT_OOB, "越界访问");
    return (uint32_t)m->data[addr] |
           ((uint32_t)m->data[addr + 1] << 8) |
           ((uint32_t)m->data[addr + 2] << 16) |
           ((uint32_t)m->data[addr + 3] << 24);
}

void mem_write8(memory_t *m, uint32_t addr, uint8_t v)
{
    if (addr >= MEM_SIZE) {
        mem_abort(addr, 1, EXIT_OOB, "越界访问");
    }
    m->data[addr] = v;
}

void mem_write16(memory_t *m, uint32_t addr, uint16_t v)
{
    if (addr % 2 != 0)
        mem_abort(addr, 2, EXIT_MISALIGNED, "数据未对齐");
    if (addr > MEM_SIZE - 2)
        mem_abort(addr, 2, EXIT_OOB, "越界访问");
    m->data[addr] = (uint8_t)(v & 0xff);                  
    m->data[addr + 1] = (uint8_t)((v >> 8) & 0xff);       
}

void mem_write32(memory_t *m, uint32_t addr, uint32_t v)
{
    if (addr % 4 != 0)
        mem_abort(addr, 4, EXIT_MISALIGNED, "数据未对齐");
    if (addr > MEM_SIZE - 4)
        mem_abort(addr, 4, EXIT_OOB, "越界访问");
    m->data[addr] = (uint8_t)(v & 0xff);
    m->data[addr + 1] = (uint8_t)((v >> 8) & 0xff);
    m->data[addr + 2] = (uint8_t)((v >> 16) & 0xff);
    m->data[addr + 3] = (uint8_t)((v >> 24) & 0xff);
}
