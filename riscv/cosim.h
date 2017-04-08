// See LICENSE for license details.
#ifndef _RISCV_COSIM_H
#define _RISCV_COSIM_H

#include "decode.h"
#include "cosim_ci.h"
#include "processor.h"
#include "cachesim.h"
#include <memory>

typedef struct {int dummy;} *csHandle;
class gdbserver_t;

class cosim {
public:
static csHandle csCreateCtx(int, char **);
static void     csDestroyCtx(csHandle&);
static int      csStep(csHandle, uint8_t, csAsynEvt_t);
static int      csGetCPUChg(csHandle, uint8_t, csChgInfo_t&);
static int      csExecCPUop(csHandle, uint8_t, csChgOP_t&);
static void     csFesvrStart(csHandle&);
static void     csFesvrStep(csHandle&);
static void     csFesvrStop(csHandle&);
static void     csGDBStep(csHandle&);
static std::unique_ptr<gdbserver_t> gdbserver;
static std::unique_ptr<icache_sim_t> ic;
static std::unique_ptr<dcache_sim_t> dc;
static std::unique_ptr<cache_sim_t> l2;
static uint16_t gdb_port;
};

#endif
