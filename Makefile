# Force bash as the shell so that 'time' works
SHELL = /bin/bash

GCC_VERSION := $(shell g++ --version | grep -E -o -e "4\.[0-9]\.[0-9]" | grep -E -o -e "^4\.[0-9]" | head -n1)

ifeq ($(GCC_VERSION),4.8)
  STD := -std=c++11
else ifeq ($(GCC_VERSION),4.7)
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
CXXFLAGS = -pthread ${STD} -pedantic
# For C++ linking:
# $(CC) $(LDFLAGS) n.o $(LOADLIBES) $(LDLIBS)
# Linker program: (default cc, aka gcc)
CC = g++
# Extra linker flags: (use -L)
# LDFLAGS = -L
# Library flags for linker: (LOADLIBES same, but deprecated) (use -l)
# THIS LINE WILL CAUSE DUPLICATIONS, BUT THE DATA IS NEEDED IF *.o IS EVER USED!!!!
LDLIBS = -pthread ${STD} -pedantic

ifndef SIZE
  $(warning Please add 'SIZE=[size]' to the command line)
  SIZE=64
endif

ifndef BG_COMPUTERS
  $(warning Please run 'export BG_COMPUTERS="x.x.x.x:y.y.y.y:z.z.z.z"' on the command line)
  BG_COMPUTERS := $(shell hostname -i)
  $(info 'BG_COMPUTERS' set to '$(BG_COMPUTERS)')
endif
ifndef BG_PORT
  $(warning Please run 'export BG_PORT="xxxx"' on the command line)
  BG_PORT := 10021
  $(info 'BG_PORT' set to '$(BG_PORT)')
endif

strassen_int: strassen_int.cpp

strassen_int_futures: strassen_int_futures.cpp
#	g++ strassen_int_futures.cpp -o strassen_int_futures ${STD} -pthread

client_manager: client_manager.cpp connection.h

server_leaf: server_leaf.cpp connection.h

# Add all recipes where we want to ignore errors....
.IGNORE: strassen_int_futures

.PHONY: clean all mult_compare compare_big clean_temp client_test server_test dist_test local_test

mult_compare: strassen_int strassen_noThreads_int standard strassen_int_opt standard_noThread strassen_int_opt2
	time ./strassen_noThreads_int 512_1.txt 512_2.txt 512 | diff -q - 512_1-2.txt
	time ./strassen_int 512_1.txt 512_2.txt 512 | diff -q - 512_1-2.txt
	time ./standard 512_1.txt 512_2.txt 512 | diff -q - 512_1-2.txt
	time ./strassen_int_opt 512_1.txt 512_2.txt 512 | diff -q - 512_1-2.txt
	time ./strassen_int_opt2 512_1.txt 512_2.txt 512 | diff -q - 512_1-2.txt
	time ./standard_noThread 512_1.txt 512_2.txt 512 | diff -q - 512_1-2.txt

client_test: client_manager
	@echo "Use the BG_COMPUTERS and BG_PORT environment variables"
	@echo "For example, |export BG_COMPUTERS='x.x.x.x:y.y.y.y:z.z.z.z'|"
	@echo "For example, |export BG_PORT='xxxx'|"
	time ./client_manager 64_1.txt 64_2.txt 64 | diff -q - 64_1-2.txt
	#time ./client_manager 32_1.txt 32_2.txt 32 | diff -q - 32_1-2.txt
	#time ./client_manager 32_1.txt 32_2.txt 32 | diff - 32_1-2.txt

server_test: server_leaf
	@echo "Use the BG_PORT environment variable"
	@echo "For example, |export BG_PORT='xxxx'|"
	./server_leaf $(BG_PORT)

dist_test: client_manager server_leaf
	. ./run_parallel.bash 10021 206 207 208 209 217 218 219 ;\
	#time ./client_manager 64_1.txt 64_2.txt 64 | diff -q - 64_1-2.txt
	time ./client_manager 64_1.txt 64_2.txt 64

local_test: client_manager server_leaf
	export BG_COMPUTERS="$(shell hostname -i)"; \
	export BG_PORT="10021"; \
	./server_leaf 10021 & \
	time ./client_manager 64_1.txt 64_2.txt 64 | diff -q - 64_1-2.txt

clean_temp:
	-rm -rf /tmp/brysoncg

create_big: generator standard
#ifndef SIZE
#  $(warning Please add 'SIZE=[size]' to the command line)
#endif
	@echo "creating matrices, if required"
	-@( [ ! -d /tmp/brysoncg ] && mkdir /tmp/brysoncg ) || true
	-@( [ ! -f /tmp/brysoncg/$(SIZE)_1.txt ] && ./generator $(SIZE) > /tmp/brysoncg/$(SIZE)_1.txt ) || true
	-@( [ ! -f /tmp/brysoncg/$(SIZE)_2.txt ] && ./generator $(SIZE) > /tmp/brysoncg/$(SIZE)_2.txt ) || true
	-@( [ ! -f /tmp/brysoncg/$(SIZE)_1-2.txt ] && ./standard /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) \
	> /tmp/brysoncg/$(SIZE)_1-2.txt ) || true
	# Final check: make sure they all exist, or fail and exit.
	@( ( [ -d /tmp/brysoncg ] && [ -f /tmp/brysoncg/$(SIZE)_1.txt ] && \
	[ -f /tmp/brysoncg/$(SIZE)_2.txt ] && [ -f /tmp/brysoncg/$(SIZE)_1-2.txt ] ) \
	 && echo "Matrices created/exist, continuing" ) || \
	( echo "Matrices do NOT exist, exiting now!!" && false )

create_big_fast: generator strassen_int_opt
#ifndef SIZE
#  $(warning Please add 'SIZE=[size]' to the command line)
#endif
	@echo "creating matrices, if required"
	-@( [ ! -d /tmp/brysoncg ] && mkdir /tmp/brysoncg ) || true
	-@( [ ! -f /tmp/brysoncg/$(SIZE)_1.txt ] && ./generator $(SIZE) > /tmp/brysoncg/$(SIZE)_1.txt ) || true
	-@( [ ! -f /tmp/brysoncg/$(SIZE)_2.txt ] && ./generator $(SIZE) > /tmp/brysoncg/$(SIZE)_2.txt ) || true
	-@( [ ! -f /tmp/brysoncg/$(SIZE)_1-2.txt ] && ./strassen_int_opt /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) \
	> /tmp/brysoncg/$(SIZE)_1-2.txt ) || true
	# Final check: make sure they all exist, or fail and exit.
	@( ( [ -d /tmp/brysoncg ] && [ -f /tmp/brysoncg/$(SIZE)_1.txt ] && \
	[ -f /tmp/brysoncg/$(SIZE)_2.txt ] && [ -f /tmp/brysoncg/$(SIZE)_1-2.txt ] ) \
	 && echo "Matrices created/exist, continuing" ) || \
	( echo "Matrices do NOT exist, exiting now!!" && false )

compare_big: create_big standard standard_noThread strassen_int_opt strassen_int_opt2
#ifndef SIZE
#  $(warning Please add 'SIZE=[size]' to the command line)
#endif
	-time ./standard /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -q - /tmp/brysoncg/$(SIZE)_1-2.txt
	-time ./standard_noThread /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -q - /tmp/brysoncg/$(SIZE)_1-2.txt
	-time ./strassen_int_opt /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -q - /tmp/brysoncg/$(SIZE)_1-2.txt
	-time ./strassen_int_opt2 /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -q - /tmp/brysoncg/$(SIZE)_1-2.txt

compare_big_fast: create_big_fast strassen_int_opt strassen_int_opt2
#ifndef SIZE
#  $(warning Please add 'SIZE=[size]' to the command line)
#endif
	-time ./strassen_int_opt /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -q - /tmp/brysoncg/$(SIZE)_1-2.txt
	-time ./strassen_int_opt2 /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -q - /tmp/brysoncg/$(SIZE)_1-2.txt

compare_big_limit: create_big standard standard_noThread strassen_int_opt strassen_int_opt2
#ifndef SIZE
#  $(warning Please add 'SIZE=[size]' to the command line)
#endif
	-time ./standard /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) 200 | diff -q - /tmp/brysoncg/$(SIZE)_1-2.txt
	-time ./standard_noThread /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -q - /tmp/brysoncg/$(SIZE)_1-2.txt
	-time ./strassen_int_opt /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -q - /tmp/brysoncg/$(SIZE)_1-2.txt
	-time ./strassen_int_opt2 /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -q - /tmp/brysoncg/$(SIZE)_1-2.txt

compare_big_all: create_big strassen_int strassen_noThreads_int standard standard_noThread strassen_int_opt strassen_int_opt2
#ifndef SIZE
#  $(warning Please add 'SIZE=[size]' to the command line)
#endif
	-time ./standard /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -q - /tmp/brysoncg/$(SIZE)_1-2.txt
	-time ./standard_noThread /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -q - /tmp/brysoncg/$(SIZE)_1-2.txt
	-time ./strassen_int_opt /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -q - /tmp/brysoncg/$(SIZE)_1-2.txt
	-time ./strassen_int_opt2 /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -q - /tmp/brysoncg/$(SIZE)_1-2.txt
	-time ./strassen_int /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -q - /tmp/brysoncg/$(SIZE)_1-2.txt
	-time ./strassen_noThreads_int /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -q - /tmp/brysoncg/$(SIZE)_1-2.txt

test_futures: strassen_int_futures
	-time ./strassen_int_futures 512_1.txt 512_2.txt 512 | diff -q - 512_1-2.txt

generate_solutions: standard_noThread
	time ./standard_noThread 16_1.txt 16_2.txt 16 > 16_1-2.txt
	time ./standard_noThread 16_2.txt 16_1.txt 16 > 16_2-1.txt
	time ./standard_noThread 16_1.txt 16_1.txt 16 > 16_1-1.txt
	time ./standard_noThread 16_2.txt 16_2.txt 16 > 16_2-2.txt
	time ./standard_noThread 32_1.txt 32_2.txt 32 > 32_1-2.txt
	time ./standard_noThread 32_2.txt 32_1.txt 32 > 32_2-1.txt
	time ./standard_noThread 32_1.txt 32_1.txt 32 > 32_1-1.txt
	time ./standard_noThread 32_2.txt 32_2.txt 32 > 32_2-2.txt
	time ./standard_noThread 64_1.txt 64_2.txt 64 > 64_1-2.txt
	time ./standard_noThread 64_2.txt 64_1.txt 64 > 64_2-1.txt
	time ./standard_noThread 64_1.txt 64_1.txt 64 > 64_1-1.txt
	time ./standard_noThread 64_2.txt 64_2.txt 64 > 64_2-2.txt
	time ./standard_noThread 128_1.txt 128_2.txt 128 > 128_1-2.txt
	time ./standard_noThread 128_2.txt 128_1.txt 128 > 128_2-1.txt
	time ./standard_noThread 128_1.txt 128_1.txt 128 > 128_1-1.txt
	time ./standard_noThread 128_2.txt 128_2.txt 128 > 128_2-2.txt
	time ./standard_noThread 512_1.txt 512_2.txt 512 > 512_1-2.txt
	time ./standard_noThread 512_2.txt 512_1.txt 512 > 512_2-1.txt
	time ./standard_noThread 512_1.txt 512_1.txt 512 > 512_1-1.txt
	time ./standard_noThread 512_2.txt 512_2.txt 512 > 512_2-2.txt

clean:
	-rm *.o
	-rm generator
	-rm strassen_noThreads
	-rm strassen_noThreads_int
	-rm strassen_threads
	-rm strassen_int
	-rm strassen_int_opt
	-rm strassen_int_opt2
	-rm standard
	-rm standard_noThread
	-rm client_manager
	-rm server_leaf

# Generic recipes: All recipes that do not use these need to come above this line.
# These are called 'Pattern Rules'
#%: %.o
#	g++ $*.o -o $* ${STD} -pthread
#
#%.o: %.cpp %.h
#	g++ -c $*.cpp ${STD} -pthread
