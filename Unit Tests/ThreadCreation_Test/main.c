/*
*	File: main.c
*	Purpose: Unit Test APES Project 1 - 2019 
*	Owners: Poorn Mehta & Khalid AlAwadhi
* 
* 
*	PURPOSE OF TEST:
*	The purpose of this test is to see if we can create pThreads
*	successfully and then join them before exiting the Main pThread.
* 
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

void * LoggingThread(void * args)
{
	
	return NULL; 
}

void * SocketThread(void * args)
{
	
	return NULL; 
}

void * TempThread(void * args)
{
	
	return NULL; 
}

void * LuxThread(void * args)
{
	
	return NULL; 
}



int main(int argc, char *argv[])
{
	/* Create the needed pThreads */
	pthread_t Log_pThread, Socket_pThread, Temp_pThread, Lux_pThread;


	/* Create Logging pThread */
	if(pthread_create(&Log_pThread, NULL, &LoggingThread, NULL) != 0)
	{
		printf("Main pThread ERROR: Could not create Logging Thread!\n\n");
	}
	else
	{
		printf("Main pThread SUCCESS: Created Logging Thread!\n\n");
	}
	
	/* Create Socket pThread */
	if(pthread_create(&Socket_pThread, NULL, &SocketThread, NULL) != 0)
	{
		printf("Main pThread ERROR: Could not create Socket Thread!\n\n");
	}
	else
	{
		printf("Main pThread SUCCESS: Created Socket Thread!\n\n");
	}

	/* Create Temp pThread */
	if(pthread_create(&Temp_pThread, NULL, &TempThread, NULL) != 0)
	{
		printf("Main pThread ERROR: Could not create Temp Thread!\n\n");
	}
	else
	{
		printf("Main pThread SUCCESS: Created Temp Thread!\n\n");
	}
	
	/* Create Lux pThread */
	if(pthread_create(&Lux_pThread, NULL, &LuxThread, NULL) != 0)
	{
		printf("Main pThread ERROR: Could not create Lux Thread!\n\n");
	}
	else
	{
		printf("Main pThread SUCCESS: Created Lux Thread!\n\n");
	}
	
	/* Wait for pThreads to finish */
	pthread_join(Log_pThread, NULL);
	pthread_join(Socket_pThread, NULL);
	pthread_join(Temp_pThread, NULL);
	pthread_join(Lux_pThread, NULL);
	
	printf("Main pThread SUCCESS: All tests passed!\n\n");
}