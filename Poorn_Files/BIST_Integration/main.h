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

extern sig_atomic_t flag;
extern pthread_mutex_t lock;

extern int temp_file_des;
extern int lux_file_des;
extern uint8_t Temp_Error_Retry;
extern uint8_t Lux_Error_Retry;
extern uint8_t Temp_Sensor_State;
extern uint8_t Lux_Sensor_State;
extern uint8_t Temp_Warning;
extern uint8_t Lux_Warning;

extern uint8_t Counter;

#define Temperature_Signal	0xF0
#define Lux_Signal		0xF1

#define Celsius		1
#define Fahrenheit	2
#define Kelvin		3

#define Sensor_Online		1
#define Sensor_Offline	0

#define PORT 8080

#define Timer_Interval		500		// In ms
#define Sensor_Retry_Period		3000		// In ms
#define Counter_Threshold		(Sensor_Retry_Period / Timer_Interval)

//Path of I2C Bus
#define I2C_BUS		(char *)"/dev/i2c-2"

#endif
