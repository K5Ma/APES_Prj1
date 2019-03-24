/*
 * POSIX_Qs.h
 *
 *      Author: Khalid AlAwadhi
 *      Spring 2019
 */
#ifndef POSIX_QS_H_
#define POSIX_QS_H_

#include "Global_Defines.h"


/**************************************************************************************************************
 * USAGE: This function will send a Msg struct to a chosen pThread based on the parameters of our Msg struct.
 * 		  In addition, it has error handling. 
 *
 * PARAMETERS:
 *            - MsgStruct* Message => This should contain all the information needed (Source, Destination,
 *                                    Log level and the message)
 *
 * RETURNS: NONE
 *
 **************************************************************************************************************/
void SendToThreadQ(MsgStruct* Message);


#endif /* POSIX_QS_H_ */