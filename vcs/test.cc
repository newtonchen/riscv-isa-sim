#include "../riscv/cosim.h"


extern "C" {
csHandle csCreateCtx(char * s);
void     csDestroyCtx(csHandle a); 
int      csStep(csHandle a, uint8_t b, uint32_t c, uint8_t d); 
}


int main(int argc, char** argv)
{
    csHandle h;
    char cmd[64] = "abc";
    h = csCreateCtx(cmd);
    csStep(h, 0, 1, 0);
    csDestroyCtx(h);
}
