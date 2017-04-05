


program cosim_test;

  import cosim_pkg::* ;

  export "DPI" function                  cs_mem_write;

  function void cs_mem_write(input reg_t addr, reg_t data);
      $display("htif write %h = %h", addr, data);
  endfunction
    
  initial begin
    csHandle h;
    csChgInfo_t csi;
    csChgOP_t   op;
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
    for (int i = 0; i < 100; i++) begin
        csStep(h);
        csGetCPUChg(h, 0, csi);
        $display(csi2string(csi, 1));
    end
    csDestroyCtx(h);
  end
endprogram
