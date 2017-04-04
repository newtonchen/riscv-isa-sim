


program cosim_test;

  import cosim_pkg::* ;

  export "DPI" function                  cs_mem_write;

  function void cs_mem_write(input reg_t addr, reg_t data);
      $display("htif write %h = %h", addr, data);
  endfunction
    
  initial begin
    csHandle h;
    csChgInfo_t csi;
    h = csCreateCtx("spike /opt/riscv/riscv64-unknown-elf/share/riscv-tests/isa/rv64um-v-mulh");
    csFesvrStart(h);
    for (int i = 0; i < 100; i++) begin
        csStep(h);
        csGetCPUChg(h, 0, csi);
        $display(csi2string(csi, 1));
    end
    csDestroyCtx(h);
  end
endprogram
