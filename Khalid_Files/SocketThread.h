/*
 * SocketThread.h
 *
 *      Author: Poorn Mehta
 *      Spring 2019
 */

#ifndef SOCKETTHREAD_H_
#define SOCKETTHREAD_H_



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


uint8_t kill_socket_init(void);


#endif /* SOCKETTHREAD_H_ */