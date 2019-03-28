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
 *  whether to print all logfiles to   *
 *  stdout or not                      *
 ***************************************/
#define DEBUG_PRINTF 				1



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
#define SOCKET_QUEUE				"/SOCKET_POSIX_Q"
#define TEMP_QUEUE					"/TEMP_POSIX_Q"
#define LUX_QUEUE					"/LUX_POSIX_Q"



/***************************************
 *  Log_error() Function Parameters:   *
 *        (found in POSIX_Qs.h)        *
 ***************************************/
#define LOGGING_AND_LOCAL			0x01
#define LOGGING_ONLY				0x02
#define LOCAL_ONLY					0x03 
 
 
/***************************************
 *  Alive Checking Defines:
 *  (Used in Main pThread)
 ***************************************/
#define LOGGING_ALIVE 				0b00000001
#define SOCKET_ALIVE 				0b00000010
#define TEMP_ALIVE 					0b00000100
#define LUX_ALIVE 					0b00001000


/***************************************
 *  Poorn Defines                      *
 ***************************************/
#define Temperature_Signal			0xF0
#define Lux_Signal					0xF1

#define Celsius						1
#define Fahrenheit					2
#define Kelvin						3

#define PORT 						8080

#define time_high 					0x02  //for 402ms
#define time_med 					0x01  //for 101ms
#define time_low 					0x00  //for 13ms

#define gain 						0x10  //for maximum gain



#endif /* GLOBAL_DEFINES_H_ */