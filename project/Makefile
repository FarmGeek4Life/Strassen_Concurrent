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
CXXFLAGS = -pthread ${STD} -pedantic -g
# For C++ linking:
# $(CC) $(LDFLAGS) n.o $(LOADLIBES) $(LDLIBS)
# Linker program: (default cc, aka gcc)
CC = g++
# Extra linker flags: (use -L)
# LDFLAGS = -L
# Library flags for linker: (LOADLIBES same, but deprecated) (use -l)
# THIS LINE WILL CAUSE DUPLICATIONS, BUT THE DATA IS NEEDED IF *.o IS EVER USED!!!!
LDLIBS = -pthread ${STD} -pedantic -g

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

client_manager: client_manager.cpp connection.h errorColors.h matrix.h

client: client_remove client_manager

client_remove:
	-rm client_manager

server_leaf: server_leaf.cpp connection.h errorColors.h matrix.h

server_cleanup:
	./cleanup_servers_failsafe.bash

server: server_cleanup server_remove server_leaf

server_remove:
	-rm server_leaf

# Add all recipes where we want to ignore errors....
.IGNORE: strassen_int_futures

.PHONY: clean all mult_compare compare_big clean_temp client_test server_test dist_test local_test server_cleanup server server_remove client client_remove

mult_compare: strassen_int strassen_noThreads_int standard strassen_int_opt standard_noThread strassen_int_opt2
	time ./strassen_noThreads_int matrices/512_1.txt matrices/512_2.txt 512 | diff -q - matrices/512_1-2.txt
	time ./strassen_int           matrices/512_1.txt matrices/512_2.txt 512 | diff -q - matrices/512_1-2.txt
	time ./standard               matrices/512_1.txt matrices/512_2.txt 512 | diff -q - matrices/512_1-2.txt
	time ./strassen_int_opt       matrices/512_1.txt matrices/512_2.txt 512 | diff -q - matrices/512_1-2.txt
	time ./strassen_int_opt2      matrices/512_1.txt matrices/512_2.txt 512 | diff -q - matrices/512_1-2.txt
	time ./standard_noThread      matrices/512_1.txt matrices/512_2.txt 512 | diff -q - matrices/512_1-2.txt

TEST_SIZE := 512
client_test: client_manager
# Available sizes: 16 32 64 128 512
	@echo "Use the BG_COMPUTERS and BG_PORT environment variables"
	@echo "For example, |export BG_COMPUTERS='x.x.x.x:y.y.y.y:z.z.z.z'|"
	@echo "For example, |export BG_PORT='xxxx'|"
	export BG_COMPUTERS="$(BG_COMPUTERS)"; \
	export BG_PORT="$(BG_PORT)"; \
	time ./client_manager /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -qs - /tmp/brysoncg/$(SIZE)_1-2.txt
#	time ./client_manager matrices/$(TEST_SIZE)_1.txt matrices/$(TEST_SIZE)_2.txt $(TEST_SIZE) | diff -qs - matrices/$(TEST_SIZE)_1-2.txt
#	time ./client_manager matrices/16_1.txt  matrices/16_2.txt  16  | diff -qs - matrices/16_1-2.txt
#	time ./client_manager matrices/32_1.txt  matrices/32_2.txt  32  | diff -qs - matrices/32_1-2.txt
#	time ./client_manager matrices/64_1.txt  matrices/64_2.txt  64  | diff -qs - matrices/64_1-2.txt
#	time ./client_manager matrices/128_1.txt matrices/128_2.txt 128 | diff -qs - matrices/128_1-2.txt

server_test: server_leaf
	@echo "Use the BG_PORT environment variable"
	@echo "For example, |export BG_PORT='xxxx'|"
	./server_leaf $(BG_PORT)

server_test_view: server_leaf
	@echo "Use the BG_PORT environment variable"
	@echo "For example, |export BG_PORT='xxxx'|"
	./cleanup_servers_failsafe.bash
	./run_parallel.bash $(BG_PORT) 206 207 208 209 217 218 219
	./kill_local.bash
	./server_leaf $(BG_PORT)

client_debug: client_manager
	@echo "Use the BG_COMPUTERS and BG_PORT environment variables"
	@echo "For example, |export BG_COMPUTERS='x.x.x.x:y.y.y.y:z.z.z.z'|"
	@echo "For example, |export BG_PORT='xxxx'|"
	export BG_COMPUTERS="$(BG_COMPUTERS)"; \
	export BG_PORT="$(BG_PORT)"; \
	gdb client_manager -ex "run matrices/64_1.txt matrices/64_2.txt 64 | diff -qs - matrices/64_1-2.txt"

server_debug: server_leaf
	@echo "Use the BG_PORT environment variable"
	@echo "For example, |export BG_PORT='xxxx'|"
	gdb server_leaf -ex "run $(BG_PORT)"

dist_test: client_manager server_leaf
# KILL ANY OPEN SERVERS FIRST....
	./cleanup_servers_failsafe.bash
# Make careful note of the line continuation......
	. ./run_parallel.bash 10021 206 207 208 209 217 218 219 ; \
	time ./client_manager matrices/16_1.txt matrices/16_2.txt 16 | diff -qs - matrices/16_1-2.txt
#	time ./client_manager matrices/32_1.txt matrices/32_2.txt 32 | diff -qs - matrices/32_1-2.txt
#	time ./client_manager matrices/64_1.txt matrices/64_2.txt 64 | diff -qs - matrices/64_1-2.txt
#	time ./client_manager matrices/64_1.txt matrices/64_2.txt 64; \
#	echo "exited with status '$$?'"

dist_debug: client_manager server_leaf
# KILL ANY OPEN SERVERS FIRST....
	./cleanup_servers_failsafe.bash
# Make careful note of the line continuation......
	. ./run_parallel.bash 10021 206 207 208 209 217 218 219 ; \
	gdb client_manager --ex "run matrices/64_1.txt matrices/64_2.txt 64"
#	time ./client_manager matrices/64_1.txt matrices/64_2.txt 64 | diff -q - matrices/64_1-2.txt

local_test: client_manager server_leaf
# KILL ANY OPEN SERVERS FIRST....
	./kill_local.bash
	export BG_COMPUTERS="$(shell hostname -i)"; \
	export BG_PORT="10021"; \
	./server_leaf 10021 & \
	time ./client_manager matrices/128_1.txt matrices/128_2.txt 128 | diff -qs - matrices/128_1-2.txt
#	time ./client_manager matrices/64_1.txt  matrices/64_2.txt  64 | diff -qs -  matrices/64_1-2.txt
#	PID=$?; \
#	time ./client_manager matrices/64_1.txt  matrices/64_2.txt  64 | diff -qs -  matrices/64_1-2.txt; \
#	kill -9 $PID

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
	-@( [ ! -f /tmp/brysoncg/$(SIZE)_1-2.txt ] && time (./strassen_int_opt /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) \
	> /tmp/brysoncg/$(SIZE)_1-2.txt) ) || true
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

compare_big_fast_s: create_big_fast strassen_int_opt strassen_int_opt2
#ifndef SIZE
#  $(warning Please add 'SIZE=[size]' to the command line)
#endif
#	-time ./strassen_int_opt /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -q - /tmp/brysoncg/$(SIZE)_1-2.txt
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
	-time ./strassen_int_futures matrices/512_1.txt matrices/512_2.txt 512 | diff -qs - matrices/512_1-2.txt

generate_solutions: standard_noThread
	time ./standard_noThread matrices/16_1.txt  matrices/16_2.txt  16  > matrices/16_1-2.txt
	time ./standard_noThread matrices/16_2.txt  matrices/16_1.txt  16  > matrices/16_2-1.txt
	time ./standard_noThread matrices/16_1.txt  matrices/16_1.txt  16  > matrices/16_1-1.txt
	time ./standard_noThread matrices/16_2.txt  matrices/16_2.txt  16  > matrices/16_2-2.txt
	time ./standard_noThread matrices/32_1.txt  matrices/32_2.txt  32  > matrices/32_1-2.txt
	time ./standard_noThread matrices/32_2.txt  matrices/32_1.txt  32  > matrices/32_2-1.txt
	time ./standard_noThread matrices/32_1.txt  matrices/32_1.txt  32  > matrices/32_1-1.txt
	time ./standard_noThread matrices/32_2.txt  matrices/32_2.txt  32  > matrices/32_2-2.txt
	time ./standard_noThread matrices/64_1.txt  matrices/64_2.txt  64  > matrices/64_1-2.txt
	time ./standard_noThread matrices/64_2.txt  matrices/64_1.txt  64  > matrices/64_2-1.txt
	time ./standard_noThread matrices/64_1.txt  matrices/64_1.txt  64  > matrices/64_1-1.txt
	time ./standard_noThread matrices/64_2.txt  matrices/64_2.txt  64  > matrices/64_2-2.txt
	time ./standard_noThread matrices/128_1.txt matrices/128_2.txt 128 > matrices/128_1-2.txt
	time ./standard_noThread matrices/128_2.txt matrices/128_1.txt 128 > matrices/128_2-1.txt
	time ./standard_noThread matrices/128_1.txt matrices/128_1.txt 128 > matrices/128_1-1.txt
	time ./standard_noThread matrices/128_2.txt matrices/128_2.txt 128 > matrices/128_2-2.txt
	time ./standard_noThread matrices/512_1.txt matrices/512_2.txt 512 > matrices/512_1-2.txt
	time ./standard_noThread matrices/512_2.txt matrices/512_1.txt 512 > matrices/512_2-1.txt
	time ./standard_noThread matrices/512_1.txt matrices/512_1.txt 512 > matrices/512_1-1.txt
	time ./standard_noThread matrices/512_2.txt matrices/512_2.txt 512 > matrices/512_2-2.txt

clean:
	-rm *.o
# Pre-compiled headers.
	-rm *.gch
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
