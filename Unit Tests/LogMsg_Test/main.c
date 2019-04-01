/*
*	File: main.c
*	Purpose: Unit Test APES Project 1 - 2019 
*	Owners: Poorn Mehta & Khalid AlAwadhi
* 
* 
*	PURPOSE OF TEST:
*	The purpose of this test is to send a message from Main to the Logging
* 	thread and have it be logged to a file. This will test our IPC and structure.
* 	As well as our error handling.
* 
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <mqueue.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/syscall.h>

/***************************************
 *  Thread Numbering Enum:             *
 *  Used for source and destination    *   
 ***************************************/
typedef enum
{
	Main = 1,
	Logging = 2,
	Socket = 3,
	Temp = 4,
	Lux = 5
} Sources;


/***************************************
 *        Message Structure            *
 ***************************************/
typedef struct MsgStruct 
{
	uint8_t Source;
	uint8_t Dest;
	char LogLevel[150];				//Expected values: INFO | WARNING | ERROR | CRITICAL
	char Msg[150];
} MsgStruct;


/***************************************
 *      pThread Argument Structure     *
 ***************************************/
typedef struct Pthread_ArgsStruct
{
    char LogFile_Path[100];			//Used to store the wanted logfile path
}Pthread_ArgsStruct;


/***************************************
 *          POSIX Queues               *
 ***************************************/
#define MAIN_QUEUE					"/MAIN_POSIX_Q"
#define LOGGING_QUEUE				"/LOGGING_POSIX_Q"

/***************************************
 *  Log_error() Function Parameters:   *
 ***************************************/
#define LOGGING_AND_LOCAL			0x01
#define LOGGING_ONLY				0x02
#define LOCAL_ONLY					0x03 



double GetCurrentTime()
{
	/* Declaring structure for time */
	struct timeval time;
	
	/* Get current time and save it */
	gettimeofday(&time, 0);
	
	/* Combine the Secs with uSecs by typecasting
     * Long Int to Double and return as 1 Double value */
	return (double)(time.tv_sec)+(((double)(time.tv_usec))/1000000);
}

void SendToThreadQ(uint8_t Src, uint8_t Dst, char* Log, char* Message);

void Log_error(uint8_t Src, char* Err_Msg, int errnum, uint8_t SendToLogging)
{
	char Error_Log[150];										//Stores the complete error message
	char ErrMsg_strerror[150];									//Stores the strerror_r error message
	
	strerror_r(errnum, ErrMsg_strerror, 150);					//Get error via a thread-safe function
	
	sprintf(Error_Log, "%s: %s", Err_Msg, ErrMsg_strerror);		//Combine user message with the strerror
	
	
	/* Get name of source */
	char* Source_text; 
	switch(Src)
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
			
			
	/* Output error depending on chosen mode */
	switch(SendToLogging)
	{
		case LOGGING_ONLY:
			SendToThreadQ(Src, Logging, "ERROR", Error_Log);
			break;

		case LOGGING_AND_LOCAL:
			printf("[%lf] Error in Thread '%s' => %s\n\n", GetCurrentTime(), Source_text, Error_Log);
			SendToThreadQ(Src, Logging, "ERROR", Error_Log);
			break;
			
		default:
			printf("[%lf] Error in Thread '%s' => %s\n\n", GetCurrentTime(), Source_text, Error_Log);
			break;
	}
}


void SendToThreadQ(uint8_t Src, uint8_t Dst, char* Log, char* Message)
{
	MsgStruct Msg2Send =
	{
		.Source = Src,
		.Dest = Dst
	};
	strcpy(Msg2Send.LogLevel, Log);
	strcpy(Msg2Send.Msg, Message);
	
	mqd_t MQ;						//Message queue descriptor
	
	char *DEST_Q_NAME;				//This will store the name of destination queue
	
	/* Check what is the destination pThread */
	switch(Msg2Send.Dest)
	{
		case Main:
			DEST_Q_NAME = MAIN_QUEUE;
			break;

		case Logging:
			DEST_Q_NAME = LOGGING_QUEUE;
			break;

		default:
			Msg2Send.Dest = Logging;
			DEST_Q_NAME = LOGGING_QUEUE;
			char *text1 = "WARNING - No destination thread for this msg!";
			strcpy(Msg2Send.LogLevel, text1);
			break;
	}
	
	/* Open the chosen Thread POSIX queue - write only */
	MQ = mq_open(DEST_Q_NAME, O_WRONLY | O_CLOEXEC);
	
	char ErrMsg[250];								//Temp variable
	
	/* Error check */
	if(MQ == (mqd_t) -1)
	{
		sprintf(ErrMsg, "SendToThreadQ() => mq_open(), attempted to open '%u' queue, called by Thread '%u'", Msg2Send.Dest, Msg2Send.Source);
		Log_error(0, ErrMsg, errno, LOCAL_ONLY);
	}
	
	/* Send Msg to POSIX queue */
	if(mq_send(MQ, &Msg2Send, sizeof(MsgStruct), 0) != 0)
	{
		sprintf(ErrMsg, "SendToThreadQ() => mq_send(), attempted to send message '%s' from '%u' to '%u'", Msg2Send.Msg, Msg2Send.Source, Msg2Send.Dest);
		Log_error(0, ErrMsg, errno, LOCAL_ONLY);
	}
	
	if(mq_close(MQ) != 0)
	{
		sprintf(ErrMsg, "SendToThreadQ() => mq_close(), attempted to close '%u' queue", Msg2Send.Source);
		Log_error(0, ErrMsg, errno, LOCAL_ONLY);
	}
}




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
	
	uint8_t i = 0; 
	
	/* Loop forever waiting for Msgs from other pThreads while at least one is alive */
	while(i == 0)
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
			i++;
		}
	}

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
	char* Line7 = "*               UNIT TEST             *\n";
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





int main(int argc, char *argv[])
{
	struct Pthread_ArgsStruct args;						//Create the pthread args structure

	char User_LogFilePath[100];							//This will store the log file path location to pass to the Logging pthread

	printf("Starting... PID: %d\n\n", getpid());

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
	
	/* Create the needed pThreads */
	pthread_t Log_pThread;


	/* Create Logging pThread */
	if(pthread_create(&Log_pThread, NULL, &LoggingThread, (void *)&args) != 0)
	{
		printf("Main pThread ERROR: Could not create Logging Thread!\n\n");
	}
	else
	{
		printf("Main pThread SUCCESS: Created Logging Thread!\n\n");
	}

	SendToThreadQ(Main, Logging, "INFO", "This is a unit test. If you can read this in the txt file, it works!");
	
	/* Wait for pThreads to finish */
	pthread_join(Log_pThread, NULL);
	
	
	printf("Main pThread SUCCESS: All tests passed!\n\n");
}










//
// MAIN PTHREAD CREATE 
//int main(int argc, char *argv[])
//{
//	/* Create the needed pThreads */
//	pthread_t Log_pThread, Socket_pThread, Temp_pThread, Lux_pThread;
//
//
//	/* Create Logging pThread */
//	if(pthread_create(&Log_pThread, NULL, &LoggingThread, NULL) != 0)
//	{
//		printf("Main pThread ERROR: Could not create Logging Thread!\n\n");
//	}
//	else
//	{
//		printf("Main pThread SUCCESS: Created Logging Thread!\n\n");
//	}
//	
//	/* Create Socket pThread */
//	if(pthread_create(&Socket_pThread, NULL, &SocketThread, NULL) != 0)
//	{
//		printf("Main pThread ERROR: Could not create Socket Thread!\n\n");
//	}
//	else
//	{
//		printf("Main pThread SUCCESS: Created Socket Thread!\n\n");
//	}
//
//	/* Create Temp pThread */
//	if(pthread_create(&Temp_pThread, NULL, &TempThread, NULL) != 0)
//	{
//		printf("Main pThread ERROR: Could not create Temp Thread!\n\n");
//	}
//	else
//	{
//		printf("Main pThread SUCCESS: Created Temp Thread!\n\n");
//	}
//	
//	/* Create Lux pThread */
//	if(pthread_create(&Lux_pThread, NULL, &LuxThread, NULL) != 0)
//	{
//		printf("Main pThread ERROR: Could not create Lux Thread!\n\n");
//	}
//	else
//	{
//		printf("Main pThread SUCCESS: Created Lux Thread!\n\n");
//	}
//	
//	/* Wait for pThreads to finish */
//	pthread_join(Log_pThread, NULL);
//	pthread_join(Socket_pThread, NULL);
//	pthread_join(Temp_pThread, NULL);
//	pthread_join(Lux_pThread, NULL);
//	
//	printf("Main pThread SUCCESS: All tests passed!\n\n");
//}