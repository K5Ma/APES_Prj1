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
pthread_mutex_t lock;					//Used to lock and unlock critical sections in code
volatile sig_atomic_t flag = 0;			//This will indicate whether any valid user signal has been received or not
uint8_t sig_sync = 0;					//Used to toggle in signal_function()
volatile uint8_t LogKillSafe = 3;		//Used when other threads are killed. They decrement this value which assures Logging pThread is killed last.
volatile uint8_t AliveThreads = 0x00;	//Used in Main and checked bitwise to see which pThreads are alive

/*
 * LAST WORKING ON:
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
 * 2- [COMPLETED] IMPLEMENT METHOD TO CHECK CHILD THREADS ARE ALIVE AT SOME INTERVAL
 * 				L--> USED AliveThreads GLOBAL VARIABLE (CODE IN MAIN)
 *
 * 3- [COMPLETED] IMPLEMENT METHOD TO CLEANLY EXIT WHEN REQUESTED (MAKE CHILD EXITS PROPERLY
 * 		 THEN MAIN)
 *
 * 4- [] LOG ERROR INFORMATION AND INDICATE ERROR WITH BB USR LEDS (EG. MISSING SENSOR)
 *
 * 5- [COMPLETED] CREATE MY OWN TIME GET FUNCTION
 * 				L--> FOUND IN My_Time .h/.c
 *
 * 6- [COMPLETED] IMPLEMENT SIGNAL HANDLER
 *
 * 7- [COMPLETED] DISPLAY THREAD IDS AT START-UP
 *
 * 8- [SCRAPPED] KILL ONLY SPECIFIED THREADS
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
 * 14- []
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
 *
#########################################################################################*/

uint8_t kill_socket_init(void)
{

	// data structure
	typedef struct
	{
	  char str[150];
	  int num;
	}t_strct;

	t_strct t_strct1;
	t_strct *pt_strct1 = &info1;

	strcpy(pt_strct1->str, "Exit");
	pt_strct1->num = 1;

	int temp_sock, t_out;
	struct sockaddr_in t_client;

	// socket init on t_client end
	temp_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(temp_sock < 0)
	{
		printf("\nSocket Creation Failed\n");
		return 1;
	}

	t_client.sin_family = AF_INET;

	if(inet_pton(AF_INET, "127.0.0.1", &t_client.sin_addr)<=0)
	{
	printf("\nInvalid address/ Address not supported \n");
	return 1;
	}

	t_client.sin_port = htons(PORT);

	if(connect(temp_sock, (struct sockaddr *)&t_client, sizeof(t_client)) < 0)
	{
		printf("\nSocket Connection Failed\n");
		return 1;
	}

	t_out = write(temp_sock,pt_strct1,sizeof(t_strct));
	if (t_out < 0)
	{
		printf("\nSocket Writing Failed\n");
		return 1;
	}

	return 0;
}

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
		if(kill_socket_init())		printf("\nSocket killing failed\n");
		flag = value;
	}
}



void sig_setup(void)
{
	// Configuring timer and signal action
	struct sigaction custom_signal_action;
	struct itimerval custom_timer;

	// Set all initial values to 0 in the structure
	memset(&custom_signal_action, 0, sizeof (custom_signal_action));

	// Set signal action handler to point to the address of the target function (to execute on receiving signal)
	custom_signal_action.sa_handler = &signal_function;

	// Setting interval to 250ms
	custom_timer.it_interval.tv_sec = 0;
	custom_timer.it_interval.tv_usec = 250000;

	// Setting initial delay to 2s
	custom_timer.it_value.tv_sec = 2;
	custom_timer.it_value.tv_usec = 0;

	// Setting the signal action to kick in the handler function for these 3 signals
	sigaction (SIGVTALRM, &custom_signal_action, 0);
	sigaction (SIGUSR1, &custom_signal_action, 0);
	sigaction (SIGUSR2, &custom_signal_action, 0);

	// Starting timer
	setitimer (ITIMER_VIRTUAL, &custom_timer, 0);
}




int main(int argc, char *argv[])
{
	struct Pthread_ArgsStruct args;						//Create the pthread args structure

	char User_LogFilePath[100];							//This will store the log file path location to pass to the Logging pthread

	printf("Starting... PID: %d\n\n", getpid());

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
	sleep(2);



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

	/* Create Temp pThread */
	if(pthread_create(&Temp_pThread, NULL, &TempThread, NULL) != 0)
	{
		Log_error(Main, "Temp pthread_create()", errno, LOGGING_AND_LOCAL);
	}
	else
	{
		printf("[%lf] Main pThread SUCCESS: Created Temp Thread!\n\n", GetCurrentTime());
	}


	/* Create Lux pThread */
	if(pthread_create(&Lux_pThread, NULL, &LuxThread, NULL) != 0)
	{
		Log_error(Main, "Lux pthread_create()", errno, LOGGING_AND_LOCAL);
	}
	else
	{
		printf("[%lf] Main pThread SUCCESS: Created Lux Thread!\n\n", GetCurrentTime());
	}

	/* Let other pThreads execute before checking */
	sleep(2);

	/* While there is at least one thread alive: */
	while( AliveThreads != 0 )
	{
		/* Create a copy of the global variable, AliveThreads, and store it in the local variable CurrentAlive.
		 * This is done as to not halt the other pThreads trying to use AliveThreads. It is faster to update
		 * local variable and do work rather than halt all threads until the alive check is done. */
		pthread_mutex_lock(&lock);
		uint8_t CurrentAlive = AliveThreads;		//Create a copy of the global variable
		AliveThreads = 0;							//Reset alive bits
		pthread_mutex_unlock(&lock);


		/* Check Logging pThread */
		if(CurrentAlive & LOGGING_ALIVE)
		{
			SendToThreadQ(Main, Logging, "INFO", "Logging pThread is alive");
		}
		else
		{
			printf("[%lf] Main pThread(ERROR): Logging pThread is not alive\n\n", GetCurrentTime());
		}

		/* Check Socket pThread */
		if(CurrentAlive & SOCKET_ALIVE)
		{
			SendToThreadQ(Main, Logging, "INFO", "Socket pThread is alive");
		}
		else
		{
			//SendToThreadQ(Main, Logging, "ERROR", "Socket pThread is not alive");
			printf("[%lf] Main pThread(ERROR): Socket pThread is not alive\n\n", GetCurrentTime());
		}

		/* Check Temp pThread */
		if(CurrentAlive & TEMP_ALIVE)
		{
			SendToThreadQ(Main, Logging, "INFO", "Temp pThread is alive");
		}
		else
		{
			SendToThreadQ(Main, Logging, "ERROR", "Temp pThread is not alive");
			printf("[%lf] Main pThread(ERROR): Temp pThread is not alive\n\n", GetCurrentTime());
		}


		/* Check Lux pThread */
		if(CurrentAlive & LUX_ALIVE)
		{
			SendToThreadQ(Main, Logging, "INFO", "Lux pThread is alive");
		}
		else
		{
			SendToThreadQ(Main, Logging, "ERROR", "Lux pThread is not alive");
			printf("[%lf] Main pThread(ERROR): Lux pThread is not alive\n\n", GetCurrentTime());
		}

		/* Check again after 10 secs */
		sleep(10);
	}

	printf("DEBUG: ALL MY THREADS ARE DEAD :(!\n\n");

//
//	/* Wait for pThreads to finish */
//	pthread_join(Log_pThread, NULL);
//	pthread_join(Socket_pThread, NULL);
//	pthread_join(Temp_pThread, NULL);
//	pthread_join(Lux_pThread, NULL);

}

//	/* While there is at least one thread alive: */
//	while( AliveThreads != 0 )
//	{
//		/* Check if Logging Thread is alive */
//		if(AliveThreads & LOGGING_ALIVE)
//		{
//			SendToThreadQ(Main, Logging, "INFO", "Are you alive?");
//
//			/* Sleep for a bit */
//			sleep(4);
//
//			/* Check for a response:
//			 * If a response is not received, that means the thread is not alive */
//			if(mq_receive(MQ, &MsgRecv, sizeof(MsgStruct), NULL) == -1)
//			{
//				Log_error(Main, "Alive check 'Logging': mq_receive()", errno, LOCAL_ONLY);
//				AliveThreads &= ~LOGGING_ALIVE;						//Since the pThread is not alive, stop checking for it
//				printf("[%lf] Main pThread(ERROR): Logging pThread is not alive\n\n", GetCurrentTime());
//			}
//			else
//			{
//				/* Check the response, if its correct, log it */
//				Main_AliveCheck_Resp(Logging, &MsgRecv);
//			}
//		}
//
//
//		/* Check if Socket Thread is alive */
//		if(AliveThreads & SOCKET_ALIVE)
//		{
//			SendToThreadQ(Main, Socket, "INFO", "Are you alive?");
//
//			/* Sleep for a bit */
//			sleep(1);
//
//			/* Check for a response:
//			 * If a response is not received, that means the thread is not alive */
//			if(mq_receive(MQ, &MsgRecv, sizeof(MsgStruct), NULL) == -1)
//			{
//				Log_error(Main, "Alive check 'Socket': mq_receive()", errno, LOGGING_AND_LOCAL);
//				AliveThreads &= ~SOCKET_ALIVE;						//Since the pThread is not alive, stop checking for it
//				printf("[%lf] Main pThread(ERROR): Socket pThread is not alive\n\n", GetCurrentTime());
//			}
//			else
//			{
//				/* Check the response, if its correct, log it */
//				Main_AliveCheck_Resp(Socket, &MsgRecv);
//			}
//		}
//
//		/* Check if Temp Thread is alive */
//		if(AliveThreads & TEMP_ALIVE)
//		{
//			SendToThreadQ(Main, Temp, "INFO", "Are you alive?");
//
//			/* Sleep for a bit */
//			sleep(1);
//
//			/* Check for a response:
//			 * If a response is not received, that means the thread is not alive */
//			if(mq_receive(MQ, &MsgRecv, sizeof(MsgStruct), NULL) == -1)
//			{
//				Log_error(Main, "Alive check 'Temp': mq_receive()", errno, LOGGING_AND_LOCAL);
//				AliveThreads &= ~TEMP_ALIVE;						//Since the pThread is not alive, stop checking for it
//				printf("[%lf] Main pThread(ERROR): Temp pThread is not alive\n\n", GetCurrentTime());
//			}
//			else
//			{
//				/* Check the response, if its correct, log it */
//				Main_AliveCheck_Resp(Temp, &MsgRecv);
//			}
//		}
//
//		/* Check if Lux Thread is alive */
//		if(AliveThreads & LUX_ALIVE)
//		{
//			SendToThreadQ(Main, Lux, "INFO", "Are you alive?");
//
//			/* Sleep for a bit */
//			sleep(1);
//
//			/* Check for a response:
//			 * If a response is not received, that means the thread is not alive */
//			if(mq_receive(MQ, &MsgRecv, sizeof(MsgStruct), NULL) == -1)
//			{
//				Log_error(Main, "Alive check 'Lux': mq_receive()", errno, LOGGING_AND_LOCAL);
//				AliveThreads &= ~LUX_ALIVE;						//Since the pThread is not alive, stop checking for it
//				printf("[%lf] Main pThread(ERROR): Lux pThread is not alive\n\n", GetCurrentTime());
//			}
//			else
//			{
//				/* Check the response, if its correct, log it */
//				Main_AliveCheck_Resp(Lux, &MsgRecv);
//			}
//		}
//
//		/* Sleep for 5 secs as we don't need to check continuously */
//		printf("DEBUG: DONE CHECKING, GOING TO SLEEP\n\n");
//		sleep(5);
//	}
