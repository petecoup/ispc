
ARCH = $(shell uname)

TASK_CXX=../tasks_pthreads.cpp
TASK_LIB=-lpthread

ifeq ($(ARCH), Darwin)
  TASK_CXX=../tasks_gcd.cpp
  TASK_LIB=
endif

TASK_OBJ=$(addprefix objs/, $(subst ../,, $(TASK_CXX:.cpp=.o)))

CXX=g++
CXXFLAGS=-Iobjs/ -O3 -Wall -m64
ISPC=ispc
ISPCFLAGS=-O2 --target=sse4x2 --arch=x86-64

default: volume

.PHONY: dirs clean

dirs:
	/bin/mkdir -p objs/

clean:
	/bin/rm -rf objs *~ volume

volume: dirs objs/volume.o objs/volume_serial.o objs/volume_ispc.o $(TASK_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ objs/volume.o objs/volume_ispc.o objs/volume_serial.o $(TASK_OBJ) -lm $(TASK_LIB)

objs/%.o: %.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

objs/%.o: ../%.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

objs/volume.o: objs/volume_ispc.h 

objs/%_ispc.h objs/%_ispc.o: %.ispc
	$(ISPC) $(ISPCFLAGS) $< -o objs/$*_ispc.o -h objs/$*_ispc.h
