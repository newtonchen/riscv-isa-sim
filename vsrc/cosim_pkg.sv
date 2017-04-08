`timescale 1 ns / 10 ps
`define COSIM_MAX_OP   16
`define COSIM_MAX_INSN 4

package cosim_pkg;

  typedef longint   unsigned  uint64_t;
  typedef int       unsigned  uint32_t;
  typedef shortint  unsigned  uint16_t;
  typedef byte      unsigned  uint8_t;
  typedef longint   signed    int64_t;
  typedef int       signed    int32_t;
  typedef shortint  signed    int16_t;
  typedef byte      signed    int8_t;
  typedef uint64_t            reg_t;
  typedef uint64_t            insn_bits_t;
  typedef chandle csHandle;

  typedef enum uint8_t
  {
    cs_RdXPR,
    cs_WrXPR,
    cs_RdFPR,
    cs_WrFPR,
    cs_RdCSR,
    cs_WrCSR,
    cs_LDint8,
    cs_LDint16,
    cs_LDint32,
    cs_LDint64,
    cs_LDuint8,
    cs_LDuint16,
    cs_LDuint32,
    cs_LDuint64,
    cs_STuint8,
    cs_STuint16,
    cs_STuint32,
    cs_STuint64
  } csChgAccess_t;

  typedef enum uint8_t
  {
    CAUSE_MISALIGNED_FETCH,
    CAUSE_FAULT_FETCH,
    CAUSE_ILLEGAL_INSTRUCTION,
    CAUSE_BREAKPOINT,
    CAUSE_MISALIGNED_LOAD,
    CAUSE_FAULT_LOAD,
    CAUSE_MISALIGNED_STORE,
    CAUSE_FAULT_STORE,
    CAUSE_USER_ECALL,
    CAUSE_SUPERVISOR_ECALL,
    CAUSE_HYPERVISOR_ECALL,
    CAUSE_MACHINE_ECALL
  } csTrapCause_t;

  typedef enum uint8_t
  {
    DCSR_CAUSE_NONE,
    DCSR_CAUSE_SWBP,
    DCSR_CAUSE_HWBP,
    DCSR_CAUSE_DEBUGINT,
    DCSR_CAUSE_STEP,
    DCSR_CAUSE_HALT
  } csDCause_t;

  typedef enum uint8_t
  {
    PRV_U,
    PRV_S,
    PRV_H,
    PRV_M
  } csPRV_t;

  typedef struct {
    uint8_t         trp;
    uint8_t         dbg;
    csPRV_t         prv;
    csTrapCause_t   cause;
    csDCause_t      dcause;
    reg_t           badaddr;
  } csTrapInfo_t;

  typedef struct {
      reg_t addr;
      reg_t data;
      csChgAccess_t access;
      uint8_t iNum;
  } csChgOP_t;

  typedef struct {
      csPRV_t         prv;
      reg_t           pc;
      insn_bits_t     ir;
  } csChgInsn_t;

  typedef struct {
      uint8_t         opNum, iNum;
      csChgInsn_t     insn[`COSIM_MAX_INSN];
      csChgOP_t       ops[`COSIM_MAX_OP];
      csTrapInfo_t    ti;
  } csChgInfo_t;

  typedef struct {
    uint32_t irqPend;
    uint8_t  busErr;
    uint8_t  tcmErr;
    uint8_t  yield_lr;
  } csAsynEvt_t;

  import "DPI" function chandle csCreateCtx(
                    input  string       args);
  import "DPI" function void     csDestroyCtx(
                    inout csHandle      handle);
  import "DPI" function int      csStep(
                    input csHandle      handle, 
                    input uint8_t       pid, 
                    input csAsynEvt_t   aevt);
  import "DPI" function int      csGetCPUChg(
                    input csHandle      handle, 
                    input uint8_t       pid, 
                    output csChgInfo_t  csi);
  import "DPI" function int      csExecCPUop(
                    input csHandle      handle, 
                    input uint8_t       pid, 
                    inout csChgOP_t     csi);
  import "DPI" function void     csFesvrStop(
                    input csHandle      handle);
  import "DPI" function void     csFesvrStart(
                    input csHandle      handle);
  import "DPI" function void     csFesvrStep(
                    input csHandle      handle);
  export "DPI" function                  cs_mem_write;

  function void cs_mem_write(input reg_t addr, reg_t data);
      $display("htif write %h = %h", addr, data);
  endfunction

  function automatic string csi2string(const ref csChgInfo_t csi, input bit verb = 0);
    csi2string = "";
    for(int i = 0; i < csi.iNum; i++)
        csi2string = $psprintf("%sinsn%3d, %s pc: 0x%h,\tir: 0x%h\n", csi2string, i, csi.insn[i].prv.name, csi.insn[i].pc, csi.insn[i].ir); 
    if (csi.ti.trp != 0)
        csi2string = $psprintf("%sTrap:\nCause: %s,\t%s, badaddr: 0x%h\n", csi2string, csi.ti.cause, csi.ti.prv.name, csi.ti.badaddr); 
    if (csi.ti.dbg != 0)
        csi2string = $psprintf("%sDebug:\nCause: %s\n", csi2string, csi.ti.dcause); 

    if (!verb)
        return csi2string;
    for(int i = 0; i < csi.opNum; i++)
        csi2string = $psprintf("%sop%3d,%3d : %s \taddr: 0x%h\tdata:0x%h\n", csi2string, i, csi.iNum, csi.ops[i].access.name, csi.ops[i].addr, csi.ops[i].data); 
  endfunction
endpackage
