/*
*	File: POSIX_Qs.h
*	Purpose: This header file has functions that handles IPC between pThreads
*	Owners: Poorn Mehta & Khalid AlAwadhi
*	Spring 2019
*/
#ifndef POSIX_QS_H_
#define POSIX_QS_H_

#include "Global_Defines.h"


/**************************************************************************************************************
 * USAGE: This function will send a message to a chosen pThread based on the parameters. In addition, it has
 *        error handling. 
 *
 * PARAMETERS:
 *            - uint8_t Src => Source of the message (Look at Global_Defines.h Source Enums)
 *            - uint8_t Dst => Destination of the message
 *            - char* Log => The log level message
 *            - char* Message => mesaage to send
 *
 * RETURNS: NONE
 *
 **************************************************************************************************************/
void SendToThreadQ(uint8_t Src, uint8_t Dst, char* Log, char* Message);



/**************************************************************************************************************
 * USAGE: This function will output UNIX errors alongside a message to either: send them to the logging thread, 
 *        or just output to stdout, or both. This was created using a thread-safe error retrieval function.
 *
 * PARAMETERS:
 *            - uint8_t Src => Source of the message (Look at Global_Defines.h Source Enums)
 *            - char* Err_Msg => The message to display before the UNIX error
 *            - int errnum => must be the 'errno' varible (need to include #include <errno.h>)
 *            - uint8_t Mode => The below modes are already defined in Global_Defines.h
 *                              0x01 = LOGGING_AND_LOCAL:   In this mode, the error message will be displayed to 
 *                                                          the user in addition to sending the message to the
 *                                                          Logging pThread.
 * 
 *                              0x02 = LOGGING_ONLY:        In this mode, the message will only be sent to the
 *                                                          Logging pThread.
 * 
 *                              0x03 = LOCAL_ONLY(Default): In this mode, the error message will only be 
 *                                                          displayed to user via stdout(printf).
 * 
 * RETURNS: NONE
 *
 **************************************************************************************************************/
void Log_error(uint8_t Src, char* Err_Msg, int errnum, uint8_t Mode);



/**************************************************************************************************************
 * [Deprecated]
 * 
 * USAGE: This function is used in Pthreads to check if an alive check message was sent from Main pThread.
 *
 * PARAMETERS:
 *            - uint8_t Chosen_Dest => The pThread Main sent a message to
 *            - MsgStruct* Msg2Check => The message received by the pThread to check. 
 *
 * RETURNS: TRUE => Message was an alive check from Main
 *          FALSE => Message was NOT an alive check from Main
 *
 **************************************************************************************************************/
bool Main_AliveCheck(uint8_t Chosen_Dest, MsgStruct* Msg2Check);



/**************************************************************************************************************
 * [Deprecated]
 * 
 * USAGE: This function is used in Main pThread to check if a response received is the correct response or not. 
 *
 * PARAMETERS:
 *            - uint8_t Chosen_Dest => The pThread Main sent a message to
 *            - MsgStruct* Msg2Check => The message received by the pThread to check. 
 *
 * RETURNS: TRUE => Chosen pThread got the message and is alive
 *          FALSE => Chosen pThread did not respond to the Main alive check message 
 *
 **************************************************************************************************************/
bool Main_AliveCheck_Resp(uint8_t Chosen_Dest, MsgStruct* Msg2Check);


#endif /* POSIX_QS_H_ */