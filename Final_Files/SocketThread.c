/*
*		File: SocketThread.c
*		Purpose: The source file containing functionalities and thread of Socket
*		Owners: Poorn Mehta & Khalid AlAwadhi
*		Spring 2019
*/

#include "SocketThread.h"
//#include "Global_Defines.h"
#include "POSIX_Qs.h"

uint8_t Temp_Warning;
uint8_t Lux_Warning;
uint8_t req_warning = 0;

uint8_t Socket_State;

// data structure
typedef struct
{
  char str[150];
  int num;
}info;

info info1, info2;
info *p1 = &info1;
info *p2 = &info2;

int new_socket, custom_socket, cust_sock, info_in, info_out;
char loglevel_sock[30], loglevel_q[30];

pthread_mutex_t lock;

sig_atomic_t flag;
uint8_t LogKillSafe;
uint8_t AliveThreads;


void * SocketThread(void * args)
{
	/* Init the Socket Thread */
	if(SocketThread_Init())
	{
		Log_error(Socket, "Socket Init Failed... Exiting Thread", ENOMSG, LOGGING_AND_LOCAL);
		return 0;
	}
	else		SendToThreadQ(Socket, Logging, "INFO", "Socket Init Succeeded");

	/* Create the Socket Thread POSIX queue */
	mqd_t MQ;											//Message queue descriptor

	/* Initialize the queue attributes */
	struct mq_attr attr;
	attr.mq_flags = 0;									/* Flags: 0 or O_NONBLOCK */
	attr.mq_maxmsg = 10;								/* Max. # of messages on queue */
	attr.mq_msgsize = sizeof(MsgStruct);				/* Max. message size (bytes) */
	attr.mq_curmsgs = 0;								/* # of messages currently in queue */

	/* Create the Socket Thread queue to get messages from other pThreads */
	MQ = mq_open(SOCKET_QUEUE, O_CREAT | O_RDONLY | O_CLOEXEC, 0666, &attr);
	if(MQ == (mqd_t) -1)
	{
		Log_error(Socket, "mq_open()", errno, LOGGING_AND_LOCAL);
	}

	MsgStruct MsgRecv;

	while(1)
	{
		// Wait for signal
		while((flag != SIGUSR1) && (flag != SIGUSR2))
		{
			/* Set alive bit */
			Socket_State = Socket_Online;
			pthread_mutex_lock(&lock_var);
			AliveThreads |= SOCKET_ALIVE;
			pthread_mutex_unlock(&lock_var);

			custom_socket = accept(new_socket, (struct sockaddr *)0, 0);

			if(custom_socket < 0)
			{
				Log_error(Socket, "Socket Accepting Failed accept()", errno, LOGGING_AND_LOCAL);
			}
			else
			{
				cust_sock = custom_socket;
			}

			   info_in = read(custom_socket,p2,sizeof(info));
            if(info_in < 0)
            {
                Log_error(Socket, "Socket Reading Failed read()", errno, LOGGING_AND_LOCAL);
            }

            char Socket_Text[150];
            sprintf(loglevel_sock, "INFO");

            char Socket_Text_q[60];
            sprintf(loglevel_q, "INFO");

            if( strcmp("Exit", p2->str) == 0 )
            {
                sprintf(Socket_Text, "Socket Thread is Exiting");
                SendToThreadQ(Socket, Logging, loglevel_sock, Socket_Text);
//*************************************************************************************************************************88
           //     SendToThreadQ(Socket, Logging, "INFO", "User Signal Passed - Killing Socket Thread");

                if(mq_unlink(SOCKET_QUEUE) != 0)
                {
                        Log_error(Socket, "mq_unlink()", errno, LOGGING_AND_LOCAL);
                }
                else
                {
                    SendToThreadQ(Socket, Logging, "INFO", "Successfully unlinked Socket queue!");
                }

//                char TempTxt[150];
//                if(flag == SIGUSR1)
//                {
//                    sprintf(TempTxt, "Exit Reason: User Signal 1 Received (%d)", flag);
//                    SendToThreadQ(Socket, Logging, "INFO", TempTxt);
//                }
//                else
//                {
//                    sprintf(TempTxt, "Exit Reason: User Signal 2 Received (%d)", flag);
//                    SendToThreadQ(Socket, Logging, "INFO", TempTxt);
//                }

				/* Decrement the LogKillSafe and clear the alive bit */
				pthread_mutex_lock(&lock_var);
				LogKillSafe--;
				AliveThreads &= ~SOCKET_ALIVE;
				pthread_mutex_unlock(&lock_var);

                SendToThreadQ(Socket, Logging, "INFO", "Socket Thread has terminated successfully and will now exit");


                return 0;
//*************************************************************************************************************************88
            }
						else
            {
                if(strcmp("Temperature", p2->str) == 0)
                {
										req_warning =	Temp_Warning_Req;
                    if(p2->num == Celsius)
                    {
                        sprintf(Socket_Text, "Client Requested Temperature in C");
                        sprintf(Socket_Text_q, "TC");
                    }
                    else if(p2->num == Fahrenheit)
                    {
                        sprintf(Socket_Text, "Client Requested Temperature in F");
                        sprintf(Socket_Text_q, "TF");
                    }
                    else if(p2->num == Kelvin)
                    {
                        sprintf(Socket_Text, "Client Requested Temperature in K");
                        sprintf(Socket_Text_q, "TK");
                    }
                    else
                    {
                        sprintf(Socket_Text, "Client Requested Temperature in Invalid Parameter - Sending in C");
                        sprintf(loglevel_sock, "WARNING");
                        sprintf(Socket_Text_q, "TC");
                    }

                    SendToThreadQ(Socket, Temp, loglevel_q, Socket_Text_q);
                }

                else if(strcmp("Lux", p2->str) == 0)
                {
										req_warning =	Lux_Warning_Req;
                    sprintf(Socket_Text, "Client Requested Lux");
                    sprintf(Socket_Text_q, "LX");
                    SendToThreadQ(Socket, Lux, loglevel_q, Socket_Text_q);
                }

                else
                {
                    sprintf(Socket_Text, "Invalid Client Request");
                    sprintf(loglevel_sock, "ERROR");
                }

                SendToThreadQ(Socket, Logging, loglevel_sock, Socket_Text);


                //SOCKET Q MUST WAIT FOR A RESPONSE, ELSE GIVE ERROR
                struct timespec tm;
                clock_gettime(CLOCK_REALTIME, &tm);
                tm.tv_sec += 2;

                int resp = mq_timedreceive(MQ, &MsgRecv, sizeof(MsgStruct), NULL, &tm);
                if(resp == -1)
                {
                    Log_error(Socket, "mq_timedreceive()", errno, LOGGING_AND_LOCAL);
                    p1->num = 0;
                }
                else if(resp < sizeof(MsgStruct))
                {
                    Log_error(Socket, "mq_timedreceive()", errno, LOGGING_AND_LOCAL);
                    p1->num = 0;
                }
                else if(resp == sizeof(MsgStruct))
                {
                    sprintf(Socket_Text, "Got Response from Queue: %s", MsgRecv.Msg);
                    strcpy(loglevel_sock, "INFO");
                    strcpy(p1->str, MsgRecv.Msg);
										if(req_warning == Temp_Warning_Req)		p1->num = Temp_Warning;
										else if(req_warning == Lux_Warning_Req)		p1->num = Lux_Warning;
										else		p1->num = 0;
                    SendToThreadQ(Socket, Logging, loglevel_sock, Socket_Text);
                }

                // Have to do this since custom_socket is getting corrupted
                custom_socket = cust_sock;
                info_out = write(custom_socket,p1,sizeof(info));
                if (info_out < 0)
                {
                    Log_error(Socket, "Socket Writing Failed write()", errno, LOGGING_AND_LOCAL);
                }
                else
                {
                    sprintf(Socket_Text, "Data sent Successfully to the Remote Client");
                    strcpy(loglevel_sock, "INFO");
                    SendToThreadQ(Socket, Logging, loglevel_sock, Socket_Text);
                }
            }
        }
    }
}




uint8_t SocketThread_Init(void)
{
		char Text[60];

		sprintf(Text, "Socket Thread successfully created! TID: %ld", syscall(SYS_gettid));

		SendToThreadQ(Socket, Logging, "INFO", Text);

		struct sockaddr_in custom_server;

		// socket init on server end
		new_socket = socket(AF_INET, SOCK_STREAM, 0);
		if(new_socket < 0)
		{
				Log_error(Socket, "Socket Creation Failed socket()", errno, LOGGING_AND_LOCAL);
				return 1;
		}

		custom_server.sin_family = AF_INET;
		custom_server.sin_addr.s_addr = INADDR_ANY;
		custom_server.sin_port = htons(PORT);

		if(bind(new_socket, (struct sockaddr *)&custom_server, sizeof(custom_server)) < 0)
		{
				Log_error(Socket, "Socket Binding Failed bind()", errno, LOGGING_AND_LOCAL);
				return 1;
		}

		if(listen(new_socket, 5) < 0)
		{
				Log_error(Socket, "Socket Listening Failed listen()", errno, LOGGING_AND_LOCAL);
				return 1;
		}

		return 0;
}

uint8_t kill_socket_init(void)
{

    // data structure
    typedef struct
    {
      char str[150];
      int num;
    }t_strct;

    t_strct t_strct1;
    t_strct *pt_strct1 = &t_strct1;

    strcpy(pt_strct1->str, "Exit");
    pt_strct1->num = 1;

    int temp_sock, t_out;
    struct sockaddr_in t_client;

    // socket init on t_client end
    temp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(temp_sock < 0)
    {
		Log_error(Main, "Socket Creation Failed", errno, LOGGING_AND_LOCAL);
        return 1;
    }

    t_client.sin_family = AF_INET;

    if(inet_pton(AF_INET, "127.0.0.1", &t_client.sin_addr)<=0)
    {
		    Log_error(Main, "Invalid/Unsupported Target IP Address", errno, LOGGING_AND_LOCAL);
		    return 1;
    }

    t_client.sin_port = htons(PORT);

    if(connect(temp_sock, (struct sockaddr *)&t_client, sizeof(t_client)) < 0)
    {
		Log_error(Main, "Socket Connection Failed", errno, LOGGING_AND_LOCAL);
        return 1;
    }

    t_out = write(temp_sock,pt_strct1,sizeof(t_strct));
    if (t_out < 0)
    {
		Log_error(Main, "Socket Writing Failed", errno, LOGGING_AND_LOCAL);
        return 1;
    }

    return 0;
}
