


program cosim_test;

  import cosim_pkg::* ;

  initial begin
    csHandle h;
    csChgInfo_t csi;
    h = csCreateCtx("abc");
    csStep(h);
    csGetCPUChg(h, 0, csi);
    $display(csi2string(csi, 1));
    csDestroyCtx(h);
  end
endprogram
