//
// food_festival_sim_beta.cpp
//
// Course: Systems Software, Assignment: 05
// Fall Semester 2013
// University of Zurich
//
// Created on: 20.11.2013
// Author: Samuel von Baussnern
//
// Beta version
//

#include <iostream>
#include <vector>
#include <random>

#include <cstdlib>
#include <cstddef>
#include <cstdint>

#include <pthread.h>
#include <time.h>

#include "general_utils.h"

using std::cout;
using std::endl;
using std::cin;
using std::vector;
using std::size_t;

// To ensure that I can convert the milliseconds to nanoseconds.
typedef long long sleep_t;
typedef vector<pthread_t> thread_store_t;

// Evil globals:
thread_store_t threads;
pthread_mutex_t thread_store_lock = PTHREAD_MUTEX_INITIALIZER;
bool KEEP_RUNNING {true};

void print_usage() {
  cout << "Usage: food_festival_sim [number of visitors] "
    << "[fixed sleep time for attendant (ms)] "
    << "[minimum of sleep time for visitor (ms)] "
    << "[maximum of sleep time for visitor (ms)]"
    << endl;
}

struct food_stand {
	unsigned num_of_samples;
	pthread_mutex_t lock;

	food_stand(unsigned s) :
		num_of_samples(s) {}
};

struct visitor {
	food_stand* food_samples;
	const sleep_t min_sleep;
	const sleep_t max_sleep;
	int id;
	unsigned long long number_of_getting_food;
	sleep_t total_waiting_time;

	visitor(food_stand* fs, sleep_t min_s, sleep_t max_s, int i)
		: food_samples(fs)
		, min_sleep(min_s)
		, max_sleep(max_s)
		, id(i)
		, number_of_getting_food(0)
		, total_waiting_time(0) {}
};

struct attendant {
	food_stand* food_samples;
	sleep_t waiting_time;

	timespec delay;

	attendant(food_stand* fs, sleep_t wt)
		: food_samples(fs)
		, waiting_time(wt)
	{
        delay.tv_sec = waiting_time / 1000;
        delay.tv_nsec = (long) (waiting_time - (delay.tv_sec * 1000)) * 1000000;

        cout << "Attendant created" << endl;
	}

	~attendant() {
		delete food_samples;
	}
};


void init_attendant (food_stand* fs, sleep_t waiting_time);
void init_visitors (food_stand* fs, int number_of_visitors, sleep_t min_sleep_time, sleep_t max_sleep_time, vector<visitor*>& visitor_store);

void* input_listener (void *);
void* run_attendant (void* arg);
void* run_visitor (void* arg);


int main(int argc, char** argv) {
  if (argc != 5) print_usage();

  const int number_of_visitors {std::atoi(argv[1])};
  const sleep_t attendant_sleep_time {std::atoll(argv[2])};
  const sleep_t visitor_min_sleep_time {std::atoll(argv[3])};
  const sleep_t visitor_max_sleep_time {std::atoll(argv[4])};

  food_stand* fs = new food_stand(10);

  if (pthread_mutex_init(&fs->lock, NULL) != 0) {
	  err_exit(0, "Couldn't init lock on food_stand");
  }

  //
  // Pthread creation
  //
  // Input listener
  int err;
  pthread_t input_listener_id;
  err = pthread_create(&input_listener_id, NULL, input_listener, NULL);
  if (err != 0) {
	err_exit(err, "Can't create thread!");
  } else {
	pthread_mutex_lock(&thread_store_lock);
	threads.push_back(input_listener_id);
	pthread_mutex_unlock(&thread_store_lock);
  }

  // Attendant
  init_attendant(fs, attendant_sleep_time);

  // Visitors
  vector<visitor*> visitors;
  init_visitors(fs, number_of_visitors, visitor_min_sleep_time, visitor_max_sleep_time, visitors);

  //
  // Pthread joining
  //
  for (size_t i {0}, e {threads.size()}; i < e; ++i) {
	  int err = pthread_join(threads[i], NULL);
	  if (err != 0)
	    err_exit(err, "Can't join with thread!");
  }

  // Output
  cout << "Visitor Id\tAte that much\tAverage waiting time [ms]" << endl;
  for (size_t i {0}, e {visitors.size()}; i < e; ++i) {
	  cout << i << "\t" << visitors[i]->number_of_getting_food
			  << "\t" << (double)visitors[i]->total_waiting_time / (visitors[i]->number_of_getting_food != 0 ? visitors[i]->number_of_getting_food : 1)
			  << endl;

	  delete visitors[i];
  }

  return 0;
}

void init_attendant (food_stand* fs, sleep_t waiting_time) {
	attendant* a = new attendant(fs, waiting_time);

	pthread_t a_id;


	int err = pthread_create(&a_id, NULL, run_attendant, static_cast<void*>(a));
	if (err != 0) {
		err_exit(err, "Can't create thread!");
	} else {
		pthread_mutex_lock(&thread_store_lock);
		threads.push_back(a_id);
		pthread_mutex_unlock(&thread_store_lock);
	}
}

void init_visitors (food_stand* fs, int number_of_visitors, sleep_t min_sleep_time, sleep_t max_sleep_time, vector<visitor*>& visitor_store) {
	while (number_of_visitors--) {
		visitor* v  = new visitor(fs, min_sleep_time, max_sleep_time, number_of_visitors);

		pthread_t v_id;

		int err = pthread_create(&v_id, NULL, run_visitor, static_cast<void*>(v));
		if (err != 0) {
			err_exit(err, "Can't create thread!");
		} else {
			pthread_mutex_lock(&thread_store_lock);
			threads.push_back(v_id);
			visitor_store.push_back(v);
			pthread_mutex_unlock(&thread_store_lock);
		}
	}
}

void* input_listener (void *) {
  char user_input;

  cout << "Press any button any time to stop the simulation" << endl;
  cin >> user_input;

  KEEP_RUNNING = false;
  return static_cast<void*>(0);
}

void* run_attendant(void* arg) {
	attendant* a {static_cast<attendant*>(arg)};

	while (KEEP_RUNNING) {
		pthread_mutex_lock(&(a->food_samples->lock));
		if (a->food_samples->num_of_samples < 10) {
			a->food_samples->num_of_samples = 10;
			cout << "Attendant filled up the plates." << endl;}
		pthread_mutex_unlock(&(a->food_samples->lock));

		nanosleep(&(a->delay), NULL);
	}

	delete a;

	return static_cast<void*>(0);
}

void* run_visitor(void* arg) {
	visitor* v {static_cast<visitor*>(arg)};

	while (KEEP_RUNNING) {
		// Only request the lock if it's necessary.
		if (v->food_samples->num_of_samples > 0) {
			pthread_mutex_lock(&(v->food_samples->lock));
			if (v->food_samples->num_of_samples > 0) {
				--(v->food_samples->num_of_samples);
				++(v->number_of_getting_food);
				cout << "Visitor " << v->id << " took a sample. " << v->food_samples->num_of_samples << " samples left." << endl;
			}
			pthread_mutex_unlock(&(v->food_samples->lock));
		}

		timespec delay;
		unsigned int diff = v->max_sleep - v->min_sleep;
		sleep_t delay_ms = v->min_sleep;
		if (diff != 0) {
				delay_ms += rand() % diff; // Pretty bad algorithm... Since modulo will not preserve the pseudorandomness.
		}
		delay.tv_sec = delay_ms / 1000;
		delay.tv_nsec = (delay_ms - (delay.tv_sec * 1000)) * 1000000;
		v->total_waiting_time += delay_ms;
		nanosleep(&delay, NULL);
	}

	return static_cast<void*>(0);
}
