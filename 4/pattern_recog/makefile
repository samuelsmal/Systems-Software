all: pattern_recog

pattern_recog : main.o
	g++ -lgomp -std=c++11 main.o -o bin/patt_rec

main.o: main.cpp
	g++ -fopenmp -c -std=c++11 -O3 -Wall main.cpp

clean:
	rm -rf *o pattern_recog
