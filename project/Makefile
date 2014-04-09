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

# -g is for debugging... - remove when using optimizations (counterproductive)
# -pedantic provides improved information about compile errors with c++11/0x
# -O3 - highest level of optimization...
COMP_FLAGS = -pthread ${STD} -pedantic -O3
# Using implicit recipes...
# For C++ compilation:
# $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c [files]
# C++ compiler program, default 'g++'
# CXX = g++
# Flags for the C++ preprocessor
#CPPFLAGS = 
# Flags for the C++ compiler
#CXXFLAGS = -pthread ${STD} -pedantic -g
CXXFLAGS = ${COMP_FLAGS}
# For C++ linking:
# $(CC) $(LDFLAGS) n.o $(LOADLIBES) $(LDLIBS)
# Linker program: (default cc, aka gcc)
CC = g++
# Extra linker flags: (use -L)
# LDFLAGS = -L
# Library flags for linker: (LOADLIBES same, but deprecated) (use -l)
# THIS LINE WILL CAUSE DUPLICATIONS, BUT THE DATA IS NEEDED IF *.o IS EVER USED!!!!
#LDLIBS = -pthread ${STD} -pedantic -g
LDLIBS = ${COMP_FLAGS}

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
ifndef BG_COUNT
  $(warning Please run 'export BG_COUNT="x"' on the command line)
  BG_COUNT := 7
  $(info 'BG_COUNT' set to '$(BG_COUNT)')
endif

client_manager: client_manager.cpp connection.h errorColors.h matrix.h

client: client_remove client_manager

client_remove:
	-rm client_manager

server_leaf: server_leaf.cpp connection.h errorColors.h matrix.h

server_cleanup:
	./cleanup_servers_failsafe.bash

server: server_cleanup server_remove server_leaf server_start

server_remove:
	-rm server_leaf

server_start: server_leaf
	[[ "$(BG_COMPUTERS)" != "$$(hostname -i)" ]] && ./use_count.bash $(BG_PORT) $(BG_COUNT)

# Add all recipes where we want to ignore errors....
.IGNORE: strassen_int_futures

.PHONY: clean all compare_huge clean_temp client_test server_test server_cleanup server server_remove client client_remove

client_test: client_manager
	export BG_COMPUTERS="$(BG_COMPUTERS)"; \
	export BG_PORT="$(BG_PORT)"; \
	time ./client_manager /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -qs - /tmp/brysoncg/$(SIZE)_1-2.txt

client_test_set: client_manager
	time ./client_manager /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -qs - /tmp/brysoncg/$(SIZE)_1-2.txt
	time ./client_manager /tmp/brysoncg/$(SIZE)_2.txt /tmp/brysoncg/$(SIZE)_3.txt $(SIZE) | diff -qs - /tmp/brysoncg/$(SIZE)_2-3.txt
	time ./client_manager /tmp/brysoncg/$(SIZE)_3.txt /tmp/brysoncg/$(SIZE)_4.txt $(SIZE) | diff -qs - /tmp/brysoncg/$(SIZE)_3-4.txt
	time ./client_manager /tmp/brysoncg/$(SIZE)_4.txt /tmp/brysoncg/$(SIZE)_5.txt $(SIZE) | diff -qs - /tmp/brysoncg/$(SIZE)_4-5.txt
	time ./client_manager /tmp/brysoncg/$(SIZE)_5.txt /tmp/brysoncg/$(SIZE)_6.txt $(SIZE) | diff -qs - /tmp/brysoncg/$(SIZE)_5-6.txt
	time ./client_manager /tmp/brysoncg/$(SIZE)_6.txt /tmp/brysoncg/$(SIZE)_1.txt $(SIZE) | diff -qs - /tmp/brysoncg/$(SIZE)_6-1.txt

server_test: server_leaf
	@echo "Use the BG_PORT environment variable"
	@echo "For example, |export BG_PORT='xxxx'|"
	./server_leaf $(BG_PORT)

clean_temp:
	-rm -rf /tmp/brysoncg

generate_big: generator
	@echo "creating matrices, if required"
	-@( [ ! -d /tmp/brysoncg ] && mkdir /tmp/brysoncg ) || true
	-@( ( [ ! -f /tmp/brysoncg/$(SIZE)_1.txt ] || [ $$(wc -w /tmp/brysoncg/$(SIZE)_1.txt | grep -E -o -e "^[0-9]{1,15}") -ne $$(( $(SIZE) * $(SIZE) )) ] ) && \
	./generator $(SIZE) > /tmp/brysoncg/$(SIZE)_1.txt ) || true
	-@( ( [ ! -f /tmp/brysoncg/$(SIZE)_2.txt ] || [ $$(wc -w /tmp/brysoncg/$(SIZE)_2.txt | grep -E -o -e "^[0-9]{1,15}") -ne $$(( $(SIZE) * $(SIZE) )) ] ) && \
	./generator $(SIZE) > /tmp/brysoncg/$(SIZE)_2.txt ) || true

create_huge: generate_big strassen_int_opt_fasterer
	@echo "creating matrix solution, if required"
	-@( ( [ ! -f /tmp/brysoncg/$(SIZE)_1-2.txt ] || [ $$(wc -w /tmp/brysoncg/$(SIZE)_1-2.txt | grep -E -o -e "^[0-9]{1,15}") -ne $$(( $(SIZE) * $(SIZE) )) ] ) && \
	time (./strassen_int_opt_fasterer /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) > /tmp/brysoncg/$(SIZE)_1-2.txt) ) || true
# Final check: make sure they all exist, or fail and exit.
	@( ( [ -d /tmp/brysoncg ] && [ -f /tmp/brysoncg/$(SIZE)_1.txt ] && \
	[ -f /tmp/brysoncg/$(SIZE)_2.txt ] && [ -f /tmp/brysoncg/$(SIZE)_1-2.txt ] ) \
	 && echo "Matrices created/exist, continuing" ) || \
	( echo "Matrices do NOT exist, exiting now!!" && false )

create_big_lomem: generate_big strassen_int_opt_large
	@echo "creating matrix solution, if required"
	-@( ( [ ! -f /tmp/brysoncg/$(SIZE)_1-2.txt ] || [ $$(wc -w /tmp/brysoncg/$(SIZE)_1-2.txt | grep -E -o -e "^[0-9]{1,15}") -ne $$(( $(SIZE) * $(SIZE) )) ] ) && \
	time (./strassen_int_opt_large /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) > /tmp/brysoncg/$(SIZE)_1-2.txt) ) || true
# Final check: make sure they all exist, or fail and exit.
	@( ( [ -d /tmp/brysoncg ] && [ -f /tmp/brysoncg/$(SIZE)_1.txt ] && \
	[ -f /tmp/brysoncg/$(SIZE)_2.txt ] && [ -f /tmp/brysoncg/$(SIZE)_1-2.txt ] ) \
	 && echo "Matrices created/exist, continuing" ) || \
	( echo "Matrices do NOT exist, exiting now!!" && false )

compare_huge: create_huge strassen_int_opt_faster strassen_int_opt_fasterer strassen_int_opt_large
	-time ./strassen_int_opt_faster /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -qs - /tmp/brysoncg/$(SIZE)_1-2.txt
	-time ./strassen_int_opt_large /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -qs - /tmp/brysoncg/$(SIZE)_1-2.txt
	-time ./strassen_int_opt_fasterer /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -qs - /tmp/brysoncg/$(SIZE)_1-2.txt

local_test_set: strassen_int_opt_fasterer
	time ./strassen_int_opt_fasterer /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) | diff -qs - /tmp/brysoncg/$(SIZE)_1-2.txt
	time ./strassen_int_opt_fasterer /tmp/brysoncg/$(SIZE)_2.txt /tmp/brysoncg/$(SIZE)_3.txt $(SIZE) | diff -qs - /tmp/brysoncg/$(SIZE)_2-3.txt
	time ./strassen_int_opt_fasterer /tmp/brysoncg/$(SIZE)_3.txt /tmp/brysoncg/$(SIZE)_4.txt $(SIZE) | diff -qs - /tmp/brysoncg/$(SIZE)_3-4.txt
	time ./strassen_int_opt_fasterer /tmp/brysoncg/$(SIZE)_4.txt /tmp/brysoncg/$(SIZE)_5.txt $(SIZE) | diff -qs - /tmp/brysoncg/$(SIZE)_4-5.txt
	time ./strassen_int_opt_fasterer /tmp/brysoncg/$(SIZE)_5.txt /tmp/brysoncg/$(SIZE)_6.txt $(SIZE) | diff -qs - /tmp/brysoncg/$(SIZE)_5-6.txt
	time ./strassen_int_opt_fasterer /tmp/brysoncg/$(SIZE)_6.txt /tmp/brysoncg/$(SIZE)_1.txt $(SIZE) | diff -qs - /tmp/brysoncg/$(SIZE)_6-1.txt

create_tests: generator strassen_int_opt_fasterer
	-@( [ ! -d /tmp/brysoncg ] && mkdir /tmp/brysoncg ) || true
	-@( ( [ ! -f /tmp/brysoncg/$(SIZE)_1.txt ] || [ $$(wc -w /tmp/brysoncg/$(SIZE)_1.txt | grep -E -o -e "^[0-9]{1,15}") -ne $$(( $(SIZE) * $(SIZE) )) ] ) && \
	./generator $(SIZE) > /tmp/brysoncg/$(SIZE)_1.txt ) || true
	-@( ( [ ! -f /tmp/brysoncg/$(SIZE)_2.txt ] || [ $$(wc -w /tmp/brysoncg/$(SIZE)_2.txt | grep -E -o -e "^[0-9]{1,15}") -ne $$(( $(SIZE) * $(SIZE) )) ] ) && \
	./generator $(SIZE) > /tmp/brysoncg/$(SIZE)_2.txt ) || true
	-@( ( [ ! -f /tmp/brysoncg/$(SIZE)_3.txt ] || [ $$(wc -w /tmp/brysoncg/$(SIZE)_2.txt | grep -E -o -e "^[0-9]{1,15}") -ne $$(( $(SIZE) * $(SIZE) )) ] ) && \
	./generator $(SIZE) > /tmp/brysoncg/$(SIZE)_3.txt ) || true
	-@( ( [ ! -f /tmp/brysoncg/$(SIZE)_4.txt ] || [ $$(wc -w /tmp/brysoncg/$(SIZE)_2.txt | grep -E -o -e "^[0-9]{1,15}") -ne $$(( $(SIZE) * $(SIZE) )) ] ) && \
	./generator $(SIZE) > /tmp/brysoncg/$(SIZE)_4.txt ) || true
	-@( ( [ ! -f /tmp/brysoncg/$(SIZE)_5.txt ] || [ $$(wc -w /tmp/brysoncg/$(SIZE)_2.txt | grep -E -o -e "^[0-9]{1,15}") -ne $$(( $(SIZE) * $(SIZE) )) ] ) && \
	./generator $(SIZE) > /tmp/brysoncg/$(SIZE)_5.txt ) || true
	-@( ( [ ! -f /tmp/brysoncg/$(SIZE)_6.txt ] || [ $$(wc -w /tmp/brysoncg/$(SIZE)_2.txt | grep -E -o -e "^[0-9]{1,15}") -ne $$(( $(SIZE) * $(SIZE) )) ] ) && \
	./generator $(SIZE) > /tmp/brysoncg/$(SIZE)_6.txt ) || true
	-@( ( [ ! -f /tmp/brysoncg/$(SIZE)_1-2.txt ] || [ $$(wc -w /tmp/brysoncg/$(SIZE)_1-2.txt | grep -E -o -e "^[0-9]{1,15}") -ne $$(( $(SIZE) * $(SIZE) )) ] ) && \
	time (./strassen_int_opt_fasterer /tmp/brysoncg/$(SIZE)_1.txt /tmp/brysoncg/$(SIZE)_2.txt $(SIZE) > /tmp/brysoncg/$(SIZE)_1-2.txt) ) || true
	-@( ( [ ! -f /tmp/brysoncg/$(SIZE)_2-3.txt ] || [ $$(wc -w /tmp/brysoncg/$(SIZE)_1-2.txt | grep -E -o -e "^[0-9]{1,15}") -ne $$(( $(SIZE) * $(SIZE) )) ] ) && \
	time (./strassen_int_opt_fasterer /tmp/brysoncg/$(SIZE)_2.txt /tmp/brysoncg/$(SIZE)_3.txt $(SIZE) > /tmp/brysoncg/$(SIZE)_2-3.txt) ) || true
	-@( ( [ ! -f /tmp/brysoncg/$(SIZE)_3-4.txt ] || [ $$(wc -w /tmp/brysoncg/$(SIZE)_1-2.txt | grep -E -o -e "^[0-9]{1,15}") -ne $$(( $(SIZE) * $(SIZE) )) ] ) && \
	time (./strassen_int_opt_fasterer /tmp/brysoncg/$(SIZE)_3.txt /tmp/brysoncg/$(SIZE)_4.txt $(SIZE) > /tmp/brysoncg/$(SIZE)_3-4.txt) ) || true
	-@( ( [ ! -f /tmp/brysoncg/$(SIZE)_4-5.txt ] || [ $$(wc -w /tmp/brysoncg/$(SIZE)_1-2.txt | grep -E -o -e "^[0-9]{1,15}") -ne $$(( $(SIZE) * $(SIZE) )) ] ) && \
	time (./strassen_int_opt_fasterer /tmp/brysoncg/$(SIZE)_4.txt /tmp/brysoncg/$(SIZE)_5.txt $(SIZE) > /tmp/brysoncg/$(SIZE)_4-5.txt) ) || true
	-@( ( [ ! -f /tmp/brysoncg/$(SIZE)_5-6.txt ] || [ $$(wc -w /tmp/brysoncg/$(SIZE)_1-2.txt | grep -E -o -e "^[0-9]{1,15}") -ne $$(( $(SIZE) * $(SIZE) )) ] ) && \
	time (./strassen_int_opt_fasterer /tmp/brysoncg/$(SIZE)_5.txt /tmp/brysoncg/$(SIZE)_6.txt $(SIZE) > /tmp/brysoncg/$(SIZE)_5-6.txt) ) || true
	-@( ( [ ! -f /tmp/brysoncg/$(SIZE)_6-1.txt ] || [ $$(wc -w /tmp/brysoncg/$(SIZE)_1-2.txt | grep -E -o -e "^[0-9]{1,15}") -ne $$(( $(SIZE) * $(SIZE) )) ] ) && \
	time (./strassen_int_opt_fasterer /tmp/brysoncg/$(SIZE)_6.txt /tmp/brysoncg/$(SIZE)_1.txt $(SIZE) > /tmp/brysoncg/$(SIZE)_6-1.txt) ) || true

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
