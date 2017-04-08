// See LICENSE for license details.
#ifndef _RISCV_COSIM_CI_H
#define _RISCV_COSIM_CI_H

#include "decode.h"
#define COSIM_MAX_OP   16
#define COSIM_MAX_INSN 4

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
  csChgAccSTuint8,
  csChgAccSTuint16,
  csChgAccSTuint32,
  csChgAccSTuint64
} csChgAccess_t;

typedef struct csTrapInfo__ {
  uint8_t   trp;
  uint8_t   dbg;
  uint8_t   prv;
  uint8_t   cause;
  uint8_t   dcause;
  reg_t     badaddr;
} csTrapInfo_t;

typedef struct {
    reg_t addr;
    reg_t data;
    uint8_t access;
    uint8_t iNum;
} csChgOP_t;

typedef struct csChgInsn__ {
    uint8_t         prv;
    reg_t           pc;
    insn_bits_t     ir;
} csChgInsn_t;

typedef struct csChgInfo__ {
    uint8_t         opNum, iNum;
    csChgInsn_t     insn[COSIM_MAX_INSN];
    csChgOP_t       ops[COSIM_MAX_OP];
    csTrapInfo_t    ti;

    void clear() { opNum = 0; iNum = 0; ti.trp = 0; ti.dbg = 0;}
    void access(csChgAccess_t acc, reg_t addr, reg_t value) {
        csChgOP_t op;
        if (opNum >= COSIM_MAX_OP)
            return;
        op.addr    = addr;
        op.data    = value;
        op.access  = acc;
        op.iNum    = iNum;
        ops[opNum] = op;
        opNum++;
    }
    void doInsn(reg_t pc_, uint8_t priv_, insn_bits_t insn_) {
        csChgInsn_t ins;
        if (iNum >= COSIM_MAX_INSN)
            return;
        ins.ir      = insn_;
        ins.pc      = pc_;
        ins.prv     = priv_;
        insn[iNum]  = ins;
        iNum++;
    }
} csChgInfo_t;


typedef struct {
    uint32_t irqPend;
    uint8_t  busErr;
    uint8_t  tcmErr;
    uint8_t  yield_lr;
} csAsynEvt_t;

#endif
