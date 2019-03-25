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
 * USAGE: This function will output UNIX errors alongside a message to either send them to the logging thread, 
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


#endif /* POSIX_QS_H_ */