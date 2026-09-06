#ifndef SIM_H
#define SIM_H

/* 模拟器全局约定：退出码（week1.md ⑤-A 自检与 main.c/各模块共同遵守） */
enum {
    EXIT_OK         = 0,  /* 正常停机（ECALL 且 a0=0）          */
    EXIT_INTERNAL   = 1,  /* 内部错误 / 程序请求异常退出         */
    EXIT_USAGE      = 2,  /* 命令行用法错误                      */
    EXIT_LOAD       = 3,  /* 程序文件不存在或格式非法            */
    EXIT_BADINST    = 4,  /* 非法 / 尚未支持的指令               */
    EXIT_MISALIGNED = 5,  /* 非对齐访存                          */
    EXIT_OOB        = 6,  /* 访存地址越界（超出 1MB）            */
};

#endif
