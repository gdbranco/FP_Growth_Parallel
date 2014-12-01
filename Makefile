.PHONY: prog

CFLAGS=-ansi -g  -std=c++11 -Wall 

all: prog

prog: main.cpp 
	g++ $(CFLAGS) main.cpp -o fptree.out -fopenmp

clean:
	rm -f *.o
	rm -f *.exe
	rm -f *.out
	rm -f fptree
