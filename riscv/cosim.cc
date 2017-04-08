#include "cosim.h"
#include "sim.h"
#include "mmu.h"
#include "gdbserver.h"
#include "extension.h"
#include <dlfcn.h>
#include <fesvr/option_parser.h>
#include <fesvr/device.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <memory>

static void help(){
  fprintf(stderr, "usage: spike cosim [host options]\n");
  fprintf(stderr, "Host Options:\n");
  fprintf(stderr, "  -p<n>                 Simulate <n> processors [default 1]\n");
  fprintf(stderr, "  -m<n>                 Provide <n> MiB of target memory [default 4096]\n");
  fprintf(stderr, "  -g                    Track histogram of PCs\n");
  fprintf(stderr, "  -h                    Print this help message\n");
  fprintf(stderr, "  -H                 Start halted, allowing a debugger to connect\n");
  fprintf(stderr, "  --isa=<name>          RISC-V ISA string [default %s]\n", DEFAULT_ISA);
  fprintf(stderr, "  --ic=<S>:<W>:<B>      Instantiate a cache model with S sets,\n");
  fprintf(stderr, "  --dc=<S>:<W>:<B>        W ways, and B-byte blocks (with S and\n");
  fprintf(stderr, "  --l2=<S>:<W>:<B>        B both powers of 2).\n");
  fprintf(stderr, "  --extension=<name>    Specify RoCC Extension\n");
  fprintf(stderr, "  --extlib=<name>       Shared library to load\n");
  fprintf(stderr, "  --gdb-port=<port>  Listen on <port> for gdb to connect\n");
  fprintf(stderr, "  --dump-config-string  Print platform configuration string and exit\n");
}

std::unique_ptr<gdbserver_t>  cosim::gdbserver;
std::unique_ptr<icache_sim_t> cosim::ic;
std::unique_ptr<dcache_sim_t> cosim::dc;
std::unique_ptr<cache_sim_t>  cosim::l2;
uint16_t cosim::gdb_port;

extern "C" {
csHandle csCreateCtx(char * s) {
    enum { kMaxArgs = 64 };
    int argc = 0;
    char *argv[kMaxArgs];
    
    char *p2 = strtok(s, " ");
    while (p2 && argc < kMaxArgs-1)
      {
        argv[argc++] = p2;
        p2 = strtok(0, " ");
      }
    argv[argc] = 0;
    return cosim::csCreateCtx(argc, argv);
}
void     csDestroyCtx(csHandle* a) { 
    cosim::csDestroyCtx(*a); 
}
int      csStep(csHandle a, uint8_t b, csAsynEvt_t c) { 
    return cosim::csStep(a, b, c);
}
int      csGetCPUChg(csHandle a, uint8_t b, csChgInfo_t* c) {
    return cosim::csGetCPUChg(a, b, *c);
}
int      csExecCPUop(csHandle a, uint8_t b, csChgOP_t* c) {
    return cosim::csExecCPUop(a, b, *c);
}
void     csFesvrStep(csHandle a) { 
    cosim::csFesvrStep(a); 
}
void     csFesvrStart(csHandle a) { 
    cosim::csFesvrStart(a); 
}
void     csFesvrStop(csHandle a) { 
    cosim::csFesvrStop(a); 
}
void     csGDBStep(csHandle a) { 
    cosim::csGDBStep(a); 
}
}

csHandle cosim::csCreateCtx(int argc, char ** argv){
  bool debug = false;
  bool halted = false;
  bool histogram = false;
  bool log = false;
  bool dump_config_string = false;
  size_t nprocs = 1;
  size_t mem_mb = 0;
  std::function<extension_t*()> extension;
  const char* isa = DEFAULT_ISA;
  gdb_port = 0;

  option_parser_t parser;
  parser.help(&help);
  parser.option('h', 0, 0, [&](const char* s){help();});
  parser.option('d', 0, 0, [&](const char* s){debug = true;});
  parser.option('g', 0, 0, [&](const char* s){histogram = true;});
  parser.option('l', 0, 0, [&](const char* s){log = true;});
  parser.option('p', 0, 1, [&](const char* s){nprocs = atoi(s);});
  parser.option('m', 0, 1, [&](const char* s){mem_mb = atoi(s);});
  // I wanted to use --halted, but for some reason that doesn't work.
  parser.option('H', 0, 0, [&](const char* s){halted = true;});
  parser.option(0, "gdb-port", 1, [&](const char* s){gdb_port = atoi(s);});
  parser.option(0, "ic", 1, [&](const char* s){ic.reset(new icache_sim_t(s));});
  parser.option(0, "dc", 1, [&](const char* s){dc.reset(new dcache_sim_t(s));});
  parser.option(0, "l2", 1, [&](const char* s){l2.reset(cache_sim_t::construct(s, "L2$"));});
  parser.option(0, "isa", 1, [&](const char* s){isa = s;});
  parser.option(0, "extension", 1, [&](const char* s){extension = find_extension(s);});
  parser.option(0, "dump-config-string", 0, [&](const char *s){dump_config_string = true;});
  parser.option(0, "extlib", 1, [&](const char *s){
    void *lib = dlopen(s, RTLD_NOW | RTLD_GLOBAL);
    if (lib == NULL) {
      fprintf(stderr, "Unable to load extlib '%s': %s\n", s, dlerror());
      exit(-1);
    }
  });

  auto argv1 = parser.parse(argv);
  std::vector<std::string> htif_args(argv1, (const char*const*)argv + argc);
  sim_t* s = new sim_t(isa, nprocs, mem_mb, halted, htif_args);
  if (gdb_port) {
    gdbserver = std::unique_ptr<gdbserver_t>(new gdbserver_t(gdb_port, s));
    s->set_gdbserver(&(*gdbserver));
  }

  if (dump_config_string) {
    printf("%s", s->get_config_string());
    return 0;
  }

  if (!*argv1)
    help();

  if (ic && l2) ic->set_miss_handler(&*l2);
  if (dc && l2) dc->set_miss_handler(&*l2);
  for (size_t i = 0; i < nprocs; i++)
  {
    if (ic) s->get_core(i)->get_mmu()->register_memtracer(&*ic);
    if (dc) s->get_core(i)->get_mmu()->register_memtracer(&*dc);
    if (extension) s->get_core(i)->register_extension(extension());
  }

  s->set_debug(debug);
  s->set_log(log);
  s->set_histogram(histogram);

  return (reinterpret_cast<csHandle>(s));
};


void cosim::csDestroyCtx(csHandle& handle){
  sim_t* s = reinterpret_cast<sim_t*>(handle);
  if (s != NULL)
      free(s);
  handle = NULL;
}

int cosim::csStep(csHandle handle, uint8_t pid, csAsynEvt_t aevt){
  sim_t* s = reinterpret_cast<sim_t*>(handle);
  if (s == NULL) return 0;
  if (pid >= s->procs.size()) return 0;
  processor_t& p = *s->procs[pid];
  if (aevt.yield_lr)
    p.yield_load_reservation();
  p.csi.clear();
  p.aevt = aevt;
  p.step(1);
  return 1;
}

int cosim::csGetCPUChg(csHandle handle, uint8_t pid, csChgInfo_t& csi) {
  sim_t* s = reinterpret_cast<sim_t*>(handle);
  if (s == NULL) return 0;
  if (pid >= s->procs.size()) return 0;
  csi = s->procs[pid]->csi;
  return 1;
}

int cosim::csExecCPUop(csHandle handle, uint8_t pid, csChgOP_t& op) {
  sim_t* s = reinterpret_cast<sim_t*>(handle);
  if (s == NULL) return 0;
  if (pid >= s->procs.size()) return 0;
  processor_t& p = *s->procs[pid];
  switch(op.access){
    case csChgAccRdXPR:
        if (op.addr >= NXPR)
            break;
        op.data = p.state.XPR[op.addr];
        return 1;
    case csChgAccWrXPR:
        if (op.addr >= NXPR)
            break;
        p.state.XPR.write(op.addr, op.data);
        return 1;
    case csChgAccRdFPR:
        if (op.addr >= NFPR)
            break;
        op.data = p.state.FPR[op.addr];
        return 1;
    case csChgAccWrFPR:
        if (op.addr >= NFPR)
            break;
        p.state.FPR.write(op.addr, op.data);
        return 1;
    case csChgAccRdCSR:
        op.data = p.get_csr__(op.addr);
        return 1;
    case csChgAccWrCSR:
        p.set_csr__(op.addr, op.data);
        return 1;
    case csChgAccLDint8:
        op.data = p.get_mmu()->load_int8(op.addr);
        return 1;
    case csChgAccLDint16:
        op.data = p.get_mmu()->load_int16(op.addr);
        return 1;
    case csChgAccLDint32:
        op.data = p.get_mmu()->load_int32(op.addr);
        return 1;
    case csChgAccLDint64:
        op.data = p.get_mmu()->load_int64(op.addr);
        return 1;
    case csChgAccLDuint8:
        op.data = p.get_mmu()->load_uint8(op.addr);
        return 1;
    case csChgAccLDuint16:
        op.data = p.get_mmu()->load_uint16(op.addr);
        return 1;
    case csChgAccLDuint32:
        op.data = p.get_mmu()->load_uint32(op.addr);
        return 1;
    case csChgAccLDuint64:
        op.data = p.get_mmu()->load_uint64(op.addr);
        return 1;
    case csChgAccSTuint8:
        p.get_mmu()->store_uint8(op.addr, op.data);
        return 1;
    case csChgAccSTuint16:
        p.get_mmu()->store_uint16(op.addr, op.data);
        return 1;
    case csChgAccSTuint32:
        p.get_mmu()->store_uint32(op.addr, op.data);
        return 1;
    case csChgAccSTuint64:
        p.get_mmu()->store_uint64(op.addr, op.data);
        return 1;
        break;
  };
  fprintf(stderr, "cosim: Error executing cosim::csExecCPUop: op %d\n", op.access);
  return 0;
}

void cosim::csFesvrStart(csHandle& handle){
  sim_t* s = reinterpret_cast<sim_t*>(handle);
  s->start();
}

void cosim::csFesvrStep(csHandle& handle){
  sim_t* s = reinterpret_cast<sim_t*>(handle);
  s->step_htif();
}

void cosim::csFesvrStop(csHandle& handle){
  sim_t* s = reinterpret_cast<sim_t*>(handle);
  s->stop();
}

void cosim::csGDBStep(csHandle& handle){
  sim_t* s = reinterpret_cast<sim_t*>(handle);
  if (s == NULL) return;
  if (gdb_port == 0) return;
  s->gdbserver->handle();
}

