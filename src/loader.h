#ifndef LOADER_H
#define LOADER_H

#include <stddef.h>
#include "memory.h"

/* 十六进制文本程序格式：
 *   每行 8 个 hex 字符 = 一条 32 位指令，从 TEXT_BASE(0x0) 顺序装载；
 *   允许空行与 '#' 开头的注释行；不支持 0x 前缀。 */
/* 成功返回装载的指令条数；
 * 文件打不开 / 行格式非法 / 超出内存 → stderr 报错并 exit(EXIT_LOAD)。 */
size_t load_hex(const char *path, memory_t *mem);

#endif
