/*
 * SocketThread.h
 *
 *      Author: Poorn Mehta
 *      Spring 2019
 */

#ifndef SOCKETTHREAD_H_
#define SOCKETTHREAD_H_

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
void SocketThread_Init();


#endif /* SOCKETTHREAD_H_ */
