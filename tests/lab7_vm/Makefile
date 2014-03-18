# Force bash as the shell so that 'time' works
SHELL = /bin/bash

GCC_VERSION := $(shell g++ --version | grep -E -o -e "4\.[0-9]\.[0-9]" | grep -E -o -e "^4\.[0-9]" | head -n1)

ifeq ($(GCC_VERSION),4.8)
  STD := -std=c++11
else ifeq ($(GCC_VERSION),4.8)
  STD := -std=c++11
else
  STD := -std=c++0x
endif

# Using implicit recipes...
# For C++ compilation:
# $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c [files]
# C++ compiler program, default 'g++'
# CXX = g++
# Flags for the C++ preprocessor
#CPPFLAGS = 
# Flags for the C++ compiler
CXXFLAGS = -pthread ${STD}
# For C++ linking:
# $(CC) $(LDFLAGS) n.o $(LOADLIBES) $(LDLIBS)
# Linker program: (default cc, aka gcc)
CC = g++
# Extra linker flags: (use -L)
# LDFLAGS = -L
# Library flags for linker: (LOADLIBES same, but deprecated) (use -l)
# THIS LINE WILL CAUSE DUPLICATIONS, BUT THE DATA IS NEEDED IF *.o IS EVER USED!!!!
LDLIBS = -pthread ${STD}

all: threads-pthreads threads-omp threads-c++11 threads-c++11_future threads-c++11_future_no-async threads-opencl

# Ignore failed recipes....
.IGNORE: threads-c++11_future threads-c++11_future_no-async threads-opencl

threads-pthreads: threads-pthreads.cpp

threads-c++11: threads-c++11.cpp

threads-c++11_future: threads-c++11_future.cpp

threads-c++11_future_no-async: threads-c++11_future_no-async.cpp

threads-omp: threads-omp.cpp
	g++ -o threads-omp threads-omp.cpp -fopenmp

threads-opencl: threads-opencl.cpp
	-g++ -o threads-opencl threads-opencl.cpp ${STD} -lOpenCL

test: all
	./threads-pthreads | diff - threads_solution.txt
	./threads-omp | diff - threads_solution.txt
	./threads-c++11 | diff - threads_solution.txt
	-./threads-c++11_future_no-async | diff - threads_solution.txt
	-./threads-c++11_future | diff - threads_solution.txt
	-./threads-opencl | diff - threads_solution.txt

compare: all
	time -p ./threads-pthreads
	time -p ./threads-omp
	time -p ./threads-c++11
	-time -p ./threads-c++11_future_no-async
	-time -p ./threads-c++11_future
	-time -p ./threads-opencl

clean:
	-rm trap-omp
	-rm threads-pthreads
	-rm threads-omp
	-rm threads-c++11
	-rm threads-c++11_future_no-async
	-rm threads-c++11_future
	-rm threads-opencl
	-rm test_cl

test_cl: test_cl.cpp
	g++ test_cl.cpp -o test_cl -lOpenCL ${STD}

.PHONY : clean run run2 runoff test test2 time compare all

trap-omp: trap-omp.cpp
	g++ -o trap-omp trap-omp.cpp -fopenmp

trap-omp_off: trap-omp.cpp
	g++ -o trap-omp_off trap-omp.cpp

time: trap-omp
	time -p trap-omp
	time -p trap-omp 2
	time -p trap-omp 3
	time -p trap-omp 4
	time -p trap-omp 5
	time -p trap-omp 6
	time -p trap-omp 7
	time -p trap-omp 8
	time -p trap-omp 16
	time -p trap-omp 32
	time -p trap-omp 64
	time -p trap-omp 128

test2: run run2

run: trap-omp
	./trap-omp

runoff: trap-omp_off
	./trap-omp_off

run2: trap-omp
	./trap-omp 2
