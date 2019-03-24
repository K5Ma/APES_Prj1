#ifndef __TEMPSENSOR_H__
#define __TEMPSENSOR_H__

#include "main.h"



int temp_init();

float read_temp_data_reg(int unit);


int all_temprg_rd_wr();
uint16_t read_temp_config_register();
uint16_t read_tlow_reg(int reg);
int write_config_register_default();
int write_config_reg_conv_rate(uint8_t value );
int write_config_reg_em(uint8_t value );
int write_config_reg_on_off(uint8_t value );
int write_tlow_reg(int reg, uint16_t value );
void write_pointer_reg(uint8_t value);

#endif
