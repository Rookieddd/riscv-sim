/* loader.c
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
    uint32_t id = 0;
    for(int i =0 ;i < 8 ;i++ ){
        char cur = s[i];
        int temp = 0;
        if(cur >= '0' || cur <= '9')
            temp = cur - '0';
        else if(cur >= 'a' || cur <= 'f')
            temp = cur - 'a' + 10;
        else if(cur >= 'A' || cur <= 'F')
            temp = cur - 'A' + 10;
        else
            return 0;
        id =id << 4 | (uint32_t)temp;
    }
    char tail = s[8];
    if (tail != '\0' && !isspace((unsigned char)tail) && tail != '#')
    //判断一个串结尾是否合法
    //isspace 判断taili是否是' '、'\t'、'\n'、'\v'、'\f'、'\r'，如果不是，则返回0，取反,if条件成立
    
        return 0;
    *out = id;
    return 1;
}
/*
1)首先只读方式文件，如果fopen返回NULL，则输出打开失败
2)将信息存到p中，跳过无效信息（空格，空行，注释行）
  判断指令行和指令地址是否合法，写入内存，地址向后移动四位并计数
3)输出计数结果，如果没有指令，打印输出
*/
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
        while (isspace((unsigned char)*p))   
            p++;
        if (*p == '\0' || *p == '#')         
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
