/*
 * LuxThread.h
 *
 *      Author: Khalid AlAwadhi
 *      Spring 2019
 */

#ifndef LUXTHREAD_H_
#define LUXTHREAD_H_



/**************************************************************************************************************
 * USAGE: This function contains all what the lux pthread will do. 
 *
 * PARAMETERS:
 *            - NONE
 *
 * RETURNS: NONE
 *
 **************************************************************************************************************/
void * LuxThread(void * args);


/**************************************************************************************************************
 * USAGE: This function will be called initially when the Lux Thread is first initialized. 
 *
 * PARAMETERS:
 *            - NONE
 *
 * RETURNS: NONE
 *
 **************************************************************************************************************/
void LuxThread_Init();


#endif /* LUXTHREAD_H_ */