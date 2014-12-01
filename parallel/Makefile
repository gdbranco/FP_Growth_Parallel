.PHONY: prog

CFLAGS=-ansi -g  -std=c++11 -Wall -O6

all: prog

prog: main.cpp 
	g++ $(CFLAGS) main.cpp -o fptree.out -lpthread

clean:
	rm -f *.o
	rm -f *.exe
	rm -f *.out
	rm -f fptree
