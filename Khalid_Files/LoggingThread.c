#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <mqueue.h>

#include "LoggingThread.h"
#include "Global_Defines.h"
#include "My_Time.h"



void * LoggingThread(void * args)
{
	struct Pthread_ArgsStruct *Arguments = args;		//Get args

	LogFile_Init(Arguments->LogFile_Path);
	
	/* Create LogThread RX POSIX Q */
	mqd_t mq;											//Message queue descriptor
	//ssize_t Msg_Bytes;								//Stores the number of bytes recivied
	
	/* Initialize the queue attributes */
	struct mq_attr attr;
	attr.mq_flags = 0;									//message queue flags
	attr.mq_maxmsg = 10;								//maximum number of messages
	attr.mq_msgsize = MAX_SIZE_Q;						//maximum message size
	attr.mq_curmsgs = 0;								//number of messages currently queued
	
	/* Open the Thread1 queue to store pointers into it - Write only */
	//mq = mq_open(THREAD_1_QUEUE, O_CREAT | O_WRONLY | O_NONBLOCK, 0666, &attr);
	
}



void LogFile_Init(char* LogFilePath)
{
	/* File pointer */
	FILE *MyFileP;

	/* Modify the permissions of the file to be write and open the file (anything stored previously will be erased) */
	MyFileP = fopen(LogFilePath, "w");
	
	if(MyFileP == NULL)
	{
		printf("!!FATAL ERROR: Could not open log file: %s\n", LogFilePath);
		exit(1); 
	}
	
	/* NOTE:
	 * Statements are stored in strings because we want to have a debug output functionality. 
	 * So, if we change anything in fprintf() we will also need to go to the printf() and
	 * change the text there. Using string and storing our text there makes it easier
	 * as we only need to change the text in one place rather than two. */
	char* Line1 = "[%lf] Logging Thread: Logfile succesfully created!\n\n";
	char* Line2 = "***************************************\n";
	char* Line3 = "*     APES Project 1:                 *\n";
	char* Line4 = "*       *insert cool name here*       *\n";
	char* Line5 = "*                                     *\n";
	char* Line6 = "*  By: Khalid AlAwadhi | Poorn Mehta  *\n";
	char* Line7 = "*                              v1.1   *\n";
	char* Line8 = "***************************************\n\n";

	fprintf(MyFileP, Line1, GetCurrentTime());
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
	printf(Line1, GetCurrentTime());
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
		printf("!ERROR: Could not open log file: %s\n", LogFilePath);
		printf("	|--> Logging from source %u failed\n", Message->Source);
		printf("	|--> Log Level: %s failed\n", Message->LogLevel);
		printf("	L--> Message: %s\n\n", Message->Msg);
		return;
	}

	/* This string will store the text to output later on (just a temp variable) */
	char* text;

	switch(Message->Source)
	{
		case Main:
			text = "[%lf] Main Thread(%s): %s\n";
			fprintf(MyFileP, text, GetCurrentTime(), Message->LogLevel, Message->Msg);

			#ifdef DEBUG_PRINTF
			printf(text, GetCurrentTime(), Message->LogLevel, Message->Msg);
			#endif
			break;

		case Logging:
			text = "[%lf] Logging Thread(%s): %s\n";
			fprintf(MyFileP, text, GetCurrentTime(), Message->LogLevel, Message->Msg);

			#ifdef DEBUG_PRINTF
			printf(text, GetCurrentTime(), Message->LogLevel, Message->Msg);
			#endif
			break;

		case Socket:
			text = "[%lf] Socket Thread(%s): %s\n";
			fprintf(MyFileP, text, GetCurrentTime(), Message->LogLevel, Message->Msg);

			#ifdef DEBUG_PRINTF
			printf(text, GetCurrentTime(), Message->LogLevel, Message->Msg);
			#endif
			break;

		case Temp:
			text = "[%lf] Temp Thread(%s): %s\n";
			fprintf(MyFileP, text, GetCurrentTime(), Message->LogLevel, Message->Msg);

			#ifdef DEBUG_PRINTF
			printf(text, GetCurrentTime(), Message->LogLevel, Message->Msg);
			#endif
			break;

		case Lux:
			text = "[%lf] Lux Thread(%s): %s\n";
			fprintf(MyFileP, text, GetCurrentTime(), Message->LogLevel, Message->Msg);

			#ifdef DEBUG_PRINTF
			printf(text, GetCurrentTime(), Message->LogLevel, Message->Msg);
			#endif
			break;

		default:
			text = "[%lf] ERROR - No thread source! Msg(%s): %s\n";
			fprintf(MyFileP, text, GetCurrentTime(), Message->LogLevel, Message->Msg);

			#ifdef DEBUG_PRINTF
			printf(text, GetCurrentTime(), Message->LogLevel, Message->Msg);
			#endif
			break;
	}

	/* Flush file output */
	fflush(MyFileP);  

	/* Close the file */
	fclose(MyFileP);
}