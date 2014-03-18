thread_comp: thread_comp.o
	g++ -o thread_comp thread_comp.o -std=c++0x -pthread

thread_comp.o: thread_comp.cpp
	g++ -c thread_comp.cpp -std=c++0x -pthread

clean:
	-rm -rf *.o
	-rm -rf thread_comp
