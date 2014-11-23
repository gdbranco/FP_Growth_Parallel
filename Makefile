.PHONY: prog

CFLAGS=-ansi -std=c++11 -Wall 

all: prog

prog: main.cpp 
	g++ $(CFLAGS) main.cpp -o fptree

clean:
	rm -f *.o
	rm -f *.exe
	rm -f fptree
