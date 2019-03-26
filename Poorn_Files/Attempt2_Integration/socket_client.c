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


// for time
struct timespec timespec_struct;
struct timeval current_time;

// flag for sigterm
int flag = 0;

// global file pointer
FILE *fptr;

// data structure
typedef struct
{
  char str[150];
  int num;
}info;

info info1, info2;
info *p1 = &info1;
info *p2 = &info2;

// led controller example function
void Command_LED(int led)
{
	gettimeofday(&current_time, NULL);
	if(led) fprintf(fptr,"<%lu.%06lu> LED is Turned ON by Client\n", current_time.tv_sec, current_time.tv_usec);
	else  fprintf(fptr,"<%lu.%06lu> LED is Turned OFF by Client\n", current_time.tv_sec, current_time.tv_usec);
}

// sigterm handler
void sig_exit(int value)
{
	flag = 1;
	printf("\nSIGTERM Received, Exiting...\n");
	gettimeofday(&current_time, NULL);
	fprintf(fptr,"\n<%lu.%06lu>Client Exiting... Closing Socket and File Pointers\n", current_time.tv_sec, current_time.tv_usec);
}

// sigterm setup
void sigact_setup(void)
{
   struct sigaction sig_act;
   memset(&sig_act, 0, sizeof(struct sigaction));
   sig_act.sa_handler = &sig_exit;
   sigaction(SIGTERM, &sig_act, 0);

}

int main(int argc, char *argv[])
{
	// setting up clock for time stamps
	clock_gettime(CLOCK_REALTIME, &timespec_struct);

	printf("\nProcess ID:%d\n", getpid());

	sigact_setup();

//GEEKSFORGEEKSTART
/*
	struct sockaddr_in address; 
	int sock = 0, valread; 
	struct sockaddr_in serv_addr; 
	char *hello = "Hello from client"; 
	char buffer[1024] = {0}; 
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
	printf("\n Socket creation error \n"); 
	return -1; 
	} 

	memset(&serv_addr, '0', sizeof(serv_addr)); 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 

	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
	{ 
	printf("\nInvalid address/ Address not supported \n"); 
	return -1; 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
	printf("\nConnection Failed \n"); 
	return -1; 
	} 
	send(sock , hello , strlen(hello) , 0 ); 
	printf("Hello message sent\n"); 
	valread = read( sock , buffer, 1024); 
	printf("%s\n",buffer ); 
	return 0; */

//GEEKSFORGEEKSEND

//ORIGINAL

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
	if (setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, 
		                                  (const char *)&tout, sizeof(tout))) 
	{ 
		printf("setsockopt\n"); 
		return(0); 
	} 

	client.sin_family = AF_INET;

	custom_host = gethostbyname("localhost");
	if(custom_host == 0)
	{
		printf("\nSocket Getting Failed\n");
		return(0);
	}

	if(inet_pton(AF_INET, "192.168.50.122", &client.sin_addr)<=0)  
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
	fptr = fopen("slog.txt","w+");
	if(fptr == 0)
	{
		printf("\nError while opening file for logging\n");
		return 0;
	}

	fprintf(fptr, "\n\nClient Start with PID: %d\n", getpid());
	gettimeofday(&current_time, NULL);
	fprintf(fptr, "Time Stamp: *%lu.%06lu*\n", current_time.tv_sec, current_time.tv_usec);
    	fprintf(fptr,"IPC using Socket\n");
	fprintf(fptr,"No special resources being utilized\n");

	// tx1, rx1 - both contains two messages - 1 string and 1 integer

	if(argc > 1)
	{
		if(strcmp("tempc", argv[1]) == 0)
		{
			strcpy(p1->str, "Temperature");
			p1->num = 1;		
		}
		else if(strcmp("tempf", argv[1]) == 0)
		{
			strcpy(p1->str, "Temperature");
			p1->num = 2;		
		}
		else if(strcmp("tempk", argv[1]) == 0)
		{
			strcpy(p1->str, "Temperature");
			p1->num = 3;		
		}
		else if(strcmp("lux", argv[1]) == 0)
		{
			printf("\nSending Lux Request\n");
			strcpy(p1->str, "Lux");
			p1->num = 1;		
		}
		else
		{
			strcpy(p1->str, "Temperature");
			p1->num = 1;		
		}
	}
	else
	{
		strcpy(p1->str, "Temperature");
		p1->num = 1;		
	}

	gettimeofday(&current_time, NULL);
	fprintf(fptr,"\n<%lu.%06lu> Sending to Server *%s* and Unit: %d", current_time.tv_sec, current_time.tv_usec, p1->str, p1->num);
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
	else
	{
		printf("\n<%lu.%06lu> String from Server *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
		fprintf(fptr,"\n<%lu.%06lu> String from Server *%s*", current_time.tv_sec, current_time.tv_usec, p2->str);
		if(p2->num == 1)
		{
			fprintf(fptr,"\n<%lu.%06lu> Temp is in C\n", current_time.tv_sec, current_time.tv_usec);
		}
		else if(p2->num == 2)
		{
			fprintf(fptr,"\n<%lu.%06lu> Temp is in F\n", current_time.tv_sec, current_time.tv_usec);
		}
		else if(p2->num == 3)
		{
			fprintf(fptr,"\n<%lu.%06lu> Temp is in K\n", current_time.tv_sec, current_time.tv_usec);	
		}
		else	fprintf(fptr,"\n<%lu.%06lu> LED State Unchanged\n", current_time.tv_sec, current_time.tv_usec);
	}


	// Wait for termination signal
//	while(flag == 0);
	
	fclose(fptr);
	close(new_socket);
}





