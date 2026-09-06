/* loader.c —— 【助手示范实现】（week1.md ④-T3）
 * 使用方式：读懂每一行 → 合上本文件独立重写 → 用你的版本替换整个文件，
 *           以 feat(loader) 提交，并在 commit message 里写明你与示范的差异点。
 * 防御式解析四原则（T3 概念题 C3 的答案素材）：
 *   1) 打不开的文件立刻报错，绝不返回"成功"；
 *   2) 每一行都验证格式，非法行带行号报告；
 *   3) 装载越界视为致命错误而非静默截断；
 *   4) 空程序（0 条指令）同样是错误。 */

#include "loader.h"
#include "sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* 解析一行开头的 8 个 hex 字符。
 * 成功：*out = 解析值，返回 1；该行不是合法指令 → 返回 0。
 * 校验点：恰好 8 位 hex；第 9 个字符必须是行尾/空白/注释符。 */
static int parse_hex_word(const char *s, uint32_t *out)
{
    uint32_t v = 0;

    for (int i = 0; i < 8; i++) {
        char c = s[i];
        int d;

        if (c >= '0' && c <= '9')
            d = c - '0';
        else if (c >= 'a' && c <= 'f')
            d = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
            d = c - 'A' + 10;
        else
            return 0;
        v = (v << 4) | (uint32_t)d;
    }

    char tail = s[8];
    if (tail != '\0' && tail != '\n' && tail != '\r' &&
        !isspace((unsigned char)tail) && tail != '#')
        return 0;   /* 8 位之后还有别的字符：按非法行处理 */

    *out = v;
    return 1;
}

size_t load_hex(const char *path, memory_t *mem)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "loader: 无法打开文件 %s\n", path);
        exit(EXIT_LOAD);
    }

    char line[128];
    size_t count = 0;
    uint32_t addr = TEXT_BASE;
    int lineno = 0;

    while (fgets(line, sizeof line, fp) != NULL) {
        lineno++;

        const char *p = line;
        while (isspace((unsigned char)*p))   /* 跳过行首空白 */
            p++;
        if (*p == '\0' || *p == '#')         /* 空行 / 注释行 */
            continue;

        uint32_t inst;
        if (!parse_hex_word(p, &inst)) {
            fprintf(stderr, "loader: %s 第 %d 行不是合法的指令行\n", path, lineno);
            exit(EXIT_LOAD);
        }
        if (addr > MEM_SIZE - 4) {           /* 装载将越过内存末尾 */
            fprintf(stderr, "loader: %s 第 %d 行超出内存容量\n", path, lineno);
            exit(EXIT_LOAD);
        }

        mem_write32(mem, addr, inst);
        addr += 4;
        count++;
    }
    fclose(fp);

    if (count == 0) {
        fprintf(stderr, "loader: %s 中没有任何指令\n", path);
        exit(EXIT_LOAD);
    }
    return count;
}
