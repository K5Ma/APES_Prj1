/*
*	File: SocketThread.h
*	Purpose: The header file containing functionalities and thread of Socket
*	Owners: Poorn Mehta & Khalid AlAwadhi
*	Spring 2019
*/

#ifndef SOCKETTHREAD_H_
#define SOCKETTHREAD_H_

#include "Global_Defines.h"

#define Temp_Warning_Req			1
#define Lux_Warning_Req				2

/**************************************************************************************************************
 * USAGE: This function contains all what the socket pThread will do.
 *
 * PARAMETERS:
 *            - NONE
 *
 * RETURNS: NONE
 *
 **************************************************************************************************************/
void * SocketThread(void * args);



/**************************************************************************************************************
 * USAGE: This function will be called initially when the Socket Thread is first initialized.
 *
 * PARAMETERS:
 *            - NONE
 *
 * RETURNS: NONE
 *
 **************************************************************************************************************/
uint8_t SocketThread_Init(void);


uint8_t kill_socket_init(void);


#endif /* SOCKETTHREAD_H_ */
