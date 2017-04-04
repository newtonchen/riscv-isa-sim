// See LICENSE for license details.
#ifndef _RISCV_COSIM_CI_H
#define _RISCV_COSIM_CI_H

#include "decode.h"
#define COSIM_MAX_OP   16
#define COSIM_MAX_INSN 1

typedef struct {
    reg_t addr;
    reg_t data;
    uint8_t access;
} csChgOP_t;

typedef enum
{
  csChgAccRdXPR,
  csChgAccWrXPR,
  csChgAccRdFPR,
  csChgAccWrFPR,
  csChgAccRdCSR,
  csChgAccWrCSR,
  csChgAccLDint8,
  csChgAccLDint16,
  csChgAccLDint32,
  csChgAccLDint64,
  csChgAccLDuint8,
  csChgAccLDuint16,
  csChgAccLDuint32,
  csChgAccLDuint64,
  csChgAccSTint8,
  csChgAccSTint16,
  csChgAccSTint32,
  csChgAccSTint64,
  csChgAccSTuint8,
  csChgAccSTuint16,
  csChgAccSTuint32,
  csChgAccSTuint64
} csChgAccess_t;

typedef struct csChgInfo__ {
    csChgOP_t ops[COSIM_MAX_OP];
    insn_bits_t ir[COSIM_MAX_INSN];
    reg_t pc[COSIM_MAX_INSN];
    uint8_t opNum, insnNum;

    void clear() { opNum = 0; insnNum = 0;}
    void access(csChgAccess_t acc, reg_t addr, reg_t value) {
        csChgOP_t op;
        if (opNum >= COSIM_MAX_OP)
            return;
        op.addr = addr;
        op.data = value;
        op.access = acc;
        ops[opNum] = op;
        opNum++;
    }
    void doInsn(reg_t pc_, insn_bits_t insn) {
        if (insnNum >= COSIM_MAX_INSN)
            return;
        ir[insnNum] = insn;
        pc[insnNum] = pc_;
        insnNum++;
    }
} csChgInfo_t;


#endif
