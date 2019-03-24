#ifndef __LIGHTSENSOR_H__
#define __LIGHTSENSOR_H__

#include "main.h"

#define time_high 0x02  //for 402ms
#define time_med 0x01  //for 101ms
#define time_low 0x00  //for 13ms

#define gain 0x10  //for maximum gain

int control_reg_wr ( int fd, int msg);
int control_reg_rd ( int fd);
int timing_reg_wr ( int fd, int msg);
int timing_reg_rd(int fd);
int control_reg_int_wr(int fd, int msg);
int control_reg_int_rd(int fd);
int threshold_int_reg_wr(int fd, int *array);
int threshold_int_reg_rd(int fd, int *array);
int id_reg_rd(int fd);
uint16_t data0_reg_rd(int fd);
uint16_t data1_reg_rd(int fd);
float get_lux(void);
int light_init(void);
int all_reg_rd_wr(int fd);

#endif
