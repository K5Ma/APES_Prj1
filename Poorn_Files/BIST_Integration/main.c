#include "main.h"

/* Our includes */
#include "Global_Defines.h"
#include "My_Time.h"
#include "POSIX_Qs.h"
#include "LoggingThread.h"
#include "SocketThread.h"
#include "TempThread.h"
#include "LuxThread.h"
#include "My_Time.c"
#include "POSIX_Qs.c"
#include "LoggingThread.c"
#include "SocketThread.c"
#include "TempThread.c"
#include "LuxThread.c"


/* Global Variables */
pthread_mutex_t lock;				//Used to lock and unlock critical sections in code
sig_atomic_t flag = 0;				//This will indicate whether any valid user signal has been received or not
uint8_t sig_sync = 0;				//Used to toggle in signal_function()

struct sigaction custom_signal_action;
struct itimerval custom_timer;

/*
 * LAST WORKING ON:
 *
 */

/*########################################################################################
 #                                    TO-DO:                                             #
 #########################################################################################
 *****************************************************************************************
 * GENERAL                                                                               *
 *****************************************************************************************
 * 1- [COMPLETED] REPLACE ALL PERRORS WITH strerror_r() TO BE ABLE TO LOG IT VIA THE
 * 				| LOGGING PTHREAD
 * 				L--> CREATED Log_error() FOUND IN POSIX_Qs.h
 *
 * 2- [COMPLETED] UPDATE SendToThreadQ() TO USE PARAMETERS INSTEAD OF STRUCT INPUT
 *
 * 3- []
 *
 *****************************************************************************************
 * MAIN THREAD                                                                           *
 *****************************************************************************************
 * 1- [COMPLETED] FIX BUG OF GETTING USER LOG FILE PATH
 *
 * 2- [] IMPLEMENT METHOD TO CHECK CHILD THREADS ARE ALIVE AT SOME INTERVAL
 *
 * 3- [] IMPLEMENT METHOD TO CLEANLY EXIT WHEN REQUESTED (MAKE CHILD EXITS PROPERLY
 * 		 THEN MAIN)
 *
 * 4- [] LOG ERROR INFORMATION AND INDICATE ERROR WITH BB USR LEDS (EG. MISSING SENSOR)
 *
 * 5- [COMPLETED] CREATE MY OWN TIME GET FUNCTION
 * 				L--> FOUND IN My_Time .h/.c
 *
 * 6- [] IMPLEMENT SIGNAL HANDLER
 *
 * 7- [COMPLETED] DISPLAY THREAD IDS AT START-UP
 *
 * 8- [] KILL ONLY SPECIFIED THREADS
 *
 * 9- []
 *
 *****************************************************************************************
 * LOGGING THREAD                                                                        *
 *****************************************************************************************
 * 1- [COMPLETED] CREATE LOGGING PROTOTYPE
 * 				L--> CREATED LoggingThread.h/.c
 *
 * 2- [COMPLETED] CREATE MSG STRUCTURE
 * 				L--> FOUND IN OurDefines.h
 *
 * 3- [COMPLETED] TEST LOGGING
 *
 * 4- [COMPLETED] CREATE LOGGING PTHREAD
 *
 * 5- [COMPLETED] CREATE POSIX QUEUE FOR LOGGING THREAD
 *
 * 6- [COMPLETED] TEST SENDING MSG FROM pMAIN -> pLOGGING
 *
 * 7- [COMPLETED] ADD FILE PATH ERROR HANDLING
 *
 * 8- [COMPLETED] LOG FILE PATH CONFIGURABLE AT RUN-TIME
 *
 * 9- [COMPLETED] START-UP LOGGING SUCCESS/FAIL REPORT
 *
 * 10- [COMPLETED] ADD LOG LEVEL TO STRUCT
 * 				L--> ADDED NEW ELEMENT TO OUR MSG STRUCT (STRING)
 *
 * 11- [COMPLETED] CREATE A MSG SEND TO QUEUE FUNCTION
 * 				L--> CREATED POSIX_Qs .h/.c
 *
 * 12- [COMPLETED] MAKE LOGGING THREAD BLOCK AND WAIT FOR ANY MESSAGES
 *
 * 13- [COMPLETED] HANDLE INITS OF OTHER THREAD TO LOGGING THREAD
 *
 * 14- [] IMPLEMENT SIGNAL KILL
 *
 * 15- []
 *
 *
 *****************************************************************************************
 * SOCKET THREAD                                                                         *
 *****************************************************************************************
 * 1- [POORN HANDLED IT] CREATE THE INTERNAL STRUCTURE OF THE SOCKET QUEUE
 *
 * 2- [COMPLETED] ADD POORN CODE
 *
 * 3- []
 *
 *
 *****************************************************************************************
 * TEMP THREAD                                                                           *
 *****************************************************************************************
 * 1- [COMPLETED] INIT THIS THREAD
 *
 * 2- [COMPLETED] ADD POORN CODE
 *
 * 3- []
 *
 *
 *****************************************************************************************
 * LUX THREAD                                                                            *
 *****************************************************************************************
 * 1- [COMPLETED] INIT THIS THREAD
 *
 * 2- [COMPLETED] ADD POORN CODE
 *
 * 3- []
 *
 *****************************************************************************************
 * UNSURE????                                                                            *
 *****************************************************************************************
A simple set of messages should be implemented. At a minimum, the set should support the
following:
 * Heartbeat notification from all threads to the main task: Either as a request-response
from Main task to individual tasks as a “ping”, or a periodic message from each task
to the Main task with Main monitoring (in some manner) the presence/absence of the report.

 * Startup tests Initialization Complete Notifications (Success/Failure) from each task

 * Error Message reporting

 * Sensor Tasks’ Data Requests (temperature/light)

 * Log Messages

 * Requests to close threads from main to other tasks

BIST:
Before you have your application begin steady state operation monitoring temperature,
light, and logging errors, tasks should have some tests run at startup to verify that
hardware and software is in working order. These tests should include:
 * Communication with the Temperature sensor that you can confirm that I2C works and that
the hardware is functioning

 * Communication with the Light sensor that you can confirm that I2C works and that the
hardware is functioning

 * Communication to the threads to make sure they have all started and up and running.

 * Log messages should be sent to the logger task when individual BIST tests have
finished along with an indicator if the hardware is connected and in working order.
If something does not startup correctly, an error code should be logged and the USR Led turned on.

#########################################################################################*/


void signal_function(int value)
{
	if(value == SIGVTALRM)
	{
		if(sig_sync == 0)
		{
			flag = Temperature_Signal;
			sig_sync = 1;
		}
		else
		{
			flag = Lux_Signal;
			sig_sync = 0;
		}
	}
	else
	{
		flag = value;
	}
}



void sig_setup(void)
{
		// Configuring timer and signal action

		// Set all initial values to 0 in the structure
		memset(&custom_signal_action, 0, sizeof (custom_signal_action));

		// Set signal action handler to point to the address of the target function (to execute on receiving signal)
		custom_signal_action.sa_handler = &signal_function;

		// Setting interval to 250ms
		custom_timer.it_interval.tv_sec = 0;
		custom_timer.it_interval.tv_usec = 250000;

		// Setting initial delay to 2s
		custom_timer.it_value.tv_sec = 1;
		custom_timer.it_value.tv_usec = 0;

		// Setting the signal action to kick in the handler function for these 3 signals
		sigaction (SIGVTALRM, &custom_signal_action, 0);
		sigaction (SIGUSR1, &custom_signal_action, 0);
		sigaction (SIGUSR2, &custom_signal_action, 0);
}




int main(int argc, char *argv[])
{
		struct Pthread_ArgsStruct args;						//Create the pthread args structure

		char User_LogFilePath[100];							//This will store the log file path location to pass to the Logging pthread

		printf("Starting...\n\n");

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


		/* Store filepath to pass to pThreads */
		strcpy(args.LogFile_Path, User_LogFilePath);


		if( pthread_mutex_init(&lock, NULL) != 0 )
		{
				Log_error(Main, "pthread_mutex_init()", errno, LOCAL_ONLY);
				return -1;
		}


		/* Setting up Signals */
		sig_setup();



		/* Create the needed pThreads */
		pthread_t Log_pThread, Socket_pThread, Temp_pThread, Lux_pThread;


		/* Create Logging pThread */
		if(pthread_create(&Log_pThread, NULL, &LoggingThread, (void *)&args) != 0)
		{
				Log_error(Main, "Logging pthread_create()", errno, LOCAL_ONLY);
		}
		else
		{
				printf("[%lf] Main pThread SUCCESS: Created Logging Thread!\n\n", GetCurrentTime());
		}

		/* Need to sleep a bit to make sure the Logging Thread starts up first */
		sleep(1);



		//BIST SHOULD BE HERE




		/* Create Socket pThread */
		if(pthread_create(&Socket_pThread, NULL, &SocketThread, NULL) != 0)
		{
				Log_error(Main, "Socket pthread_create()", errno, LOGGING_AND_LOCAL);
		}
		else
		{
				printf("[%lf] Main pThread SUCCESS: Created Socket Thread!\n\n", GetCurrentTime());
		}

		sleep(1);


		/* Create Temp pThread */
		if(pthread_create(&Temp_pThread, NULL, &TempThread, NULL) != 0)
		{
				Log_error(Main, "Temp pthread_create()", errno, LOGGING_AND_LOCAL);
		}
		else
		{
				printf("[%lf] Main pThread SUCCESS: Created Temp Thread!\n\n", GetCurrentTime());
		}

		sleep(1);


		/* Create Lux pThread */
		if(pthread_create(&Lux_pThread, NULL, &LuxThread, NULL) != 0)
		{
				Log_error(Main, "Lux pthread_create()", errno, LOGGING_AND_LOCAL);
		}
		else
		{
				printf("[%lf] Main pThread SUCCESS: Created Lux Thread!\n\n", GetCurrentTime());
		}

		sleep(1);

		// Starting timer
		setitimer (ITIMER_VIRTUAL, &custom_timer, 0);

		/* Wait for pThreads to finish */
		pthread_join(Log_pThread, NULL);
		pthread_join(Socket_pThread, NULL);
		pthread_join(Temp_pThread, NULL);
		pthread_join(Lux_pThread, NULL);
}
