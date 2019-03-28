#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <mqueue.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include "LuxThread.h"
#include "Global_Defines.h"
#include "POSIX_Qs.h"


pthread_mutex_t lock;
volatile sig_atomic_t flag;
volatile uint8_t LogKillSafe;
volatile uint8_t AliveThreads;

void * LuxThread(void * args)
{
	/* Init the Lux Thread */
	LuxThread_Init();

	/* Create the Lux Thread POSIX queue */
	mqd_t MQ;											//Message queue descriptor

	/* Initialize the queue attributes */
	struct mq_attr attr;
	attr.mq_flags = O_NONBLOCK;							/* Flags: 0 or O_NONBLOCK */
	attr.mq_maxmsg = 10;								/* Max. # of messages on queue */
	attr.mq_msgsize = sizeof(MsgStruct);				/* Max. message size (bytes) */
	attr.mq_curmsgs = 0;								/* # of messages currently in queue */

	/* Create the Lux Thread queue to get messages from other pThreads */
	MQ = mq_open(LUX_QUEUE, O_CREAT | O_RDONLY | O_NONBLOCK | O_CLOEXEC, 0666, &attr);
	if(MQ == (mqd_t) -1)
	{
		Log_error(Lux, "mq_open()", errno, LOGGING_AND_LOCAL);
	}


	// Reception Structure from Socket
	MsgStruct MsgRecv;


	while(1)
	{
		/* Set alive bit */
		pthread_mutex_lock(&lock);
		AliveThreads |= LUX_ALIVE;
		pthread_mutex_unlock(&lock);


		// Wait for signal
		while((flag == 0) || (flag == Temperature_Signal));

		// If timer interrupt has passed signal, log cpu usage
		if(flag == Lux_Signal)
		{
			flag = 0;
			pthread_mutex_lock(&lock);
			float Lux_Value = 0; 		//ADD I2C CODE TO GET LUX
			pthread_mutex_unlock(&lock);

			char Lux_Text[60];

			sprintf(Lux_Text, "Lux is *%f*", Lux_Value);

			SendToThreadQ(Lux, Logging, "INFO", Lux_Text);

			// Check if there is a message from Main or Socket
			int resp = mq_receive(MQ, &MsgRecv, sizeof(MsgStruct), NULL);
			if(resp != -1)
			{
				if(resp == sizeof(MsgStruct))
				{
					if(strcmp("LX",MsgRecv.Msg) == 0)
					{
						sprintf(Lux_Text, "Lux is *%f*", Lux_Value);
					}
					SendToThreadQ(Lux, Socket, "INFO", Lux_Text);
				}
				else
				{
					sprintf(Lux_Text, ">>>>>>>>>Expected:%d Got:%d<<<<<<<<<<<", sizeof(MsgStruct), resp);
					SendToThreadQ(Lux, Logging, "ERROR", Lux_Text);
				}
			}
		}

		/* Check for KILL signals */
		else if(flag == SIGUSR1 || flag == SIGUSR2)
		{
			SendToThreadQ(Lux, Logging, "INFO", "User Signal Passed - Killing Lux Thread");

			if(mq_unlink(LUX_QUEUE) != 0)
			{
				Log_error(Lux, "mq_unlink()", errno, LOGGING_AND_LOCAL);
			}
			else
			{
				SendToThreadQ(Lux, Logging, "INFO", "Successfully unlinked Lux queue!");
			}

			char TempTxt[150];
			if(flag == SIGUSR1)
			{
				sprintf(TempTxt, "Exit Reason: User Signal 1 Received (%d)", flag);
				SendToThreadQ(Lux, Logging, "INFO", TempTxt);
			}
			else
			{
				sprintf(TempTxt, "Exit Reason: User Signal 2 Received (%d)", flag);
				SendToThreadQ(Lux, Logging, "INFO", TempTxt);
			}

			/* Decrement the LogKillSafe Global Variable */
			pthread_mutex_lock(&lock);
			LogKillSafe--;
			pthread_mutex_unlock(&lock);

			SendToThreadQ(Lux, Logging, "INFO", "Lux Thread has terminated successfully and will now exit");

			return 0;
		}
	}
}



void LuxThread_Init()
{

	//INIT I2C SENSOR

	char Text[60];

	sprintf(Text, "Lux Thread successfully created! TID: %ld", syscall(SYS_gettid));

	SendToThreadQ(Lux, Logging, "INFO", Text);
}
