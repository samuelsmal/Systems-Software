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
#include <cstdlib>
#include <pthread.h>
#include <random>
#include <list>
#include <time.h>

#include "general_utils.h"
#include "food_sim_util.h"

using std::cout;
using std::endl;
using std::cin;

typedef std::list<bool> food_samples_t;

void print_usage() {
  cout << "Usage: food_festival_sim [number of visitors] "
    << "[fixed sleep time for attendant (ms)] "
    << "[minimum of sleep time for visitor (ms)] "
    << "[maximum of sleep time for visitor (ms)]"
    << endl;
}

int main(int argc, char** argv) {
  if (argc != 5) print_usage();

  const int number_of_visitors {std::atoi(argv[1])};
  const int attendant_sleep_time {std::atoi(argv[2])};
//  const int visitor_min_sleep_time {std::atoi(argv[3])};
//  const int visitor_max_sleep_time {std::atoi(argv[4])};

  pthread_mutex_t plate_lock = PTHREAD_MUTEX_INITIALIZER;

  food_samples_t food_samples;
  fill_food(food_samples);

  int err;

  //
  // Pthread creation
  //

  // Input listener
  pthread_t input_listener_id;
  err = pthread_create(&input_listener_id, NULL, input_listener, NULL);
  if (err != 0)
    err_exit(err, "Can't create thread!");

  // Attendant
  pthread_t attendant_id;
  err = pthread_create(&attendant_id, NULL, attendant, (void *)attendant_sleep_time);
  if (err != 0)
    err_exit(err, "Can't create thread!");

  // Visitors
  pthread_t visitors_id[number_of_visitors];

  for (int i {0}; i < number_of_visitors; ++i) {

    err = pthread_create(&visitors_id[i], NULL, visitor, (void *)rand_vis);
    if (err != 0)
      err_exit(err, "Can't create thread!");
  }

  //
  // Pthread joining
  //

  err = pthread_join(input_listener_id, NULL);
  if (err != 0)
    err_exit(err, "Can't join with thread!");

  err = pthread_join(attendant_id, NULL);
  if (err != 0)
    err_exit(err, "Can't join with thread!");

  for (int i {0}; i < number_of_visitors; ++i) {
    err = pthread_join(visitors_id[i], NULL);
    if (err != 0)
      err_exit(err, "Can't join with thread!");
  }

  return 0;
}
