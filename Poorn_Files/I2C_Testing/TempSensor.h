#ifndef __TEMPSENSOR_H__
#define __TEMPSENSOR_H__

/*
*		File: TempSensor.h
*		Purpose: To interface with Temperature Sensor (TMP102) on I2C bus, on BeagleBoneGreen, running custom linux kernel (platform by Buildroot)
*		Owner: Poorn Mehta
*		Date: 3/26/2019
*/

#include "main.h"		// Contains all useful include files

//Path of I2C Bus
#define I2C_BUS		(char *)"/dev/i2c-2"

//I2C Address of the Sensor
#define Temp_Addr		0x48

//Internal Addresses of different Registers
#define Temp_Data_Reg			0
#define Temp_Config_Reg		1
#define Temp_TLow_Reg			2
#define Temp_THigh_Reg		3

//Default expected value of the Configuration Register
#define Temp_Config_Default		0x6080 // 0x6080 and not 0x60A0 since I am setting THigh and TLow before reading config

//Shutdown Mode Defines and Macros
#define Temp_Shutdown_Pos			8		// Position of the bit controlling Shutdown Mode
#define Temp_Shutdown_Mask		(1 << Temp_Shutdown_Pos)	// Mask for multiple purposes
#define Temp_Write_Shutdown_On		(Temp_Config_Default | Temp_Shutdown_Mask)	// Writing 1 to the bit sets the mode ON
#define Temp_Write_Shutdown_Off		(Temp_Config_Default & (~Temp_Shutdown_Mask))		// Writing 0 to the bit sets the mode OFF
#define Temp_Test_Shutdown_On(x)		((x & Temp_Shutdown_Mask) >> Temp_Shutdown_Pos) == 1 ? 0 : 1		// Macro that returns 0 on success (Shutdown mode is ON), 1 on failure
#define Temp_Test_Shutdown_Off(x)		((x & Temp_Shutdown_Mask) >> Temp_Shutdown_Pos) == 0 ? 0 : 1		// Macro that returns 0 on success (Shutdown mode is OFF), 1 on failure

//Fault Bits Defines and Macros
#define Temp_Fault_Pos			11	// Position of the least significant fault bit
#define Temp_Fault_Mask			(3 << Temp_Fault_Pos)		// Masks both fault bits
#define Temp_Write_Fault_Test		(Temp_Config_Default | Temp_Fault_Mask)	// Setting both bits 1 just for the sake of testing since this will flip the both bits from default
#define Temp_Test_Fault(x)		((x & Temp_Fault_Mask) >> Temp_Fault_Pos) == 3 ? 0 : 1		// Macro that returns 0 on success (Fault Bits are Successfully set), 1 on failure

//Extended Mode Defines and Macros
#define Temp_Extended_Pos			4		// Position of the bit controlling Extended Mode
#define Temp_Extended_Mask		(1 << Temp_Extended_Pos)	// Mask for multiple purposes
#define Temp_Write_Extended_Set		(Temp_Config_Default | Temp_Extended_Mask)	// Writing 1 to the bit sets the mode ON
#define Temp_Write_Extended_Clear		(Temp_Config_Default & (~Temp_Extended_Mask))		// Writing 0 to the bit sets the mode OFF
#define Temp_Test_Extended_Set(x)		((x & Temp_Extended_Mask) >> Temp_Extended_Pos) == 1 ? 0 : 1	// Macro that returns 0 on success (Extended mode is Set), 1 on failure
#define Temp_Test_Extended_Clear(x)		((x & Temp_Extended_Mask) >> Temp_Extended_Pos) == 0 ? 0 : 1	// Macro that returns 0 on success (Extended mode is Clear), 1 on failure

//Conversion Rate	Defines and Macros
#define Temp_Conversion_Pos			6		// Position of the least significant conversion rate bit
#define Temp_Conversion_Mask			(3 << Temp_Conversion_Pos)		//Masks both conversion rate bits
#define Temp_Write_Conversion_Test		(Temp_Config_Default ^ Temp_Conversion_Mask)	// This will inverse both bits from default
#define Temp_Test_Conversion(x)		((x & Temp_Conversion_Mask) >> Temp_Conversion_Pos) == 1 ? 0 : 1		// Macro that returns 0 on success (Conversion Rate Successfully set), 1 on failure

#define Temp_High_Threshold		30		// Temperature in degree C, this defines the highest temperature crossing which - the alert pin will flip
#define Temp_Low_Threshold		20		// Temperature in degree C, this defines the lowest temperature crossing which - the alert pin will flip

//Macro that uses write() API for writing to I2C bus, returns 0 on success, 1 on failure
#define generic_write_temp_reg(t_val, t_bytes)				(write(temp_file_des, t_val, t_bytes)) == t_bytes ? 0 : 1

//Macro that uses read() API for reading from I2C bus, returns 0 on success, 1 on failure
#define generic_read_temp_reg(t_val, t_bytes)				(read(temp_file_des, t_val, t_bytes)) == t_bytes ? 0 : 1

//Macro to write to pointer register
#define write_reg_ptr(x)															(generic_write_temp_reg(x, 1)) == 0 ? 0 : 1

//Macro to write to any other internal register, after writing to pointer register
#define write_reg(x)															(generic_write_temp_reg(x, 3)) == 0 ? 0 : 1

//Macro to read from any internal register
#define read_reg(x)															(generic_read_temp_reg(x, 2)) == 0 ? 0 : 1

//Function Prototypes
uint8_t custom_temp_reg_write(uint8_t r_addr, uint16_t r_val);
uint8_t custom_temp_reg_read(uint8_t r_addr, uint8_t *r_val);
uint8_t custom_set_temp_thresholds(void);
uint8_t custom_test_temp_config(void);
uint8_t get_temp(float *t_data);
uint8_t custom_temp_init(void);

#endif
