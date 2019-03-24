/*
 * Global_Defines.h
 *
 *      Author: Khalid AlAwadhi
 *      Spring 2019
 * 
 */

#ifndef GLOBAL_DEFINES_H_
#define GLOBAL_DEFINES_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>


/***************************************
 *  This define is used to indicate    *
 *  whether to print all logfiles to  *
 *  stdout or not                      *
 ***************************************/
#define DEBUG_PRINTF 1



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
#define MAIN_QUEUE				"/MAIN_POSIX_Q"
#define LOGGING_QUEUE			"/LOGGING_POSIX_Q"
#define SOCKET_QUEUE			"/SOCKET_POSIX_Q"
#define TEMP_QUEUE				"/TEMP_POSIX_Q"
#define LUX_QUEUE				"/LUX_POSIX_Q"


#endif /* GLOBAL_DEFINES_H_ */