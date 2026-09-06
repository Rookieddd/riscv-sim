/* main.c —— 命令行入口（骨架版：T6 由你完成）
 * 用法：sim <program.hex> [--trace N] [--max-cycle N] */
#include <stdio.h>
#include "sim.h"
#include "memory.h"
#include "loader.h"
#include "golden.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "用法: sim <program.hex> [--trace N] [--max-cycle N]\n");
        return EXIT_USAGE;
    }

    static memory_t mem;        /* 1MB 放静态区，不占栈空间 */
    mem_init(&mem);

    size_t n = load_hex(argv[1], &mem);
    printf("sim: 已装载 %zu 条指令\n", n);

    /* TODO(T6)：本周收口清单
     * 1) 解析 --trace N / --max-cycle N（默认 10000000）
     * 2) cpu_reset(&cpu); 循环 golden_step(&cpu, &mem) 直到 halted / 出错 / 达限
     * 3) --trace 模式：每周期打印
     *      "cycle N: pc=0x%08X inst=0x%08X <译码摘要>"
     * 4) 正常停机报告：Halted: a0=<x10>, pc=0x<pc>（week1.md ⑤-A2 的预期输出）
     * 5) golden_step 返回非 0 → exit(EXIT_BADINST)；达到 max_cycle → 报错退出
     */
    fprintf(stderr, "sim: 骨架版仅装载，执行部分等待 T2–T6 填充\n");
    return EXIT_OK;
}
