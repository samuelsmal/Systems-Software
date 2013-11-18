#include <iostream>
#include <omp.h>
#include <vector>
#include <fstream>
#include <algorithm>
#include <utility>
#include <limits>

int absolute_value(int a) {
  if (a < 0) a *= -1;
  return a;
}

void error_exit(std::string message) {
  std::cout << "ERROR: " << message << std::endl;
  exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
  if (argc < 5) {
    std::cout << "Usage: patt_rec [data_matrix.txt] [pattern_matrix.txt] [mode 0: serial, 1: parallel] [number of threads]" << std::endl;
    return 1;
  }

  int data_n_rows, data_n_cols;

  std::ifstream data_file(argv[1]);
  if (!data_file) error_exit("Data file not found!");
  data_file >> data_n_rows >> data_n_cols;

  int data[data_n_rows][data_n_cols];

  for (int i = 0; i < data_n_rows; ++i)
  {
    for (int j = 0; j < data_n_cols; ++j)
    {
      data_file >> data[i][j];
    }
  }

  int pattern_n_rows, pattern_n_cols;

  std::ifstream pattern_file(argv[2]);
  if (!pattern_file) error_exit("Pattern file not found");
  pattern_file >> pattern_n_rows >> pattern_n_cols;

  int pattern[pattern_n_rows][pattern_n_cols];

  std::vector<int> pattern_elements;

  for (int i = 0; i < pattern_n_rows; ++i)
  {
    for (int j = 0; j < pattern_n_cols; ++j)
    {
      pattern_file >> pattern[i][j];
      pattern_elements.push_back(pattern[i][j]);
    }
  }

  std::nth_element(pattern_elements.begin(),
                   pattern_elements.begin() + pattern_elements.size()/2,
                   pattern_elements.end());
  const int median_pattern = pattern_elements[pattern_elements.size()/2];


  std::vector<std::tuple<int, int, double>> matches;

  double size_of_pattern_matrix = (double) pattern_n_cols * pattern_n_rows; //Evil C way.

  bool is_parallel {atoi(argv[3]) == 1}; //Absolutly horribly... but works. Should be checked with a stream.

  double time_start = omp_get_wtime();
  int num_of_threads = atoi(argv[4]);
  int chunk_size = data_n_rows / (num_of_threads * 10);
  #pragma omp parallel for num_threads(num_of_threads) if(is_parallel) schedule(guided, chunk_size)
  for (int i = 0; i < data_n_rows - pattern_n_rows; ++i) {
    std::cout << omp_get_thread_num() << std::endl;
    for (int j = 0; j < data_n_cols - pattern_n_cols; ++j) {
      double current_goodness = 0;
      std::vector<int>submatrix_elements;

      for (int m_i = 0; m_i < pattern_n_rows; ++m_i) {
        for (int m_j = 0; m_j < pattern_n_cols; ++m_j) {
          int diff = absolute_value(data[i+m_i][j+m_j] - pattern[m_i][m_j]);
          current_goodness += (double)diff;
          submatrix_elements.push_back(data[i+m_i][j+m_j]);
          if (diff > 9)
            goto BREAK_OUUUUUUUT;
        }
      }

      std::nth_element(submatrix_elements.begin(),
                       submatrix_elements.begin() + submatrix_elements.size()/2,
                       submatrix_elements.end());

      if (absolute_value(submatrix_elements[submatrix_elements.size()/2] - median_pattern) > 2)
        goto BREAK_OUUUUUUUT;

      current_goodness /= size_of_pattern_matrix;

      #pragma omp critical
      matches.push_back(std::make_tuple(i, j, current_goodness));


BREAK_OUUUUUUUT : ;
    }
  }

  double time_end = omp_get_wtime();

  std::cout << "Took " << (time_end - time_start) << " seconds" << std::endl;

  std::cout << "Matches (row, col, goodness)" << std::endl;
  double min_goodness {std::numeric_limits<double>::max()}, max_goodness {std::numeric_limits<double>::min()}, average_goodness {0};
  for (auto p : matches) {
    std::cout << "(" << std::get<0>(p) << ", " << std::get<1>(p) << ", " << std::get<2>(p) << ")" << std::endl;
    double current_goodness = std::get<2>(p);
    if (current_goodness < min_goodness) min_goodness = current_goodness;
    if (current_goodness > max_goodness) max_goodness = current_goodness;
    average_goodness += current_goodness;
  }

  std::cout << "Min goodness: " << min_goodness << std::endl
            << "Max goodness: " << max_goodness << std::endl
            << "Average goodness: " << average_goodness / matches.size() << std::endl;

  return 0;
}
