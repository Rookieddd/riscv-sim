#include "isa.h"

/* ================= TODO(T4)：由你实现 =================
 * week1.md ④-T4（本周最大块）。分发表结构建议：
 *
 *   isa_decode(inst, out):
 *     1) out->raw = inst; opcode = inst & 0x7F
 *     2) 通用字段（五种格式共用）:
 *          rd   = (inst >>  7) & 0x1F
 *          rs1  = (inst >> 15) & 0x1F
 *          rs2  = (inst >> 20) & 0x1F
 *          funct3 = (inst >> 12) & 0x07
 *          funct7 = (inst >> 25) & 0x7F
 *     3) 按 opcode 分发：
 *          OP_R      → fmt=FMT_R, imm=0
 *          OP_I      → fmt=FMT_I, imm = 算术右移符号扩展（见下）
 *          OP_LOAD   → fmt=FMT_I, imm 同上
 *          OP_STORE  → fmt=FMT_S, imm = {inst[31:25], inst[11:7]} 符号扩展
 *          OP_SYSTEM → fmt=FMT_SYS，仅当 inst == 0x00000073 时合法（ECALL）
 *          其他      → return ISA_ERR_UNSUPPORTED
 *
 * 立即数符号扩展提示（Week0-T3）：I 型 imm = (int32_t)inst >> 20;
 * S 型两段拼接后同样先左移到最高位再算术右移回原位。
 *
 * 自检：isa_decode(0x00A00093) 应得到 rd=1, rs1=0, imm=10, opcode=0x13；
 *      isa_decode(0xFFF00093) 应得到 imm=-1（附加题，先做它）。
 */
int isa_decode(uint32_t inst, decoded_t *out){   
    out->opcode = (uint8_t)(inst & 0x7F); 
    out->raw = inst;
    out->rd = (inst >>  7) & 0x1F;
    out->rs1 = (inst >> 15) & 0x1F;
    out->rs2 = (inst >> 20) & 0x1F;
    out->funct3 = (inst >> 12) & 0x07;
    out->funct7 = (inst >> 25) & 0x7F;
    if(out->opcode == OP_R){
        out->fmt = FMT_R;
        out->imm = 0;
        return ISA_OK;
    }
    else if(out->opcode == OP_I||out->opcode == OP_LOAD){
        out->fmt = FMT_I;
        out->imm = (int32_t)inst >> 20;
        return ISA_OK;
    }
    else if(out->opcode == OP_STORE){
        out->fmt = FMT_S;
        uint32_t imm12 = ((inst >> 25) << 5) | ((inst >> 7) & 0x1F);
        out->imm = (int32_t)(imm12 << 20) >> 20;
        return ISA_OK;
    }
    else if(out->opcode == OP_SYSTEM){
        if (inst == 0x00000073) {   // ECALL
            out->fmt = FMT_SYS;
            out->imm = 0;
            return ISA_OK;
        }
        return ISA_ERR_UNSUPPORTED;
    }
  
    return ISA_ERR_UNSUPPORTED;
}
