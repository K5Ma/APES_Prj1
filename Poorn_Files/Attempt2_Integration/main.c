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

// This will indicate whether any valid user signal has been received or not
sig_atomic_t flag = 0;

pthread_mutex_t lock;

uint8_t sig_sync = 0;

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
		else	flag = value;
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

	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("\n mutex init has failed\n");
		return -1;
	}

	/* Store filepath to pass to pThreads */
	strcpy(args.LogFile_Path, User_LogFilePath);

	//Setting up Signals
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
		printf("[%lf] SUCCESS: Created Logging Thread!\n\n", GetCurrentTime());
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
		printf("[%lf] SUCCESS: Created Socket Thread!\n\n", GetCurrentTime());
	}


	/* Create Temp pThread */
	if(pthread_create(&Temp_pThread, NULL, &TempThread, NULL) != 0)
	{
		Log_error(Main, "Temp pthread_create()", errno, LOGGING_AND_LOCAL);
	}
	else
	{
		printf("[%lf] SUCCESS: Created Temp Thread!\n\n", GetCurrentTime());
	}


	/* Create Lux pThread */
	if(pthread_create(&Lux_pThread, NULL, &LuxThread, NULL) != 0)
	{
		Log_error(Main, "Lux pthread_create()", errno, LOGGING_AND_LOCAL);
	}
	else
	{
		printf("[%lf] SUCCESS: Created Lux Thread!\n\n", GetCurrentTime());
	}


	/* Wait for pThreads to finish */
	pthread_join(Log_pThread, NULL);
	pthread_join(Socket_pThread, NULL);
	pthread_join(Temp_pThread, NULL);
	pthread_join(Lux_pThread, NULL);

}


//	/* DEBUG: TESTING SENDING A MSG FROM MAIN EVERY 5 SECS */
//	uint8_t Num = 0;
//
//	while(1)
//	{
//		sleep(5);
//
//		char test_text[60];
//
//		sprintf(test_text, "This is a test from main number: %u", Num);
//
//		MsgStruct TempMsg =
//		{
//			.Source = Main,
//			.Dest = Logging,
//			.LogLevel = "INFO",
//		};
//
//		strcpy(TempMsg.Msg, test_text);
//
//		SendToThreadQ(&TempMsg);
//
//		Num++;
//	}
