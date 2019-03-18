#include <stdio.h>
#include <pthread.h>
#include <string.h>

/* Our includes */
#include "OurDefines.h"
#include "LogThread.h"


/*########################################################################################
 #                                    TO-DO:                                             #
 #########################################################################################
 *****************************************************************************************
 * MAIN THREAD                                                                           *
 *****************************************************************************************
    1- [] CREATE START-UP CHECK FUNCTION 
	2- [] FIX BUG OF GETTING USER LOG FILE PATH
	3- [] 

 *****************************************************************************************
 * LOGGING THREAD                                                                        *
 *****************************************************************************************
    1- [COMPLETED - CREATED LogThread.c/.c] CREATE LOGGING PROTOTYPE
	2- [COMPLETED - FOUND IN OurDefines.h] CREATE MSG STRUCTURE
	3- [COMPLETED] TEST LOGGING
	4- [COMPELETD] CREATE LOGGING PTHREAD
	5- [] CREATE POSIX QUEUE FOR LOGGING THREAD
	6- [] TEST SENING MSG FROM pMAIN -> pLOGGING


 *****************************************************************************************
 * SOCKET THREAD                                                                         *
 *****************************************************************************************
    1- [] 


 *****************************************************************************************
 * TEMP THREAD                                                                           *
 *****************************************************************************************
    1- [] 


 *****************************************************************************************
 * LUX THREAD                                                                            *
 *****************************************************************************************
    1- [] 


#########################################################################################*/



int main(int argc, char *argv[])
{

	struct Pthread_ArgsStruct args;						//Create the pthread args structure
	
	char* User_LogFilePath;								//This will store the log file path location to pass to the Logging pthread
	
	/* Check if the user entered a logfile path */
	if(argc > 1)
	{
		printf("REACHED!!!!!!!!!");

		/* Get user wanted filepath */
		char* User_LogFTemp;

		strcpy(User_LogFTemp, (const char)argv[1]);

	//	sprintf(User_LogFilePath, "%s.txt", argv[1]);

		printf("Chosen log file path: %s\n", User_LogFTemp);
	}
	/* Else, use default logfile path */
	else
	{
		User_LogFilePath = "./LogFile.txt";
		printf("No logfile path chosen. Using default location './LogFile.txt'\n\n");
	}
	
	/* Store filepath to pass to pthreads */
//	strcpy(args.LogFile_Path, User_LogFilePath);


	/* Create the needed pthreads */
//	pthread_t Log_pThread, Socket_pThread, Temp_pThread, Lux_pThread;
	
	//pthread_create(&Log_pThread, NULL, &LoggingThread, (void *)&args);

	/* Wait for pthreads to finish */
//	pthread_join(Log_pThread, NULL);

}