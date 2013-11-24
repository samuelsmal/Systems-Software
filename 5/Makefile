all : precompilingstuff food_festival_sim

# Creates the bin directory iff it doesn't exist.
precompilingstuff :
	mkdir -p bin

food_festival_sim : food_festival_sim.o
	g++ bin/food_festival_sim.o -pthread -o bin/food_festival_sim

food_festival_sim.o : src/food_festival_sim.cpp
	g++ -c -pthread -std=c++11 -Wall -o bin/food_festival_sim.o src/food_festival_sim.cpp

clean :
	rm -rf bin
