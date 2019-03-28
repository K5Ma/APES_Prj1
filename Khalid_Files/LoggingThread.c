#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include "LoggingThread.h"
#include "Global_Defines.h"
#include "My_Time.h"
#include "POSIX_Qs.h"


extern sig_atomic_t flag;
extern uint8_t LogKillSafe;
extern pthread_mutex_t lock;
extern uint8_t AliveThreads;


void * LoggingThread(void * args)
{
	/* Get the passed arguments */
	struct Pthread_ArgsStruct *Arguments = args;
	
	/* Init the log file */
	LogFile_Init(Arguments->LogFile_Path);
	
	
	/* Create the Logging Thread POSIX queue */
	mqd_t MQ;											//Message queue descriptor

	/* Initialize the queue attributes */
	struct mq_attr attr;
	attr.mq_flags = 0;									/* Flags: 0 or O_NONBLOCK */
	attr.mq_maxmsg = 10;								/* Max. # of messages on queue */
	attr.mq_msgsize = sizeof(MsgStruct);				/* Max. message size (bytes) */
	attr.mq_curmsgs = 0;								/* # of messages currently in queue */
	
	/* Create the Logging Thread queue to get messages from other pThreads */
	MQ = mq_open(LOGGING_QUEUE, O_CREAT | O_RDONLY | O_CLOEXEC, 0666, &attr);
	if(MQ == (mqd_t) -1)
	{
		Log_error(Logging, "mq_open()", errno, LOCAL_ONLY);
	}
		
	MsgStruct MsgRecv;									//Temp variable used to store received messages
	
	/* Loop forever waiting for Msgs from other pThreads while at least one is alive */
	while(LogKillSafe > 0)
	{
		/* Block until a msg is received */
		if(mq_receive(MQ, &MsgRecv, sizeof(MsgStruct), NULL) == -1)
		{
			Log_error(Logging, "mq_receive()", errno, LOCAL_ONLY);
		}
		/* If a msg is received, log it */
		else
		{
			LogFile_Log(Arguments->LogFile_Path, &MsgRecv);
			
			/* Set alive bit */
			pthread_mutex_lock(&lock);
			AliveThreads |= LOGGING_ALIVE;
			pthread_mutex_unlock(&lock);
		}
	}
	
	/* If we reach this point, it means a KILL signal was passed (USR1 or USR2) and */
	/* the other pThreads terminated successfully, we must now kill the Logging Thread */
	printf("[%lf] Logging pThread(INFO): No other threads are alive - Killing Logging Thread\n\n", GetCurrentTime());

	if(mq_unlink(LOGGING_QUEUE) != 0)
	{
		Log_error(Logging, "mq_unlink()", errno, LOCAL_ONLY);
	}
	else
	{
		printf("[%lf] Logging pThread: Successfully unlinked Logging queue!\n\n", GetCurrentTime());
	}
	
	printf("[%lf] Logging Thread: Logging Thread has terminated successfully and will now exit\n\n", GetCurrentTime());

	return 0;
}



void LogFile_Init(char* LogFilePath)
{
	/* File pointer */
	FILE *MyFileP;

	/* Modify the permissions of the file to be write and open the file (anything stored previously will be erased) */
	MyFileP = fopen(LogFilePath, "w");
	
	if(MyFileP == NULL)
	{
		printf("!!! FATAL ERROR: Could not open log file: %s\n", LogFilePath);
		exit(1); 
	}
	
	/* NOTE:
	 * Statements are stored in strings because we want to have a debug output functionality. 
	 * So, if we change anything in fprintf() we will also need to go to the printf() and
	 * change the text t#include <sys/syscall.h>here. Using string and storing our text there makes it easier
	 * as we only need to change the text in one place rather than two. */
	char* Line1 = "[%lf] Logging Thread: Logfile successfully created! TID: %ld\n\n";
	char* Line2 = "***************************************\n";
	char* Line3 = "*     APES Project 1:                 *\n";
	char* Line4 = "*       *insert cool name here*       *\n";
	char* Line5 = "*                                     *\n";
	char* Line6 = "*  By: Khalid AlAwadhi | Poorn Mehta  *\n";
	char* Line7 = "*                              v1.5   *\n";
	char* Line8 = "***************************************\n\n";

	fprintf(MyFileP, Line1, GetCurrentTime(), syscall(SYS_gettid));
	fprintf(MyFileP, Line2);
	fprintf(MyFileP, Line3);
	fprintf(MyFileP, Line4);
	fprintf(MyFileP, Line5);
	fprintf(MyFileP, Line6);
	fprintf(MyFileP, Line7);
	fprintf(MyFileP, Line8);
	
	/* Flush file output */
	fflush(MyFileP);  

	/* Close the file */
	fclose(MyFileP);


#ifdef DEBUG_PRINTF
	printf(Line1, GetCurrentTime(), syscall(SYS_gettid));
	printf(Line2);
	printf(Line3);
	printf(Line4);
	printf(Line5);
	printf(Line6);
	printf(Line7);
	printf(Line8);
#endif
	
}



void LogFile_Log(char* LogFilePath, MsgStruct* Message)
{
	/* File pointer */
	FILE *MyFileP;
	
	/* Modify the permissions of the file to append */
	MyFileP = fopen(LogFilePath, "a");
	
	if(MyFileP == NULL)
	{
		printf("!! ERROR: Could not open log file: %s\n", LogFilePath);
		printf("	|--> Logging from source '%u' failed\n", Message->Source);
		printf("	|--> Destination: %u\n", Message->Dest);
		printf("	|--> Log Level: %s\n", Message->LogLevel);
		printf("	L--> Message: %s\n\n", Message->Msg);
		return;
	}
	
	/* Get the source number and turn it into a string. 
	 * This is done for readability in the logging file */
	char* Source_text; 
	switch(Message->Source)
	{
		case Main:
			Source_text = "Main Thread";
			break;

		case Logging:
			Source_text = "Logging Thread";
			break;

		case Socket:
			Source_text = "Socket Thread";
			break;

		case Temp:
			Source_text = "Temp Thread";
			break;

		case Lux:
			Source_text = "Lux Thread";
			break;

		default:
			Source_text = "Unknown Thread";
			break;
	}
	

	/* This string will store the text to output later on (just a temp variable) */
	char* text;

	switch(Message->Dest)
	{
		case Main:
			text = "[%lf] Main Thread(%s): %s\n		L-> Source: '%s'\n\n";
			fprintf(MyFileP, text, GetCurrentTime(), Message->LogLevel, Message->Msg, Source_text);

			#ifdef DEBUG_PRINTF
			printf(text, GetCurrentTime(), Message->LogLevel, Message->Msg, Source_text);
			#endif
			break;

		case Logging:
			text = "[%lf] Logging Thread(%s): %s\n		L-> Source: '%s'\n\n";
			fprintf(MyFileP, text, GetCurrentTime(), Message->LogLevel, Message->Msg, Source_text);

			#ifdef DEBUG_PRINTF
			printf(text, GetCurrentTime(), Message->LogLevel, Message->Msg, Source_text);
			#endif
			break;

		case Socket:
			text = "[%lf] Socket Thread(%s): %s\n		L-> Source: '%s'\n\n";
			fprintf(MyFileP, text, GetCurrentTime(), Message->LogLevel, Message->Msg, Source_text);

			#ifdef DEBUG_PRINTF
			printf(text, GetCurrentTime(), Message->LogLevel, Message->Msg, Source_text);
			#endif
			break;

		case Temp:
			text = "[%lf] Temp Thread(%s): %s\n		L-> Source: '%s'\n\n";
			fprintf(MyFileP, text, GetCurrentTime(), Message->LogLevel, Message->Msg, Source_text);

			#ifdef DEBUG_PRINTF
			printf(text, GetCurrentTime(), Message->LogLevel, Message->Msg, Source_text);
			#endif
			break;

		case Lux:
			text = "[%lf] Lux Thread(%s): %s\n		L-> Source: '%s'\n\n";
			fprintf(MyFileP, text, GetCurrentTime(), Message->LogLevel, Message->Msg, Source_text);

			#ifdef DEBUG_PRINTF
			printf(text, GetCurrentTime(), Message->LogLevel, Message->Msg, Source_text);
			#endif
			break;

		default:
			text = "[%lf] Unknown Thread '%u'(%s): Msg(%s): %s\n		L-> Source: '%s'\n\n";
			fprintf(MyFileP, text, GetCurrentTime(), Message->LogLevel, Message->Msg, Source_text);

			#ifdef DEBUG_PRINTF
			printf(text, GetCurrentTime(), Message->LogLevel, Message->Msg, Source_text);
			#endif
			break;
	}

	/* Flush file output */
	fflush(MyFileP);  

	/* Close the file */
	fclose(MyFileP);
}