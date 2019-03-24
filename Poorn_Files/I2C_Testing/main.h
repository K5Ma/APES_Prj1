#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>

#include <assert.h>
#include <errno.h>

#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include <float.h>
#include <complex.h>
#include <time.h>       
#include <mqueue.h>


pthread_t logger_id, light_id, temp_id, socket_id; 

char file_name[50];

typedef struct              //structure to be sent
{             
char timestamp[50];     
int source_id;
int log_level;
int data;
float value;
char random_string[50];
}mystruct;

struct threadParam
{
char *filename;
};

int light_client(void);
void *func_light(void);
int temp_client(void);
void *func_temp(void);
void* logger_task(void);

void* func_socket(void);

int check_status(void);
int startup_test(void);

#endif
