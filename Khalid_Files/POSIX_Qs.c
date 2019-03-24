#include <mqueue.h>
#include <stdio.h>
#include <string.h>

#include "POSIX_Qs.h"
#include "Global_Defines.h"


void SendToThreadQ(MsgStruct* Message)
{
	mqd_t MQ;						//Message queue descriptor
	
	char *DEST_Q_NAME;				//This will store the name of destination queue
	
	/* Check what is the destination pThread */
	switch(Message->Dest)
	{
		case Main:
			DEST_Q_NAME = MAIN_QUEUE;
			break;

		case Logging:
			DEST_Q_NAME = LOGGING_QUEUE;
			break;

		case Socket:
			DEST_Q_NAME = SOCKET_QUEUE;
			break;

		case Temp:
			DEST_Q_NAME = TEMP_QUEUE;
			break;

		case Lux:
			DEST_Q_NAME = LUX_QUEUE;
			break;

		default:
			Message->Dest = Logging;
			DEST_Q_NAME = LOGGING_QUEUE;
			char *text1 = "WARNING - No destination thread for this msg!";
			strcpy(Message->LogLevel, text1);
			break;
	}
	
	/* Open the chosen Thread POSIX queue - write only */
	MQ = mq_open(DEST_Q_NAME, O_WRONLY);
	
	/* Error check */
	if(MQ == (mqd_t) -1)
	{
		perror("!! ERROR in SendToThreadQ() => mq_open()");
		printf("!! Attempted to open '%u' queue (Check Global_Defines.h to know what queue maps to what number)\n\n", Message->Dest);
	}
	
	/* Send Msg to POSIX queue */
	if(mq_send(MQ, Message, sizeof(MsgStruct), 0) != 0)
	{
		perror("!! ERROR in SendToThreadQ() => mq_send()");
		printf("!! Attempted to send message '%s' from '%u' to '%u'\n\n", Message->Msg, Message->Source, Message->Dest);
	}
	
	if(mq_close(MQ) != 0)
	{
		perror("!! ERROR in SendToThreadQ() => mq_close()");
	}
}