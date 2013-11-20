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
#include <thread>
#include <chrono>

using std::cout;
using std::endl;
using std::cin;

pthread_mutex_t plate_lock = PTHREAD_MUTEX_INITIALIZER;
std::list<bool> food_samples;

class Rand_int {
 private:
  std::uniform_int_distribution<>::param_type p;
  auto r = std::bind(std::uniform_int_distribution<>{p}, std::default_random_engine{});
 public:
  Rand_int(int lo, int hi) : p{lo, hi} {}
  int operator()() const {return r();}

};

void print_usage() {
  cout << "Usage: food_festival_sim [number of visitors] "
    << "[fixed sleep time for attendant (ms)] "
    << "[minimum of sleep time for visitor (ms)] "
    << "[maximum of sleep time for visitor (ms)]"
    << endl;
}

void err_exit(int error_code, const char* error_message) {
  cout << "Error code:" << error_code << "\n\t" << error_message << endl;
  exit(EXIT_FAILURE);
}

void* input_listener (void *) {
  char user_input;

  cout << "Press any button any time to stop the simulation" << endl;

  do {
    cin >> user_input;
  } while (!user_input);

  exit(EXIT_SUCCESS);
}

void fill_food () {
  while(food_samples.size() < 10)
    food_samples.push_back(true);
}

void* visitor (void* arg) {
  while(true) {
//    std::uniform_int_distribution<>* dist = (std::uniform_int_distribution<> *)arg;
    Rand_int* r = (Rand_int *)arg;

    pthread_mutex_lock(&plate_lock);
    if (!food_samples.empty()) food_samples.pop_back();
    phtread_mutex_unlock(&plate_lock);

    std::this_thread::sleep_for(std::chrono::milliseconds(r()));
  }
}

void* attendant (void* arg) {
  while (true) {
    pthread_mutex_lock(&plate_lock);
    fill_food();
    phtread_mutex_unlock(&plate_lock);

    std::this_thread::sleep_for(std::chrono::milliseconds(*(int*)arg));
  }
}

int main(int argc, char** argv) {
  if (argc != 5) print_usage();

  const int number_of_visitors {std::atoi(argv[1])};
  const int attendant_sleep_time {std::atoi(argv[2])};
//  const int visitor_min_sleep_time {std::atoi(argv[3])};
//  const int visitor_max_sleep_time {std::atoi(argv[4])};

  fill_food();

//  std::random_device rd;
//  std::mt19937 gen(rd());
//  std::uniform_int_distribution<> dis_visitor(std::atoi(argv[3]), std::atoi(argv[4]) + 1);


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
  Rand_int rand_vis {(std::atoi(argv[3]), std::atoi(argv[4]) + 1)};
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
