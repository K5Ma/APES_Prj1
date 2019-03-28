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
pthread_t Log_pThread, Socket_pThread, Temp_pThread, Lux_pThread;

pthread_mutex_t lock;				//Used to lock and unlock critical sections in code
sig_atomic_t flag = 0;				//This will indicate whether any valid user signal has been received or not
uint8_t sig_sync = 0;				//Used to toggle in signal_function()
uint8_t Counter = 0;

int temp_file_des = -1;
int lux_file_des = -1;
uint8_t Temp_Error_Retry = 0;
uint8_t Lux_Error_Retry = 0;
uint8_t Temp_Sensor_State = Sensor_Offline;
uint8_t Lux_Sensor_State = Sensor_Offline;

struct sigaction custom_signal_action;
struct itimerval custom_timer;

void signal_function(int value)
{
	if(value == SIGVTALRM)
	{
		Counter += 1;
		if(Counter == Counter_Threshold)
		{
			Counter = 0;
			if(Temp_Error_Retry > Temp_No_Retry)
			{
				Temp_Error_Retry -= 1;
				if(close(temp_file_des))		Log_error(Main,"Closing the Temperature I2C File", errno, LOGGING_AND_LOCAL);
				SendToThreadQ(Main, Logging, "INFO", "\nTrying to get the Temperature Sensor Online... Calling TempThread_Init()\n");
				if(TempThread_Init())		Log_error(Main,"Attempt to get the Temperature Sensor Online Failed... Exiting TempThread_Init()", errno, LOGGING_AND_LOCAL);
				else
				{
					Temp_Error_Retry = Temp_No_Retry;
					Temp_Sensor_State = Sensor_Online;
					SendToThreadQ(Main, Logging, "INFO", "\nTemperature Sensor is Now Online...\n");
				}
			}

			if(Lux_Error_Retry > Lux_No_Retry)
			{
				Lux_Error_Retry -= 1;
				if(close(lux_file_des))		Log_error(Main,"Closing the Lux I2C File", errno, LOGGING_AND_LOCAL);
				SendToThreadQ(Main, Logging, "INFO", "\nTrying to get the Lux Sensor Online... Calling LuxThread_Init()\n");
				if(LuxThread_Init())		Log_error(Main,"Attempt to get the Lux Sensor Online Failed... Exiting LuxThread_Init()", errno, LOGGING_AND_LOCAL);
				else
				{
					Lux_Error_Retry = Lux_No_Retry;
					Lux_Sensor_State = Sensor_Online;
					SendToThreadQ(Main, Logging, "INFO", "\nLux Sensor is Now Online...\n");
				}
			}

		}
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
		custom_timer.it_interval.tv_usec = (Timer_Interval * 1000);

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
