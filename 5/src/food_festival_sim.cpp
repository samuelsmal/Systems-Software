//
// food_festival_sim.cpp
//
// Course: Systems Software, Assignment: 05
// Fall Semester 2013
// University of Zurich
//
// Created on: 24.11.2013
// Author: Samuel von Baussnern
//
// Uses a FIFO queue to enlist the visitors.
//

#include <iostream>
#include <vector>
#include <random>
#include <queue>

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
using std::queue;
using std::size_t;

// To ensure that I can convert the milliseconds to nanoseconds.
typedef long long sleep_t;
typedef vector<pthread_t> thread_store_t;

void print_usage() {
  cout << "Usage: food_festival_sim [number of visitors] "
    << "[fixed sleep time for attendant (ms)] "
    << "[minimum of sleep time for visitor (ms)] "
    << "[maximum of sleep time for visitor (ms)]"
    << endl;
}

//================================================================================
// Declarations
//================================================================================

struct FoodStand {
	unsigned num_of_samples;
	pthread_mutex_t lock;

	FoodStand(unsigned s) :
		num_of_samples(s) {
	  if (pthread_mutex_init(&lock, NULL) != 0) {
	    err_exit(0, "Couldn't init lock on food_stand");
	  }
	}
};

struct Attendant {
  sleep_t waiting_time;

  timespec delay;

  Attendant(sleep_t wt) : waiting_time{wt}
  {
        delay.tv_sec = waiting_time / 1000;
        delay.tv_nsec = (long) (waiting_time - (delay.tv_sec * 1000)) * 1000000;
  }
};

struct Visitor {
	const sleep_t min_sleep;
	const sleep_t max_sleep;
	int id;
	unsigned long long number_of_getting_food;
	sleep_t total_waiting_time;

	bool is_enqueued;
	bool has_food;

	Visitor(sleep_t min_s, sleep_t max_s, int i)
		: min_sleep{min_s}
		, max_sleep{max_s}
		, id{i}
		, number_of_getting_food{0}
		, total_waiting_time{0}
		, is_enqueued {false}
		, has_food {false} {}
};

void initAttendant (sleep_t waiting_time);
void initVisitors (int number_of_visitors,
		sleep_t min_sleep_time, sleep_t max_sleep_time,
		vector<Visitor*>& visitor_store);

//================================================================================
// Declr.: Pthread function
//================================================================================

void* handleInput(void *);
void* handleAttendant(void* arg);
void* handleVisitor(void* arg);
void* handleFoodDistribution(void* arg);

//================================================================================
// Evil globals
//================================================================================

thread_store_t threads;
pthread_mutex_t thread_store_lock = PTHREAD_MUTEX_INITIALIZER;

queue<Visitor*> visitor_queue;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;

FoodStand food_stand {10};

bool KEEP_RUNNING {true};

//================================================================================
// Definitions
//================================================================================

int main(int argc, char** argv) {
  if (argc != 5) print_usage();

  //==============================================================================
  // Input parsing
  //==============================================================================

  const int number_of_visitors {std::atoi(argv[1])};
  const sleep_t attendant_sleep_time {std::atoll(argv[2])};
  const sleep_t visitor_min_sleep_time {std::atoll(argv[3])};
  const sleep_t visitor_max_sleep_time {std::atoll(argv[4])};

  //==============================================================================
  // Some initialization
  //==============================================================================

  srand(time(0));

  //==============================================================================
  // Pthread creation
  //==============================================================================

  // Input listener
  int err;
  pthread_t input_listener_id;
  err = pthread_create(&input_listener_id, NULL, handleInput, NULL);
  if (err != 0) {
    err_exit(err, "Can't create thread!");
  } else {
    pthread_mutex_lock(&thread_store_lock);
    threads.push_back(input_listener_id);
    pthread_mutex_unlock(&thread_store_lock);
  }

  // Food distribution
  pthread_t food_distr_id;
  err = pthread_create(&food_distr_id, NULL, handleFoodDistribution, NULL);
  if (err != 0) {
    err_exit(err, "Can't create thread!");
  } else {
    pthread_mutex_lock(&thread_store_lock);
    threads.push_back(food_distr_id);
    pthread_mutex_unlock(&thread_store_lock);
  }

  // Attendant
  initAttendant(attendant_sleep_time);

  // Visitors
  vector<Visitor*> visitors;
  queue<Visitor*> visitor_queue;
  initVisitors(number_of_visitors,
               visitor_min_sleep_time, visitor_max_sleep_time,
               visitors);


  //==============================================================================
  // Pthread joining
  //==============================================================================

  for (size_t i {0}, e {threads.size()}; i < e; ++i) {
	  int err = pthread_join(threads[i], NULL);
	  if (err != 0)
	    err_exit(err, "Can't join with thread!");
  }

  //==============================================================================
  // Output
  //==============================================================================

  cout << "\nVisitor Id\tAte that much\tAverage waiting time [ms]" << endl;
  for (size_t i {0}, e {visitors.size()}; i < e; ++i) {
	  cout << i << "\t\t"
	       << visitors[i]->number_of_getting_food << "\t\t"
			   << (double)visitors[i]->total_waiting_time
			     / (
             visitors[i]->number_of_getting_food != 0
             ? visitors[i]->number_of_getting_food : 1
           )
			   << endl;

	  delete visitors[i];
  }
  cout << endl;

  return 0;
}

void initAttendant (sleep_t waiting_time) {
	Attendant* a = new Attendant(waiting_time);

	pthread_t a_id;

	int err = pthread_create(&a_id, NULL, handleAttendant, static_cast<void*>(a));
	if (err != 0) {
		err_exit(err, "Can't create thread!");
	} else {
		pthread_mutex_lock(&thread_store_lock);
		threads.push_back(a_id);
		pthread_mutex_unlock(&thread_store_lock);
	}
}

void initVisitors (int number_of_visitors,
                   sleep_t min_sleep_time, sleep_t max_sleep_time,
                   vector<Visitor*>& visitor_store) {
	while (number_of_visitors--) {
		Visitor* v  = new Visitor(min_sleep_time, max_sleep_time, number_of_visitors);

		pthread_t v_id;
		int err = pthread_create(&v_id, NULL, handleVisitor, static_cast<void*>(v));
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

//================================================================================
// Def.: Pthread functions
//================================================================================

void* handleInput (void *) {
  char user_input;

  cout << "Press any button any time to stop the simulation" << endl;
  cin >> user_input;

  KEEP_RUNNING = false;
  return static_cast<void*>(0);
}

void* handleAttendant(void* arg) {
  Attendant* a {static_cast<Attendant*>(arg)};

	while (KEEP_RUNNING) {
		pthread_mutex_lock(&food_stand.lock);
		if (food_stand.num_of_samples < 10) {
			food_stand.num_of_samples = 10;
			cout << "Attendant filled up the plates." << endl;}
		pthread_mutex_unlock(&food_stand.lock);

		nanosleep(&(a->delay), NULL);
	}

	delete a;

	return static_cast<void*>(0);
}

void* handleVisitor(void* arg) {
	Visitor* v {static_cast<Visitor*>(arg)};

	while (KEEP_RUNNING) {
		if (!(v->is_enqueued)) {
      pthread_mutex_lock(&queue_lock);
      visitor_queue.push(v);
      pthread_mutex_unlock(&queue_lock);

      v->is_enqueued = true;

		} else if (v->has_food) {
      timespec delay;
      unsigned int diff = v->max_sleep - v->min_sleep;
      sleep_t delay_ms = v->min_sleep;
      if (diff != 0) {
          delay_ms += rand() % diff; // Pretty bad algorithm... Since modulo will not preserve the pseudorandomness.
      }
      delay.tv_sec = delay_ms / 1000;
      delay.tv_nsec = (delay_ms - (delay.tv_sec * 1000)) * 1000000;
      nanosleep(&delay, NULL);

      v->total_waiting_time += delay_ms;
      ++v->number_of_getting_food;
      v->has_food = false;
		}
	}

	return static_cast<void*>(0);
}

void* handleFoodDistribution(void* arg) {
  Visitor* current_visitor;
  while(KEEP_RUNNING) {
    pthread_mutex_lock(&queue_lock);
    pthread_mutex_lock(&food_stand.lock);
    if (!visitor_queue.empty() && food_stand.num_of_samples > 0) {

      current_visitor = visitor_queue.front();
      visitor_queue.pop();

      current_visitor->is_enqueued = false;
      current_visitor->has_food = true;

      --food_stand.num_of_samples;

      cout << "Visitor " << current_visitor->id << " took a sample. "
           << food_stand.num_of_samples << " samples left." << endl;
    }
    pthread_mutex_unlock(&food_stand.lock);
    pthread_mutex_unlock(&queue_lock);
  }

  return static_cast<void*>(0);
}
