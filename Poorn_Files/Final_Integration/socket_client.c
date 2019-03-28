#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <malloc.h>
#include <arpa/inet.h>

#define PORT 8080

#define Temp_Normal					0x00
#define Temp_Warning_High		0x01
#define Temp_Warning_Low		0x02

#define Lux_State_Night			0x01
#define Lux_State_Day				0x02

#define Temp_Warning_Req			1
#define Lux_Warning_Req				2

uint8_t req_warning = 0;

// for time
struct timespec timespec_struct;
struct timeval current_time;

// flag for sigterm
int flag = 0;

// data structure
typedef struct
{
  char str[150];
  int num;
}info;

info info1, info2;
info *p1 = &info1;
info *p2 = &info2;

int main(int argc, char *argv[])
{
	// setting up clock for time stamps
	clock_gettime(CLOCK_REALTIME, &timespec_struct);

	printf("\nClient Started\n");

	char target_ip[30];

	if(argc > 2)
	{
		strcpy(target_ip, argv[2]);
		printf("ip entered - trying to connect to: %s\n", target_ip);
	}
	else
	{
		strcpy(target_ip, "192.168.50.122");
		printf("No ip entered - trying to connect to default ip: %s\n", target_ip);
	}

	if(argc > 1)
	{
		if(strcmp("tempc", argv[1]) == 0)
		{
			req_warning = Temp_Warning_Req;
			strcpy(p1->str, "Temperature");
			p1->num = 1;
			printf("Requesting Temperature in C\n");
		}
		else if(strcmp("tempf", argv[1]) == 0)
		{
			req_warning = Temp_Warning_Req;
			strcpy(p1->str, "Temperature");
			p1->num = 2;
			printf("Requesting Temperature in F\n");
		}
		else if(strcmp("tempk", argv[1]) == 0)
		{
			req_warning = Temp_Warning_Req;
			strcpy(p1->str, "Temperature");
			p1->num = 3;
			printf("Requesting Temperature in K\n");
		}
		else if(strcmp("lux", argv[1]) == 0)
		{
			req_warning = Lux_Warning_Req;
			printf("\nSending Lux Request\n");
			strcpy(p1->str, "Lux");
			p1->num = 1;
			printf("Requesting Lux\n");
		}
		else
		{
			req_warning = Temp_Warning_Req;
			strcpy(p1->str, "Temperature");
			p1->num = 1;
			printf("Invalid Argument - Requesting Temperature in C\n");
		}
	}
	else
	{
		req_warning = Temp_Warning_Req;
		strcpy(p1->str, "Temperature");
		p1->num = 1;
		printf("No Argument - Requesting Temperature in C\n");
	}


	int new_socket, info_in, info_out;
	struct sockaddr_in client;
	struct hostent *custom_host;

	// socket init on client end
	new_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(new_socket < 0)
	{
		printf("\nSocket Creation Failed\n");
		return(0);
	}

	struct timeval tout;
	tout.tv_sec = 2;
	tout.tv_usec = 0;
	if (setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(struct timeval)))
	{
		perror("Error in Setsockopt\n");
		return(0);
	}

	client.sin_family = AF_INET;

	custom_host = gethostbyname("localhost");
	if(custom_host == 0)
	{
		printf("\nSocket Getting Failed\n");
		return(0);
	}

	if(inet_pton(AF_INET, target_ip, &client.sin_addr)<=0)
	{
	printf("\nInvalid address/ Address not supported \n");
	return -1;
	}

//	memcpy(&client.sin_addr, custom_host->h_addr, custom_host->h_length);
	client.sin_port = htons(PORT);

	if(connect(new_socket, (struct sockaddr *)&client, sizeof(client)) < 0)
	{
		printf("\nSocket Connection Failed\n");
		return(0);
	}

	// socket init complete, main process start

	printf("\n\nClient Start with PID: %d\n", getpid());
	gettimeofday(&current_time, NULL);
	printf("Time Stamp: *%lu.%06lu*\n", current_time.tv_sec, current_time.tv_usec);
    	printf("IPC using Socket\n");
	printf("No special resources being utilized\n");

	// tx1, rx1 - both contains two messages - 1 string and 1 integer



	gettimeofday(&current_time, NULL);
	printf("\n<%lu.%06lu> Sending to Server *%s* and Unit: %d", current_time.tv_sec, current_time.tv_usec, p1->str, p1->num);
    	info_out = write(new_socket,p1,sizeof(info));
	if (info_out < 0)
	{
		printf("\nSocket Writing Failed\n");
		return(0);
	}
	gettimeofday(&current_time, NULL);
	info_in = read(new_socket,p2,sizeof(info));
	if(info_in < 0)
	{
		printf("\nSocket Reading Failed\n");
		return(0);
	}
	else if(info_in == 0)	printf("NULL Read\n");
	else if(info_in == sizeof(info))
	{
		printf("\n<%lu.%06lu> String from Server\n%s", current_time.tv_sec, current_time.tv_usec, p2->str);
		if(req_warning == Temp_Warning_Req)
		{
			if(p2->num == Temp_Normal)	printf("\nTemperature is Normal\n");
			else if(p2->num == Temp_Warning_High)		printf("\nIt's too hot\n");
			else if(p2->num == Temp_Warning_Low)		printf("\nIt's too cold\n");
			else		printf("\nTemperature state couldn't be determined\n")	;
		}
		else if(req_warning == Lux_Warning_Req)
		{
			if(p2->num == Lux_State_Day)		printf("\nIt's Daytime\n");
			else if(p2->num == Lux_State_Night)		printf("\nIt's Nighttime\n");
			else		printf("\nDay or Night couldn't be determined\n");
		}
		else		printf("\nNo valid warning acquired\n");
	}


	// Wait for termination signal
//	while(flag == 0);
	close(new_socket);
	return 0;
}
