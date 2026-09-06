#include "memory.h"
#include "sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 越界/非对齐统一致命出口：打印现场后按约定退出码终止。
 * （W1 教学简化：不做可恢复错误；W2 引入分支跳转后再评估。） */
static void mem_abort(uint32_t addr, int bytes, int code, const char *why)
    __attribute__((unused));   /* TODO(T2) 六个函数全部接线后可删除此行 */

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

/* ================= TODO(T2)：以下六个函数由你实现 =================
 * 硬性约定（week1.md ④-T2）：
 * 1) 小端拼字：mem[addr] 是最低字节（Week0-T5 你的答案原样升级）；
 * 2) read32/write32 先查 addr % 4 == 0，read16/write16 先查 addr % 2 == 0；
 * 3) 越界判断：addr + 宽度 > MEM_SIZE 即越界（注意 uint32 回绕，用
 *    addr > MEM_SIZE - width 的写法更稳）；
 * 4) 违规统一调 mem_abort()：非对齐传 EXIT_MISALIGNED，越界传 EXIT_OOB。
 */

uint8_t mem_read8(const memory_t *m, uint32_t addr)
{
    (void)m; (void)addr;
    /* TODO(T2): 越界检查后 return m->data[addr]; */
    return 0;
}

uint16_t mem_read16(const memory_t *m, uint32_t addr)
{
    (void)m; (void)addr;
    /* TODO(T2): 对齐 + 越界检查，然后两字节小端拼字 */
    return 0;
}

uint32_t mem_read32(const memory_t *m, uint32_t addr)
{
    (void)m; (void)addr;
    /* TODO(T2): 对齐 + 越界检查，然后四字节小端拼字 */
    return 0;
}

void mem_write8(memory_t *m, uint32_t addr, uint8_t v)
{
    (void)m; (void)addr; (void)v;
    /* TODO(T2): 越界检查后 m->data[addr] = v; */
}

void mem_write16(memory_t *m, uint32_t addr, uint16_t v)
{
    (void)m; (void)addr; (void)v;
    /* TODO(T2): 对齐 + 越界检查，按小端拆成两个字节写入 */
}

void mem_write32(memory_t *m, uint32_t addr, uint32_t v)
{
    (void)m; (void)addr; (void)v;
    /* TODO(T2): 对齐 + 越界检查，按小端拆成四个字节写入 */
}
