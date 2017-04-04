#include "../riscv/cosim.h"
#include <stdint.h>
extern "C"
{
void cs_mem_write(uint64_t, uint64_t) {};
}



extern "C" {
csHandle csCreateCtx(char * s);
void     csDestroyCtx(csHandle a); 
int      csStep(csHandle a, uint8_t b, uint32_t c, uint8_t d); 
void     csFesvrStart(csHandle a); 
}


int main(int argc, char** argv)
{
    csHandle h;
    char cmd[256] = "spike /opt/riscv/riscv64-unknown-elf/share/riscv-tests/isa/rv64um-v-mulh";
    h = csCreateCtx(cmd);
    csFesvrStart(h);
    csStep(h, 0, 1, 0);
    csDestroyCtx(h);
}
