all: mpm

mpm : main.o
	g++ -pthread main.o -o bin/mpm
	#./mpm sequence.txt patterns.txt 10 1

main.o: main.cpp
	g++ -c -std=c++11 -Wall main.cpp

clean:
	rm -rf *o mpm

mpm-clang : main-clang.o
	clang -pthread main.o -o bin/mpm-clang

main-clang.o	: main.cpp
	clang -c -std=c++11 main.cpp
