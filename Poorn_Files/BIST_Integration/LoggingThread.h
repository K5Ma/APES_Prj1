/*
 * LoggingThread.h
 *
 *      Author: Khalid AlAwadhi
 *      Spring 2019
 */
#ifndef LOGGINGTHREAD_H_
#define LOGGINGTHREAD_H_

#include "Global_Defines.h"


/**************************************************************************************************************
 * USAGE: This function contains all what the Logging pThread will do. 
 *
 * PARAMETERS:
 *            - NONE
 *
 * RETURNS: NONE
 *
 **************************************************************************************************************/
void * LoggingThread(void * args);



/**************************************************************************************************************
 * USAGE: This function will be called initially when the Logging Thread is first initialized. 
 *
 * PARAMETERS:
 *            - char* LogFilePath => Path to the log file
 *
 * RETURNS: NONE
 *
 **************************************************************************************************************/
void LogFile_Init(char* LogFilePath);



/**************************************************************************************************************
 * USAGE: This function will log messages received by the Logging Thread. It will decode the message and
 *        specify the destination and source it came from and log it to a file.  
 *
 * PARAMETERS:
 *            - char* LogFilePath => Path to the log file
 *            - MsgStruct* Message => The message structure to decode and log
 *
 * RETURNS: NONE
 *
 **************************************************************************************************************/
void LogFile_Log(char* LogFilePath, MsgStruct* Message);




#endif /* LOGGINGTHREAD_H_ */