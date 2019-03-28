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
#include <sys/socket.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <netdb.h>

#include <assert.h>
#include <errno.h>

#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <malloc.h>

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include <float.h>
#include <complex.h>
#include <time.h>
#include <mqueue.h>

extern pthread_mutex_t lock;

extern volatile sig_atomic_t flag;
extern volatile uint8_t LogKillSafe;
extern volatile uint8_t AliveThreads;

extern uint8_t Temp_Warning;
extern uint8_t Lux_Warning;

#define Temperature_Signal	0xF0
#define Lux_Signal		0xF1

#define Celsius		1
#define Fahrenheit	2
#define Kelvin		3

#define PORT 8080

//Path of I2C Bus
#define I2C_BUS		(char *)"/dev/i2c-2"

#endif
