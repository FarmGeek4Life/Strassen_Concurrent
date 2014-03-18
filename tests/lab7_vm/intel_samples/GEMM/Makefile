HEADERS=cmdoptions.hpp ../common/basic.hpp ../common/cmdparser.hpp ../common/oclobject.hpp
SOURCES=cmdoptions.cpp gemm.cpp ../common/basic.cpp ../common/cmdparser.cpp ../common/oclobject.cpp

all: gemm

gemm: $(HEADERS) $(SOURCES) Makefile
	g++ $(SOURCES) -I../common -lOpenCL -ogemm

clean:
	rm -f gemm

