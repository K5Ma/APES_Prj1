/*
 * OurDefines.h
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
 *  wheather to print all logfiles to  *
 *  stdout or not                      *
 ***************************************/
#define DEBUG_PRINTF 1



/***************************************
 *        Thread Source Enum           *
 ***************************************/
typedef enum
{
	Main = 0,
	Logging = 1,
	Socket = 2,
	Temp = 3,
	Lux = 4
} Sources;


/***************************************
 *        Message Structure            *
 ***************************************/
typedef struct MsgStruct 
{
	uint8_t Source;
	char LogLevel[10];				//Expcected values: INFO | WARNING | ERROR | CRITICAL
	char Msg[100];
} MsgStruct;


/***************************************
 *      Pthread Argument Structure     *
 ***************************************/
typedef struct Pthread_ArgsStruct
{
    char LogFile_Path[100];			//Used to store the wanted logfile path
}Pthread_ArgsStruct;



/***************************************
 *          POSIX Qeueus               *
 ***************************************/
#define MAX_SIZE_Q				1024
#define MAIN_QUEUE				"/MAIN_POSIX_Q"
#define LOGGING_QUEUE			"/LOGGING_POSIX_Q"


#endif /* GLOBAL_DEFINES_H_ */