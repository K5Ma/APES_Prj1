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


extern pthread_mutex_t lock;
extern sig_atomic_t flag;


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

			// Check if there is a message from socket
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
		
		// In case of user signals, log and kill the Lux Thread thread
		else if(flag == SIGUSR1 || flag == SIGUSR2)
		{
			// Notifying user
			SendToThreadQ(Lux, Logging, "INFO", "User Signal Passed - Killing Lux Thread");

			if(mq_unlink(LUX_QUEUE) != 0)
			{
				Log_error(Lux, "mq_unlink()", errno, LOGGING_AND_LOCAL);
			}

			SendToThreadQ(Lux, Logging, "INFO", "--->>>Lux Thread Exited<<<---");
			char TempTxt[150];
			if(flag == SIGUSR1)
			{
				//printf("Exit Reason: User Signal 1 Received (%d)\n", flag);
				sprintf(TempTxt, "Exit Reason: User Signal 1 Received (%d)", flag);
				SendToThreadQ(Lux, Logging, "INFO", TempTxt);
			}
			else
			{
				//printf("Exit Reason: User Signal 2 Received (%d)\n", flag);
				sprintf(TempTxt, "Exit Reason: User Signal 2 Received (%d)", flag);
				SendToThreadQ(Lux, Logging, "INFO", TempTxt);
			}
	//		flag = 1;

			// Immediately terminate the thread (unlike pthread_cancel)
			pthread_exit(0);

			// Break the infinite loop
			break;

		}
	}
	
	printf("DEBUG: LUX PTHREAD HAS FINISHED AND WILL EXIT\n");
}



void LuxThread_Init()
{
	
	//INIT I2C SENSOR 
	
	char Text[60];
	
	sprintf(Text, "Lux Thread successfully created! TID: %ld", syscall(SYS_gettid));
	
	SendToThreadQ(Lux, Logging, "INFO", Text);
}