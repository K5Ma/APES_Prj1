#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <mqueue.h>
#include <time.h>

#include "LuxThread.h"
#include "Global_Defines.h"
#include "POSIX_Qs.h"


void * LuxThread(void * args)
{
	/* Init the Lux Thread */
	LuxThread_Init();
	
	/* Create the Lux Thread POSIX queue */
	mqd_t MQ;											//Message queue descriptor

	/* Initialize the queue attributes */
	struct mq_attr attr;
	attr.mq_flags = 0;									/* Flags: 0 or O_NONBLOCK */
	attr.mq_maxmsg = 10;								/* Max. # of messages on queue */
	attr.mq_msgsize = sizeof(MsgStruct);				/* Max. message size (bytes) */
	attr.mq_curmsgs = 0;								/* # of messages currently in queue */
	
	/* Create the Lux Thread queue to get messages from other pThreads */
	MQ = mq_open(LUX_QUEUE, O_CREAT | O_RDWR , 0664, &attr);
	if(MQ == (mqd_t) -1)
	{
		perror("!! ERROR in LuxThread => mq_open()");
	}
	
	while(1)
	{
		
	}
	
	if(mq_unlink(LUX_QUEUE) != 0)
	{
		perror("!! ERROR in LuxThread => mq_unlink()");
	}
	
	printf("DEBUG: LUX PTHREAD HAS FINISHED AND WILL EXIT\n");
}



void LuxThread_Init()
{
	char Text[60];
	
	sprintf(Text, "Lux Thread successfully created! TID: %ld", syscall(SYS_gettid));
	
	SendToThreadQ(Lux, Logging, "INFO", Text);
}