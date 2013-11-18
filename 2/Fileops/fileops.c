void print_man_page() 
{
	std::cout << "fileops usage:\n    fileops filename\n\n"
			<< "    filename    relative path"
			<< std::endl;
}

void countLines(char* filename)
{
	std::cout << "Count the lines..." << std::endl;

	int pid = fork();
	//int status;

	//pid_t wait(&status);

	if (pid == 0)
	{
		/* child */
		execlp("wc", "wc", "-l", filename, (char *)0);
	}
	else 
	{
		/* parent */
	}
}

void searchFile(char* filename, char* searchString)
{
	execlp("grep", "grep", "-r", filename, (char*)0);
}

void absolute_file_path() 
{
	char current_working_directory[1024];
	if (getcwd(current_working_directory, sizeof(current_working_directory)) == NULL) 
	{
		std::cout << "Error with getcwd()!" << std::endl;
	}
	char* file_name = "woo";
	char filename[1024] = "\0";
	strcat(filename, current_working_directory);
	strcat(filename, "/");
	strcat(filename, file_name);

}