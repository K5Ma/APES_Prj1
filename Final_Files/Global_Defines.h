
/*
*	File: Global_Defines.h
*	Purpose: main header file of the APES Project 1
*	Owners: Poorn Mehta & Khalid AlAwadhi
*	Spring 2019
*/

#ifndef GLOBAL_DEFINES_H_
#define GLOBAL_DEFINES_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <assert.h>
#include <errno.h>

#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <malloc.h>

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include <float.h>
#include <complex.h>
#include <time.h>
#include <mqueue.h>



/***************************************
 *        Global Varibles            *
 ***************************************/
extern pthread_mutex_t lock, lock_var;

extern int temp_file_des;
extern int lux_file_des;
extern uint8_t Temp_Error_Retry;
extern uint8_t Lux_Error_Retry;
extern uint8_t Temp_Sensor_State;
extern uint8_t Lux_Sensor_State;
extern uint8_t Temp_Warning;
extern uint8_t Lux_Warning;

extern uint8_t Socket_State;

extern uint8_t Counter;

extern sig_atomic_t flag;
extern uint8_t LogKillSafe;
extern uint8_t AliveThreads;




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
 *  Alive Checking Defines:            *
 *  (Used in Main pThread)             *
 ***************************************/
#define LOGGING_ALIVE 				0b00000001
#define SOCKET_ALIVE 				0b00000010
#define TEMP_ALIVE 					0b00000100
#define LUX_ALIVE 					0b00001000


/***************************************
 *  LED Defines                        *
 ***************************************/
#define USR0_PORT					1
#define USR0_PIN					21

#define USR1_PORT					1
#define USR1_PIN					22

#define USR2_PORT					1
#define USR2_PIN					23

#define USR3_PORT					1
#define USR3_PIN					24

/***************************************
 *  Poorn Defines                      *
 ***************************************/
#define Temperature_Signal			0xF0
#define Lux_Signal					0xF1

#define Celsius						1
#define Fahrenheit					2
#define Kelvin						3

#define Sensor_Online		1
#define Sensor_Offline	0

#define PORT 						8080

#define time_high 					0x02  //for 402ms
#define time_med 					0x01  //for 101ms
#define time_low 					0x00  //for 13ms

#define gain 						0x10  //for maximum gain

#define Socket_Online		1
#define Socket_Offline	0

#define Timer_Interval		250		// In ms
#define Sensor_Retry_Period		5000		// In ms
#define Counter_Threshold		(Sensor_Retry_Period / Timer_Interval)

#define Alive_Testing_Interval		10		// In sec

//Path of I2C Bus
#define I2C_BUS		(char *)"/dev/i2c-2"

void signal_function(int value);
void sig_setup(void);


#endif /* GLOBAL_DEFINES_H_ */