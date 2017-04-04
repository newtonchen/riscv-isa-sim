// See LICENSE for license details.
#ifndef _RISCV_COSIM_H
#define _RISCV_COSIM_H

#include "decode.h"
#include "cosim_ci.h"
#include "processor.h"

typedef struct {int dummy;} *csHandle;

class cosim {
public:
static csHandle csCreateCtx(int, char **);
static void     csDestroyCtx(csHandle&);
static int      csStep(csHandle, uint8_t, uint32_t, uint8_t);
static int      csGetCPUChg(csHandle, uint8_t, csChgInfo_t&);
static int      csSetCPUChg(csHandle, uint8_t, csChgInfo_t&);
static void     csFesvrStart(csHandle&);
static void     csFesvrStep(csHandle&);
static void     csFesvrStop(csHandle&);
};

#endif
