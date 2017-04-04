
all:
	vcs -full64 -sverilog -debug_all -LDFLAGS "-L ${COSIM_HOME}/lib" -lriscv -lfesvr ../vsrc/cosim_pkg.sv ../vsrc/cosim_test.sv

test:
	g++ -fPIC -MMD -MP -I/home/cp/p/bin//include  -Wall -Wno-unused -g -Og -std=c++11 -DPREFIX=\"/home/cp/p/bin\" -Werror -I../build/ -I. -I.. -I../riscv -I../dummy_rocc -I../softfloat -I../spike_main -c test.cc -o test.o
	g++ -L. -L/home/cp/p/bin//lib  -Wl,-rpath,/home/cp/p/bin/lib -Wl,-rpath,/home/cp/p/bin//lib -o test test.o  -lspike_main  -lriscv  -lsoftfloat -lpthread -lfesvr -ldl
