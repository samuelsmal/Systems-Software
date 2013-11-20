all : food_festival_sim

food_festival_sim : food_festival_sim.o
	g++ bin/food_festival_sim.o -pthread -o bin/food_festival_sim
	#./bin/food_festival_sim 10 300 0 100

food_festival_sim.o : src/food_festival_sim.cpp
	g++ -c -pthread -std=c++11 -Wall -o bin/food_festival_sim.o src/food_festival_sim.cpp

clean :
	rm -rf **/*.o food_festival_sim
