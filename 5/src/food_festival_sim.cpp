//
// food_festival_sim.cpp
//
// Course: Systems Software, Assignment: 05
// Fall Semester 2013
// University of Zurich
//
// Created on: 20.11.2013
// Author: Samuel von Baussnern
//

#include <iostream>
#include <vector>
#include <random>
#include <list>
#include <memory>

#include <cstdlib>
#include <cstddef>
#include <cstdint>

#include <pthread.h>
#include <time.h>

#include "general_utils.h"

using std::cout;
using std::endl;
using std::cin;
using std::list;
using std::vector;
using std::size_t;
using std::shared_ptr;
using std::unique_ptr;

// To ensure that I can convert the milliseconds to nanoseconds.
typedef long long sleep_t;
typedef vector<pthread_t> thread_store_t;

// Evil globals:
thread_store_t threads;
pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
bool shouldAbort = false;
vector<unsigned long> plate_counter;

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

	~food_stand() {
		cout << "food stand deleted" << endl;
	}
};

struct visitor {
	int id;
	sleep_t min_sleep;
	sleep_t max_sleep;
	food_stand* food_samples;

	visitor(int i, sleep_t min_s, sleep_t max_s, food_stand* fs) :
		id(i) , min_sleep(min_s), max_sleep(max_s), food_samples(fs) {}

	~visitor() {
		cout << "visitor " << id << " deleted" << endl;
	}
};

struct attendant {
	sleep_t waiting_time;
	food_stand* food_samples;

	attendant(sleep_t wt, food_stand* fs) :
		waiting_time(wt), food_samples(fs) {}
};

void* input_listener (void *);
void init_attendant (food_stand* fs, sleep_t waiting_time);
void init_visitors (food_stand* fs, int number_of_visitors, sleep_t min_sleep_time, sleep_t max_sleep_time);

void* run_attendant (void* arg);
void* run_visitor (void* arg);


int main(int argc, char** argv) {
  if (argc != 5) print_usage();

  const int number_of_visitors {std::atoi(argv[1])};
  const sleep_t attendant_sleep_time {std::atoll(argv[2])};
  const sleep_t visitor_min_sleep_time {std::atoll(argv[3])};
  const sleep_t visitor_max_sleep_time {std::atoll(argv[4])};

  shared_ptr<food_stand> fs(new food_stand(10));

  if (pthread_mutex_init(&fs->lock, NULL) != 0) {
	  err_exit(0, "Couldn't init lock on food_stand");
  }

  //
  // Pthread creation
  //
  // Input listener
  /*
  int err;
  pthread_t input_listener_id;
  err = pthread_create(&input_listener_id, NULL, input_listener, NULL);
  if (err != 0) {
	err_exit(err, "Can't create thread!");
  } else {
	pthread_mutex_lock(global_mutex);
	threads.push_back(input_listener_id);
	pthread_mutex_unlock(global_mutex);
  }*/

  // Attendant


  //
  // Pthread joining
  //
/*
  for (size_t i {0}, e {threads.size()}; i < e; ++i) {
	  int err = pthread_join(threads[i], NULL);
	  if (err != 0)
	    err_exit(err, "Can't join with thread!");
  }*/
  return 0;
}

void* input_listener (void *) {
  char user_input;

  cout << "Press any button any time to stop the simulation" << endl;

  do {
    cin >> user_input;
  } while (!user_input);

  exit(EXIT_SUCCESS);
}

void init_attendant (food_stand* fs, sleep_t waiting_time) {
	attendant* a = new attendant(fs, waiting_time);

	pthread_t a_id;


	int err = pthread_create(&a_id, NULL, run_attendant, static_cast<void*>(a));
	if (err != 0) {
		err_exit(err, "Can't create thread!");
	} else {
		pthread_mutex_lock(global_mutex);
		threads.push_back(a_id);
		pthread_mutex_unlock(global_mutex);
	}
}

void init_visitors (food_stand* fs, int number_of_visitors, sleep_t min_sleep_time, sleep_t max_sleep_time) {
	while (number_of_visitors--) {
		unique_ptr<visitor> v (new visitor(number_of_visitors, fs, min_sleep_time, max_sleep_time));

		pthread_t v_id;

		int err = pthread_create(&v_id, NULL, run_visitor, static_cast<void*>(v));
		if (err != 0) {
			err_exit(err, "Can't create thread!");
		} else {
			pthread_mutex_lock(global_mutex);
			threads.push_back(v_id);
			pthread_mutex_unlock(global_mutex);
		}
	}
}
