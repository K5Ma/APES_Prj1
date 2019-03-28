#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <mqueue.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include "TempThread.h"
#include "Global_Defines.h"
#include "POSIX_Qs.h"

extern pthread_mutex_t lock;
extern sig_atomic_t flag;
extern uint8_t LogKillSafe;
extern uint8_t AliveThreads;

void * TempThread(void * args)
{
	/* Init the Temp Thread */
	TempThread_Init();
	
	/* Create the Temp Thread POSIX queue */
	mqd_t MQ;											//Message queue descriptor

	/* Initialize the queue attributes */
	struct mq_attr attr;
	attr.mq_flags = O_NONBLOCK;							/* Flags: 0 or O_NONBLOCK */
	attr.mq_maxmsg = 10;								/* Max. # of messages on queue */
	attr.mq_msgsize = sizeof(MsgStruct);				/* Max. message size (bytes) */
	attr.mq_curmsgs = 0;								/* # of messages currently in queue */

	
	/* Create the Temp Thread queue to get messages from other pThreads */
	MQ = mq_open(TEMP_QUEUE, O_CREAT | O_RDONLY | O_NONBLOCK | O_CLOEXEC, 0666, &attr);
	if(MQ == (mqd_t) -1)
	{
		Log_error(Temp, "mq_open()", errno, LOGGING_AND_LOCAL);
	}
	
	// Init Para
	uint8_t Temperature_Unit = Celsius;

	// Reception Structure from Socket
	MsgStruct MsgRecv;
	
	while(1)
	{
		/* Set alive bit */
		pthread_mutex_lock(&lock);
		AliveThreads |= TEMP_ALIVE;
		pthread_mutex_unlock(&lock);
			
		// Wait for signal
		while((flag == 0) || (flag == Lux_Signal));
		
		if(flag == Temperature_Signal)
		{
			flag = 0;
			pthread_mutex_lock(&lock);
			float Temperature_C = 0; //ADD FUNCTION TO GET TEMP FROM I2C SENSOR
			pthread_mutex_unlock(&lock);

			float Temperature_F = (Temperature_C * 1.8) + 32;
			float Temperature_K = Temperature_C + 273.15;

			char Temperature_Text[150];

			if(Temperature_Unit == Celsius)
			{
				sprintf(Temperature_Text, "Temperature is *%f* in C", Temperature_C);
			}
			else if(Temperature_Unit == Fahrenheit)
			{
				sprintf(Temperature_Text, "Temperature is *%f* in F", Temperature_F);
			}	
			else if(Temperature_Unit == Kelvin)
			{
				sprintf(Temperature_Text, "Temperature is *%f* in K", Temperature_K);
			}
			
			SendToThreadQ(Temp, Logging, "INFO", Temperature_Text);

			// Check if there is a message from socket
			int resp = mq_receive(MQ, &MsgRecv, sizeof(MsgStruct), NULL);
			if(resp != -1)
			{
				if(resp == sizeof(MsgStruct))
				{
					if(strcmp("TC",MsgRecv.Msg) == 0)
					{
						Temperature_Unit = Celsius;
						sprintf(Temperature_Text, "Temperature is *%f* in C", Temperature_C);
					}
					else if(strcmp("TF",MsgRecv.Msg) == 0)
					{
						Temperature_Unit = Fahrenheit;
						sprintf(Temperature_Text, "Temperature is *%f* in F", Temperature_F);
					}
					else if(strcmp("TK",MsgRecv.Msg) == 0)
					{
						Temperature_Unit = Kelvin;
						sprintf(Temperature_Text, "Temperature is *%f* in K", Temperature_K);
					}

					SendToThreadQ(Temp, Socket, "INFO", Temperature_Text);
				}
			}
			else
			{
				sprintf(Temperature_Text, ">>>>>>>>> Expected:%d Got:%d <<<<<<<<<<<", sizeof(MsgStruct), resp);
				SendToThreadQ(Temp, Logging, "ERROR", Temperature_Text);
			}
		}
		
		
		/* Check for KILL signals */
		else if(flag == SIGUSR1 || flag == SIGUSR2)
		{
			SendToThreadQ(Temp, Logging, "INFO", "User Signal Passed - Killing Temperature Thread");

			if(mq_unlink(TEMP_QUEUE) != 0)
			{
				Log_error(Temp, "mq_unlink()", errno, LOGGING_AND_LOCAL);
			}
			else
			{
				SendToThreadQ(Temp, Logging, "INFO", "Successfully unlinked Temp queue!");
			}

			char TempTxt[150];
			if(flag == SIGUSR1)
			{
				sprintf(TempTxt, "Exit Reason: User Signal 1 Received (%d)", flag);
				SendToThreadQ(Temp, Logging, "INFO", TempTxt);
			}
			else
			{
				sprintf(TempTxt, "Exit Reason: User Signal 2 Received (%d)", flag);
				SendToThreadQ(Temp, Logging, "INFO", TempTxt);
			}
			
			/* Decrement the LogKillSafe Global Variable */
			pthread_mutex_lock(&lock);
			LogKillSafe--; 
			pthread_mutex_unlock(&lock);
			
			SendToThreadQ(Temp, Logging, "INFO", "Temp Thread has terminated successfully and will now exit");
			
			return 0;
		}
	}
}



void TempThread_Init()
{
	//INIT I2C SENSOR 
	
	char Text[60];
	
	sprintf(Text, "Temp Thread successfully created! TID: %ld", syscall(SYS_gettid));
		
	SendToThreadQ(Temp, Logging, "INFO", Text);
}