#ifndef ISA_H
#define ISA_H

#include <stdint.h>

/* opcode 常量（inst[6:0]）。W1 译码范围：R / I / S + ECALL；
 * 0x63 分支(B 型)、0x67 JALR、0x6F JAL(J 型) 与 U 型属 W2。 */
#define OP_R      0x33u  /* ADD SUB AND OR XOR      */
#define OP_I      0x13u  /* ADDI ANDI ORI XORI      */
#define OP_LOAD   0x03u  /* LW（W1 只译码，W2 执行） */
#define OP_STORE  0x23u  /* SW（S 型，W1 只译码）    */
#define OP_SYSTEM 0x73u  /* ECALL                    */

/* 指令格式（P&H 图 2.3），译码层负责判定并填入 decoded_t.fmt */
typedef enum {
    FMT_R, FMT_I, FMT_S, FMT_B, FMT_U, FMT_J, FMT_SYS,
} isa_fmt_t;

/* 译码结果。约定：imm 一律在本层完成符号扩展，
 * 上层（golden / 未来的流水线）拿到的直接是带符号真值。 */
typedef struct {
    uint32_t  raw;
    uint8_t   opcode;
    uint8_t   rd, rs1, rs2;   /* 5 位寄存器编号 */
    uint8_t   funct3, funct7;
    int32_t   imm;
    isa_fmt_t fmt;
} decoded_t;

enum {
    ISA_OK = 0,
    ISA_ERR_UNSUPPORTED = -1,  /* 不认识的 opcode（含 W2 范围的 B/J 型） */
};

/* 译码：成功返回 ISA_OK 并填充 *out；不认识返回 ISA_ERR_UNSUPPORTED。 */
int isa_decode(uint32_t inst, decoded_t *out);

#endif
