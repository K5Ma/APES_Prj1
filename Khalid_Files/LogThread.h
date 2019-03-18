/*
 * LogThread.h
 *
 *      Author: Khalid AlAwadhi
 *      Spring 2019
 */

#ifndef LOGTHREAD_H_
#define LOGTHREAD_H_

#include "OurDefines.h"


/**************************************************************************************************************
 * USAGE: This function contains all what the logging pthread will do. 
 *
 * PARAMETERS:
 *            - NONE
 *
 * RETURNS: NONE
 *
 **************************************************************************************************************/
void * LoggingThread(void * args);


/**************************************************************************************************************
 * USAGE: This function will be called initally when the Logging Thread is first initalized. 
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
 *        specify what source it came from and log it to a file. 
 *
 * PARAMETERS:
 *            - char* LogFilePath => Path to the log file
 *            - MsgStruct* Message => The message structure to decode and log
 *
 * RETURNS: NONE
 *
 **************************************************************************************************************/
void LogFile_Log(char* LogFilePath, MsgStruct* Message);


#endif /* LOGTHREAD_H_ */
