.PHONY: prog

CFLAGS=-ansi -g  -std=c++11 -Wall -O6
CFLAGSO0=-ansi -g  -std=c++11 -Wall 

all: prog

O0: main.cpp
	g++ $(CFLAGSO0) main.cpp -o fptree_o0.out -lpthread

prog: main.cpp 
	g++ $(CFLAGS) main.cpp -o fptree.out -lpthread

clean:
	rm -f *.o
	rm -f *.exe
	rm -f *.out
