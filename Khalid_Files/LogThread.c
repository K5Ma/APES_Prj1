#include "LogThread.h"
#include <stdio.h>
#include "OurDefines.h"
#include <sys/time.h>



void * LoggingThread(void * args)
{
	struct Pthread_ArgsStruct *Arguments = args;		//Get args

	LogFile_Init(Arguments->LogFile_Path);

}



void LogFile_Init(char* LogFilePath)
{
	/* File pointer */
	FILE *MyFileP;

	/* Declaring structure for time */
	struct timeval time;
	
	/* Modify the permissions of the file to be write and open the file (anything stored previously will be erased) */
	MyFileP = fopen(LogFilePath, "w");
	
	/* Get current time and save it */
	gettimeofday(&time, 0);

	/* NOTE:
	 * Statements are stored in strings because we want to have a debug output functionality. 
	 * So, if we change anything in fprintf() we will also need to go to the printf() and
	 * change the text there. Using string and storing our text there makes it easier
	 * as we only need to change the text in one place rather than two. */
	char* Line1 = "[%ld.%ld] Logging Thread: Logfile succesfully created!\n\n";
	char* Line2 = "***************************************\n";
	char* Line3 = "*     APES Project 1:                 *\n";
	char* Line4 = "*       *insert cool name here*       *\n";
	char* Line5 = "*                                     *\n";
	char* Line6 = "*  By: Khalid AlAwadhi | Poorn Mehta  *\n";
	char* Line7 = "*                              v1.0   *\n";
	char* Line8 = "***************************************\n\n";

	fprintf(MyFileP, Line1, time.tv_sec, time.tv_usec);
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
	printf(Line1, time.tv_sec, time.tv_usec);
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

	/* Declaring structure for time */
	struct timeval time;
	
	/* Modify the permissions of the file to append */
	MyFileP = fopen(LogFilePath, "a");
	
	/* Get current time and save it */
	gettimeofday(&time, 0);

	/* This string will store the text to output later on (just a temp variable) */
	char* text;

	switch(Message->Source)
	{
		case Main:
			text = "[%ld.%ld] Main Thread: %s\n";
			fprintf(MyFileP, text, time.tv_sec, time.tv_usec, Message->Msg);

			#ifdef DEBUG_PRINTF
			printf(text, time.tv_sec, time.tv_usec, Message->Msg);
			#endif
			break;

		case Logging:
			text = "[%ld.%ld] Logging Thread: %s\n";
			fprintf(MyFileP, text, time.tv_sec, time.tv_usec, Message->Msg);

			#ifdef DEBUG_PRINTF
			printf(text, time.tv_sec, time.tv_usec, Message->Msg);
			#endif
			break;

		case Socket:
			text = "[%ld.%ld] Socket Thread: %s\n";
			fprintf(MyFileP, text, time.tv_sec, time.tv_usec, Message->Msg);

			#ifdef DEBUG_PRINTF
			printf(text, time.tv_sec, time.tv_usec, Message->Msg);
			#endif
			break;

		case Temp:
			text = "[%ld.%ld] Temp Thread: %s\n";
			fprintf(MyFileP, text, time.tv_sec, time.tv_usec, Message->Msg);

			#ifdef DEBUG_PRINTF
			printf(text, time.tv_sec, time.tv_usec, Message->Msg);
			#endif
			break;

		case Lux:
			text = "[%ld.%ld] Lux Thread: %s\n";
			fprintf(MyFileP, text, time.tv_sec, time.tv_usec, Message->Msg);

			#ifdef DEBUG_PRINTF
			printf(text, time.tv_sec, time.tv_usec, Message->Msg);
			#endif
			break;

		default:
			text = "[%ld.%ld] ERROR - No thread source! Msg: %s\n";
			fprintf(MyFileP, text, time.tv_sec, time.tv_usec, Message->Msg);

			#ifdef DEBUG_PRINTF
			printf(text, time.tv_sec, time.tv_usec, Message->Msg);
			#endif
			break;
	}

	/* Flush file output */
	fflush(MyFileP);  

	/* Close the file */
	fclose(MyFileP);
}