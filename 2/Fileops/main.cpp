//
//  main.cpp
//  fileops
//

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> // getcwd()

void print_man_page() 
{
	std::cout << "fileops usage:\n    fileops filename\n\n"
			<< "    filename    relative path"
			<< std::endl;
}

void countLines(char* filename)
{
	std::cout << "Counting the lines of " << filename << std::endl;
	execlp("wc", "wc", "-l", filename, (char *)0);
	exit(1);
}

void searchFile(char* filename, const char* search_string)
{
	std::cout << "Searching for " << search_string << " in " << filename << std::endl;
	execlp("grep", "grep", "-r", search_string, filename, (char*)0);
	exit(2);
}

int main(int argc, char * argv[])
{
	if (argc == 2)
	{
		char current_working_directory[1024];
		if (getcwd(current_working_directory, sizeof(current_working_directory)) == NULL) 
		{
			std::cerr << "Error with getcwd()!" << std::endl;
			return -1;
		}
		char filename[1024] = "\0";
		strcat(filename, current_working_directory);
		strcat(filename, "/");
		strcat(filename, argv[1]);

		std::ifstream file_to_work_on(filename, std::ios::binary);
		bool file_exists = file_to_work_on;
		file_to_work_on.close();
		if (! file_exists)
		{
			std::cerr << "No such file in the current working directory!"
				<< "\nCall this program from the right directory." << std::endl;
			std::exit(3);
		}

		int chosen_option{0}, tryouts{0};

		while(chosen_option != 1 && chosen_option != 2 && tryouts < 11) 
		{
			if (tryouts > 0)
			{
				std::cout << "Wrong input."
					<< " You entered: " << chosen_option
					<< "\nTry again! Here are your options again:" << std::endl;
			}

			std::cout << "\nEnter one of the following options:"
				<< "\n   1 for counting the number of lines in the file."
				<< "\n   2 for searching for a specific string in the file."
				<< "\n>>> ";
			std::cin >> chosen_option;
			++tryouts;

			if (tryouts > 9)
			{
				std::cout << "Aborting the program, either you shouldn't be using the command line or something's wrong, really wrong." << std::endl;
				return 1;
			}
		}

		std::string search_string;
		if (chosen_option == 2)
		{
			std::cout << "\n Enter the string to search for:" 
				<< "\n>>> ";
			std::cin >> search_string;
		}
			
		pid_t pid = fork();
		int status;
		if (pid == -1)
		{
			std::cerr << "Error on fork()" << std::endl;
			exit(EXIT_FAILURE);
		}

		if (pid == 0)
		{
			if (chosen_option == 1)
			{
				countLines(filename);
			}
			else 
			{
				searchFile(filename, search_string.c_str());
			}
			exit(EXIT_SUCCESS);
		}
		else
		{
			if (wait(&status) > 0) 
			{
				if (WIFEXITED(status) && WEXITSTATUS(status))
				{
					// an error has occured.
					if (status == 256)
					{
						std::cerr << "Error with grep: Nothing found!" << std::endl;
					}
					else if (WEXITSTATUS(status) == 1)
					{
						std::cerr << "Error with wc: No file!" << std::endl;
					}
					else if (WEXITSTATUS(status) == 2)
					{
						std::cerr << "Error with grep: No file!" << std::endl;
	 				}
				}
				else 
				{
			      /* the program didn't terminate normally no idea what to do then.*/
			    }

			}
		}
	}
	else
	{
		print_man_page();
		return -1;
	}

    return 0;
}