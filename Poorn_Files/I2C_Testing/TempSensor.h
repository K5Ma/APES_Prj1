#ifndef __TEMPSENSOR_H__
#define __TEMPSENSOR_H__

#include "main.h"

#define I2C_BUS		(char *)"/dev/i2c-2"

#define Temp_Addr		0x48

#define Temp_Data_Reg			0
#define Temp_Config_Reg		1
#define Temp_TLow_Reg			2
#define Temp_THigh_Reg		3

#define Temp_Config_Default		0x6080 // 0x6080 and not 0x60A0 since I am setting THigh and TLow before reading config

//Shutdown Mode
#define Temp_Shutdown_Pos			8
#define Temp_Shutdown_Mask		(1 << Temp_Shutdown_Pos)
#define Temp_Write_Shutdown_On		(Temp_Config_Default | Temp_Shutdown_Mask)	// Writing 1 to the bit sets the mode ON
#define Temp_Write_Shutdown_Off		(Temp_Config_Default & (~Temp_Shutdown_Mask))		// Writing 0 to the bit sets the mode OFF
#define Temp_Test_Shutdown_On(x)		((x & Temp_Shutdown_Mask) >> Temp_Shutdown_Pos) == 1 ? 0 : 1
#define Temp_Test_Shutdown_Off(x)		((x & Temp_Shutdown_Mask) >> Temp_Shutdown_Pos) == 0 ? 0 : 1

//Fault Bits
#define Temp_Fault_Pos			11
#define Temp_Fault_Mask			(3 << Temp_Fault_Pos)
#define Temp_Write_Fault_Test		(Temp_Config_Default | Temp_Fault_Mask)	// Setting both bits 1 just for the sake of testing since this will flip the both bits from default
#define Temp_Test_Fault(x)		((x & Temp_Fault_Mask) >> Temp_Fault_Pos) == 3 ? 0 : 1

//Extended Mode
#define Temp_Extended_Pos			4
#define Temp_Extended_Mask		(1 << Temp_Extended_Pos)
#define Temp_Write_Extended_Set		(Temp_Config_Default | Temp_Extended_Mask)	// Writing 1 to the bit sets the mode ON
#define Temp_Write_Extended_Clear		(Temp_Config_Default & (~Temp_Extended_Mask))		// Writing 0 to the bit sets the mode OFF
#define Temp_Test_Extended_Set(x)		((x & Temp_Extended_Mask) >> Temp_Extended_Pos) == 1 ? 0 : 1
#define Temp_Test_Extended_Clear(x)		((x & Temp_Extended_Mask) >> Temp_Extended_Pos) == 0 ? 0 : 1

//Conversion Rate
#define Temp_Conversion_Pos			6
#define Temp_Conversion_Mask			(3 << Temp_Conversion_Pos)
#define Temp_Write_Conversion_Test		(Temp_Config_Default ^ Temp_Conversion_Mask)	// This will inverse both bits from default
#define Temp_Test_Conversion(x)		((x & Temp_Conversion_Mask) >> Temp_Conversion_Pos) == 1 ? 0 : 1

#define Temp_High_Threshold		30
#define Temp_Low_Threshold		20

#define generic_write_temp_reg(t_val, t_bytes)				(write(temp_file_des, t_val, t_bytes)) == t_bytes ? 0 : 1
#define generic_read_temp_reg(t_val, t_bytes)				(read(temp_file_des, t_val, t_bytes)) == t_bytes ? 0 : 1

#define write_reg_ptr(x)															(generic_write_temp_reg(x, 1)) == 0 ? 0 : 1
#define write_reg(x)															(generic_write_temp_reg(x, 3)) == 0 ? 0 : 1
#define read_reg(x)															(generic_read_temp_reg(x, 2)) == 0 ? 0 : 1

uint8_t custom_temp_reg_write(uint8_t r_addr, uint16_t r_val);
uint8_t custom_temp_reg_read(uint8_t r_addr, uint8_t *r_val);
uint8_t custom_set_temp_thresholds(void);
uint8_t custom_test_temp_config(void);
uint8_t get_temp(float *t_data);
uint8_t custom_temp_init(void);

#endif
