/*
 * TempThread.h
 *
 *      Author: Poorn Mehta
 *      Spring 2019
 */

#ifndef TEMPTHREAD_H_
#define TEMPTHREAD_H_



/**************************************************************************************************************
 * USAGE: This function contains all what the temprature pthread will do. 
 *
 * PARAMETERS:
 *            - NONE
 *
 * RETURNS: NONE
 *
 **************************************************************************************************************/
void * TempThread(void * args);


/**************************************************************************************************************
 * USAGE: This function will be called initially when the Temp Thread is first initialized. 
 *
 * PARAMETERS:
 *            - NONE
 *
 * RETURNS: NONE
 *
 **************************************************************************************************************/
void TempThread_Init();


#endif /* TEMPTHREAD_H_ */