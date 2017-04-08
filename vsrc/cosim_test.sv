


program cosim_test;

  import cosim_pkg::* ;

  export "DPI" function                  htif_mem_write;
  export "DPI" function                  dbg_mem_write;

  function void htif_mem_write(input reg_t addr, reg_t data);
      $display("htif write %h = %h", addr, data);
  endfunction

  function void dbg_mem_write(input uint32_t addr, uint32_t data);
      $display("dbg write %h = %h", addr, data);
  endfunction

  initial begin
    csHandle h;
    csChgInfo_t csi;
    csChgOP_t   op;
    csAsynEvt_t aevt = '{default : 0};
    string arg = "spike", exec;
    if ($value$plusargs("exec=%s", exec))
      arg = {arg, " ", exec};                       
    h = csCreateCtx(arg);
    //"spike /opt/riscv/riscv64-unknown-elf/share/riscv-tests/isa/rv64um-v-mulh");
    csFesvrStart(h);
    op.access = cs_WrXPR;
    op.addr   = 'h1;
    op.data   = 'hdeadbeef;
    csExecCPUop(h, 0, op);
    op.access = cs_RdXPR;
    op.addr   = 'h1;
    op.data   = 'h0;
    csExecCPUop(h, 0, op);
    $display("read result is %h\n", op.data);
    //for (int i = 0; i < 10; i++) begin
    //    csStep(h, 0, aevt);
    //    csGetCPUChg(h, 0, csi);
    //    $display(csi2string(csi, 1));
    //end
    while(1) begin
        for (int i = 0; i < 2000; i++) begin
            csStep(h, 0, aevt);
        end
        //$display(".");
        csFesvrStep(h);
        csGDBStep(h);
    end
    csGDBStep(h);
    csDestroyCtx(h);
  end
endprogram
