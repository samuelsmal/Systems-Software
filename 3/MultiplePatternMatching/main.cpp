#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <pthread.h>
#include <utility>
#include <math.h>
#include <stdlib.h>

using std::string;
using std::vector;
using std::strcmp;
using std::pair;

typedef long unsigned int u_long;
typedef std::vector<std::vector<u_long>> pattern_match_vector;

// Evil global
pthread_mutex_t pattern_lock = PTHREAD_MUTEX_INITIALIZER;


struct Range {
	u_long begin, end;
};

struct Arguments {
	Range range;
	std::string* seq;
	std::vector<string>* patterns;
	pattern_match_vector* matches;
};

void print_usage()
{
	std::cout << "Usage: mpm [sequence_file] [pattern_file] [number_of_threads] [0 (serial) | 1 (parallel)]" << std::endl;
}

void err_exit(int error_code, std::string error_message)
{
	std::cout << "Error code:" << error_code << "\n\t" << error_message << std::endl;
	exit(1);
}

void* match_patterns(void *arg)
{
	Arguments* arguments = (Arguments*)arg;
	Range range = arguments->range;
	std::string* sequence = arguments->seq;
	std::vector<string>* patterns = arguments->patterns;
	pattern_match_vector* matches = arguments->matches;
	std::string current_pattern;
	u_long sequence_size {sequence->size()};
	bool pattern_is_good {true};

	for (u_long i = range.begin; i < range.end; ++i)
	{
		size_t k = patterns->size();
		while (k--)
		{
			current_pattern = patterns->at(k);
			std::vector<u_long> found_patterns;
			if (i + current_pattern.size() <= sequence_size)
			{
				pattern_is_good = true;
				// Brute force matching, rather stupid way... using a hashvalue would be better.
				for (u_long m = 0; m < current_pattern.size(); ++m)
				{
					if (current_pattern.at(m) != sequence->at(i + m))
					{
						pattern_is_good = false;
					}
				}

				if (pattern_is_good)
				{
					found_patterns.push_back(i);
;				}
			}

			if (!found_patterns.empty())
			{
				pthread_mutex_lock(&pattern_lock);
				matches->at(k).insert(matches->at(k).end(), found_patterns.begin(), found_patterns.end());
				pthread_mutex_unlock(&pattern_lock);
			}

		}
	}

	pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
	if (argc != 5)
	{
		print_usage();
		return 1;
	}

	// Reading in the two files.
	string 			sequence;
	int 			length_of_sequence;
	std::ifstream 	sequence_file(argv[1]);
	sequence_file >> length_of_sequence;
	sequence_file >> sequence;

	vector<string> 	patterns;
	std::ifstream 	pattern_file(argv[2]);
	int 			number_of_pattners;
	string 			current_line;
	pattern_file >> number_of_pattners;
	while(!pattern_file.eof()) {
		// Ignoring the length of the patterns, since I use std::strings.
		pattern_file >> current_line;
		pattern_file >> current_line;
		patterns.push_back(current_line);
	}
	patterns.shrink_to_fit();

	// Determine the run mode.
	int 	number_of_threads = atoi(argv[3]);
	bool 	mode_is_serial;
	switch (atoi(argv[4]))
	{
		case 0:
			mode_is_serial = true;
			break;
		case 1:
			mode_is_serial = false;
			break;
		default:
			print_usage();
			return 1;
	}

	pattern_match_vector matches(patterns.size());
	pthread_mutex_init(&pattern_lock, NULL);

	// Run the pattern matching.
	if (mode_is_serial)
	{
		int		  err;
		pthread_t thread;

		Arguments arguments{{0, sequence.size()}, &sequence, &patterns, &matches};
		err = pthread_create(&thread, NULL, match_patterns, (void*)(&arguments));
		if (err != 0) err_exit(err, "can't create thread!");
		err = pthread_join(thread, NULL);
		if (err != 0) err_exit(err, "can't join thread!");
	}
	else
	{
		int 		err;
		pthread_t 	threads[number_of_threads];

		long 		i;
		u_long 		interval_delta = (u_long)ceil((double)sequence.size() / number_of_threads);
		Arguments 	arguments{{0, 0}, &sequence, &patterns, &matches};

		for (i = 0; i < number_of_threads; ++i)
		{
			arguments.range.begin = arguments.range.end;
			arguments.range.end = (arguments.range.end + interval_delta) < sequence.size()
				? arguments.range.end + interval_delta : sequence.size();
			err = pthread_create(&threads[i], NULL, match_patterns, (void*)(&arguments));
			if (err != 0) err_exit(err, "can't create thread!");
		}

    for (i = 0; i < number_of_threads; ++i)
		{
      err = pthread_join(threads[i], NULL);
			if (err != 0) err_exit(err, "can't join thread!");
		}

	}


	std::ofstream output_file("occurrences.txt");
	for (int l = 0; l < matches.size(); ++l)
	{
		std::vector<u_long> cpm = matches.at(l);
		output_file << cpm.size() << std::endl << "{";
		if (cpm.size() > 0)
		{
			output_file << cpm.at(0);
			for (int ll = 1; ll < cpm.size(); ++ll)
				output_file << ", " << cpm.at(ll);
		}
		output_file << "}" << std::endl;
	}
	output_file << std::endl;

	return 0;
}
