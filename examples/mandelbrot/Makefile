
CXX=g++ -m64
CXXFLAGS=-Iobjs/ -O3 -Wall
ISPC=ispc
ISPCFLAGS=-O2 --target=sse4x2 --arch=x86-64

default: mandelbrot

.PHONY: dirs clean

dirs:
	/bin/mkdir -p objs/

clean:
	/bin/rm -rf objs *~ mandelbrot

mandelbrot: dirs objs/mandelbrot.o objs/mandelbrot_serial.o objs/mandelbrot_ispc.o
	$(CXX) $(CXXFLAGS) -o $@ objs/mandelbrot.o objs/mandelbrot_ispc.o objs/mandelbrot_serial.o -lm

objs/%.o: %.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

objs/mandelbrot.o: objs/mandelbrot_ispc.h 

objs/%_ispc.h objs/%_ispc.o: %.ispc
	$(ISPC) $(ISPCFLAGS) $< -o objs/$*_ispc.o -h objs/$*_ispc.h
