`timescale 1 ns / 10 ps
`define COSIM_MAX_OP   16
`define COSIM_MAX_INSN 1

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

  typedef struct {
      reg_t addr;
      reg_t data;
      csChgAccess_t access;
  } csChgOP_t;

  typedef struct {
      csChgOP_t ops[`COSIM_MAX_OP];
      insn_bits_t ir[`COSIM_MAX_INSN];
      reg_t pc[`COSIM_MAX_INSN];
      uint8_t opNum, insnNum;
  } csChgInfo_t;

  import "DPI" context function chandle csCreateCtx(
                    input  string       args);
  import "DPI" context function void     csDestroyCtx(
                    input csHandle      handle);
  import "DPI" context function int      csStep(
                    input csHandle      handle, 
                    input uint8_t       pid = 0, 
                    input uint32_t      n = 1, 
                    input uint8_t       yield_lr = 0);
  import "DPI" context function int      csGetCPUChg(
                    input csHandle      handle, 
                    input uint8_t       pid, 
                    output csChgInfo_t  csi);
  import "DPI" context function int      csSetCPUChg(
                    input csHandle      handle, 
                    input uint8_t       pid, 
                    output csChgInfo_t  csi);
  import "DPI" context function void     csFesvrStop(
                    input csHandle      handle);
  import "DPI" context function void     csFesvrStart(
                    input csHandle      handle);
  import "DPI" context function void     csFesvrStep(
                    input csHandle      handle);

  function automatic string csi2string(const ref csChgInfo_t csi, input bit verb = 0);
      csi2string = "";
      for(int i = 0; i < csi.insnNum; i++)
        csi2string = $psprintf("%spc: 0x%h,\tir: 0x%h\n", csi2string, csi.pc[i], csi.ir[i]); 
      if (verb)
        for(int i = 0; i < csi.opNum; i++)
            csi2string = $psprintf("%sop%3d : %s \taddr: 0x%h\tdata:0x%h\n", csi2string, i, csi.ops[i].access.name, csi.ops[i].addr, csi.ops[i].data); 
  endfunction
endpackage
