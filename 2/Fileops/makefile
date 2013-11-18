all: fileops

fileops : main.o
	g++ main.o -o bin/fileops

main.o: main.cpp
	g++ -c -std=c++11 -Wall main.cpp

clean:
	rm -rf *o fileops
