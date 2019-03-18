/*
 * OurDefines.h
 *
 *      Author: Khalid AlAwadhi
 *      Spring 2019
 * 
 */

#ifndef OURDEFINES_H_
#define OURDEFINES_H_

#include <stdint.h>
#include <stdbool.h>


/***************************************
 *  This define is used to indicate    *
 *  wheather to print all logfiles to  *
 *  stdout or not                      *
 ***************************************/
#define DEBUG_PRINTF 1



/***************************************
 *        Thread Number Structure      *
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
	char Msg[100];
} MsgStruct;


/***************************************
 *      Pthread Argument Structure     *
 ***************************************/
typedef struct Pthread_ArgsStruct
{
    char LogFile_Path[100];			//Used to store the wanted logfile path
}Pthread_ArgsStruct;


#endif /* OURDEFINES_H_ */
