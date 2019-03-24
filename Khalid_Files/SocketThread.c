#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <mqueue.h>
#include <time.h>

#include "SocketThread.h"
#include "Global_Defines.h"
#include "POSIX_Qs.h"


void * SocketThread(void * args)
{
	/* Init the Socket Thread */
	SocketThread_Init();
	
	/* Create the Socket Thread POSIX queue */
	mqd_t MQ;											//Message queue descriptor

	/* Initialize the queue attributes */
	struct mq_attr attr;
	attr.mq_flags = 0;									/* Flags: 0 or O_NONBLOCK */
	attr.mq_maxmsg = 10;								/* Max. # of messages on queue */
	attr.mq_msgsize = sizeof(MsgStruct);				/* Max. message size (bytes) */
	attr.mq_curmsgs = 0;								/* # of messages currently in queue */
	
	/* Create the Socket Thread queue to get messages from other pThreads */
	MQ = mq_open(SOCKET_QUEUE, O_CREAT | O_RDWR , 0664, &attr);
	if(MQ == (mqd_t) -1)
	{
		perror("!! ERROR in SocketThread Thread => mq_open()");
	}
	
	
	//SOCKET SHOULD WAIT UNTIL CMD IS GOT FROM CLIENT
	
	//ONCE CMD IS GOT, STORE IT IN LOCAL STRUCT
	MsgStruct MsgRecv;
	
	//SEND CMD TO WANTED Q	
//	SendToThreadQ(&MsgRecv);
	
	//SOCKET Q MUST WAIT FOR A RESPONSE, ELSE GIVE ERROR
	struct timespec tm;
	clock_gettime(CLOCK_REALTIME, &tm);
	tm.tv_sec += 1;
	
	if( 0 > mq_timedreceive(MQ, &MsgRecv, sizeof(MsgStruct), NULL, &tm) )
	{
		
	}
	else
	{
		perror("!! ERROR in Socket Thread => mq_receive()");
	}
	
	//WAIT FOR NEXT CLIENT CMD
	

	if(mq_unlink(SOCKET_QUEUE) != 0)
	{
		perror("!! ERROR in Socket Thread => mq_unlink()");
	}
	
	printf("DEBUG: SOCKET PTHREAD HAS FINISHED AND WILL EXIT\n");
}




void SocketThread_Init()
{
	char Text[60];
	
	sprintf(Text, "Socket Thread successfully created! TID: %ld", syscall(SYS_gettid));

	MsgStruct InitMsg =
	{
		.Source = Socket,
		.Dest = Logging,
		.LogLevel = "INFO",
	};
		
	strcpy(InitMsg.Msg, Text);
		
	SendToThreadQ(&InitMsg);
}