/*
 * general_utils.h
 *
 *  Created on: 21 Nov 2013
 *      Author: samuel
 */

#ifndef GENERAL_UTILS_H_
#define GENERAL_UTILS_H_

#include <iostream>

void err_exit(int error_code, const char* error_message) {
  std::cout << "Error code:" << error_code << "\n\t" << error_message << std::endl;
  exit(EXIT_FAILURE);
}

#endif /* GENERAL_UTILS_H_ */
