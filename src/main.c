/* main.c ——【助手示范实现】（week1.md ④-T6）
 * 使用方式（与 T3 loader 相同）：读懂每一行 → 合上本文件独立重写 →
 * 以你自己的版本替换，提交时在 message 里写明你与示范的差异点。
 *
 * main 的职责边界（复试素材）：只做"编排"——解析人类意图（命令行）、
 * 驱动各模块按正确顺序协作、向人类汇报结果。它不含任何 ISA 知识：
 * 本文件里不允许出现 0x33 / funct3 / rd 这类字眼（trace 打印 raw 值除外）。
 *
 * 用法: sim <program.hex> [--trace N] [--max-cycle N]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim.h"
#include "memory.h"
#include "loader.h"
#include "golden.h"

/* ---------- T6.1 参数解析 ----------
 * 约定：argv[0]=程序名, argv[1]=程序路径, 其余是开关。
 * 解析结果经指针参数带回；遇到不认识/缺参的开关 → 报用法错并退出。
 * strtol(s,NULL,10)：把字符串按十进制转成 long——W1 从简不查 errno，
 * 但"参数必须是数字"由格式约定保证。 */
static void parse_args(int argc, char **argv,
                       long *trace_limit, long *max_cycle)
{
    *trace_limit = 0;          /* 0 = 不输出 trace */
    *max_cycle   = 10000000;   /* 周期保险丝：默认一千万（防死循环） */

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--trace") == 0) {
            if (i + 1 >= argc) {                       /* 开关后没跟数字 */
                fprintf(stderr, "sim: --trace 需要一个数字参数\n");
                exit(EXIT_USAGE);
            }
            *trace_limit = strtol(argv[++i], NULL, 10);
        } else if (strcmp(argv[i], "--max-cycle") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "sim: --max-cycle 需要一个数字参数\n");
                exit(EXIT_USAGE);
            }
            *max_cycle = strtol(argv[++i], NULL, 10);
            if (*max_cycle <= 0) {                     /* 0/负数没有意义 */
                fprintf(stderr, "sim: --max-cycle 必须为正整数\n");
                exit(EXIT_USAGE);
            }
        } else {
            fprintf(stderr, "sim: 未知参数 %s\n", argv[i]);
            exit(EXIT_USAGE);
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "用法: sim <program.hex> [--trace N] [--max-cycle N]\n");
        return EXIT_USAGE;
    }

    long trace_limit, max_cycle;
    parse_args(argc, argv, &trace_limit, &max_cycle);

    /* ---------- 装载阶段（一次性）----------
     * memory_t 放 static 区：1MB 数组太大，不进栈。
     * load_hex 出错时自己 exit(EXIT_LOAD)，能返回即代表成功。 */
    static memory_t mem;
    mem_init(&mem);
    size_t n_inst = load_hex(argv[1], &mem);
    printf("sim: 已装载 %zu 条指令\n", n_inst);

    /* ---------- 执行阶段（主循环）----------
     * 循环条件是 halted 标志：主循环不认识 ECALL，它只看"执行器说停了没"。
     * golden_step 返回非 0 = 非法/未支持指令 → 按契约映射退出码 4。 */
    cpu_t cpu;
    cpu_reset(&cpu);

    unsigned long cycle = 0;                 /* 已完成的周期数 */
    while (!cpu.halted) {
        /* T6.3 trace：打印"将要执行"的一条（cycle 从 1 数起）。
         * 取指读两次（这里一次、golden_step 里一次）——W1 从简，
         * 注释说明即可，不值得为此改接口。 */
        if (cycle < (unsigned long)trace_limit)
            printf("cycle %lu: pc=0x%08X inst=0x%08X\n",
                   cycle + 1, cpu.pc, mem_read32(&mem, cpu.pc));

        if (golden_step(&cpu, &mem) != 0) {
            fprintf(stderr, "sim: cycle %lu 于 pc=0x%08X 遇到不支持/非法指令\n",
                    cycle + 1, cpu.pc);
            exit(EXIT_BADINST);
        }
        cycle++;

        /* 周期保险丝：先干活再查表。程序死循环时保证能退出。 */
        if (cycle >= (unsigned long)max_cycle) {
            fprintf(stderr, "sim: 达到最大周期数 %ld，强制停机（疑似死循环）\n",
                    max_cycle);
            exit(EXIT_INTERNAL);
        }
    }

    /* ---------- T6.4 停机报告 ----------
     * a0 = x10（ECALL 约定用它传结果）；
     * pc 停在 ECALL 的下一条（T5 的"停机也推进 pc"在这里兑现）。 */
    printf("Halted: a0=%d, pc=0x%08X\n", cpu.xreg[10], cpu.pc);
    printf("sim: 共 %lu 周期 / %zu 条指令\n", cycle, n_inst);
    return EXIT_OK;
}
