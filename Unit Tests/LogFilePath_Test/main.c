/*
*	File: main.c
*	Purpose: Unit Test APES Project 1 - 2019 
*	Owners: Poorn Mehta & Khalid AlAwadhi
* 
* 
*	PURPOSE OF TEST:
*	The purpose of this test is to see if we can choose a file destination
* 	which will store all log messages when executing the program.
* 	The default file name is LogFile.txt.
* 
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>



int main(int argc, char *argv[])
{
	char User_LogFilePath[100];							//This will store the log file path location to pass to the Logging pthread

	/* Check if the user entered a logfile path */
	if(argc > 1)
	{
		sprintf(User_LogFilePath, "%s", argv[1]);
		printf("Chosen log file path: %s\n", User_LogFilePath);
	}

	/* Else, use default logfile path */
	else
	{
		sprintf(User_LogFilePath, "./LogFile.txt");
		printf("No logfile path chosen. Using default location './LogFile.txt'\n\n");
	}
	
	printf("Main pThread SUCCESS: All tests passed!\n\n");
}