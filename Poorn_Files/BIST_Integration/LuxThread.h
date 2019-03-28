#ifndef LUXTHREAD_H_
#define LUXTHREAD_H_
/*
*		File: LuxThread.h
*		Purpose: To interface with Light Sensor (APDS9301) on I2C bus, on BeagleBoneGreen, running custom linux kernel (platform by Buildroot)
*		Owner: Poorn Mehta
*		Date: 3/26/2019
*/
#include "main.h"



// Address of light sensor
#define Lux_Addr	0x39

#define Lux_Max_Retries			10
#define Lux_No_Retry				0

#define Lux_State_Night			0x01
#define Lux_State_Day				0x02

#define Lux_Night_Level			100

// Internal register addresses
#define Lux_Control_Reg					0x00
#define Lux_Timing_Reg					0x01
#define Lux_ThrLow_Low_Reg			0x02
#define Lux_ThrLow_High_Reg			0x03
#define Lux_ThrHigh_Low_Reg			0x04
#define Lux_ThrHigh_High_Reg		0x05
#define Lux_Intrp_Ctrl_Reg			0x06
#define Lux_ID_Reg							0x0A
#define Lux_Data0_Low						0x0C
#define Lux_Data0_High					0x0D
#define Lux_Data1_Low						0x0E
#define Lux_Data1_High					0x0F

// Reserved internal register addresses
#define Lux_Res_Reg1						0x07
#define Lux_Res_Reg2						0x08
#define Lux_Res_Reg3						0x09
#define Lux_Res_Reg4						0x0B

// ALL Macros returs 0 on success and 1 on failure

// Defines and macros for gain and integration time settings
#define Lux_Integration_Pos			0
#define Lux_Integration_Mask		(3 << Lux_Integration_Pos)
#define Lux_Low_Integration_Time		(0 << Lux_Integration_Pos)		// 13.7ms
#define Lux_Test_Low_Int_Time(x)		((x & Lux_Integration_Mask) >> Lux_Integration_Pos) == Lux_Low_Integration_Time ? 0 : 1
#define Lux_Med_Integration_Time		(1 << Lux_Integration_Pos)		// 101ms
#define Lux_Test_Med_Int_Time(x)		((x & Lux_Integration_Mask) >> Lux_Integration_Pos) == Lux_Med_Integration_Time ? 0 : 1
#define Lux_High_Integration_Time		(2 << Lux_Integration_Pos)		// 402ms
#define Lux_Test_High_Int_Time(x)		((x & Lux_Integration_Mask) >> Lux_Integration_Pos) == Lux_High_Integration_Time ? 0 : 1

#define Lux_Gain_Pos					4
#define Lux_Gain_Mask					(1 << Lux_Gain_Pos)
#define Lux_Set_Gain_Low(x)				x &	~(Lux_Gain_Mask)
#define Lux_Test_Gain_Low(x)			((x &	Lux_Gain_Mask) >> Lux_Gain_Pos) == 0 ? 0 : 1
#define Lux_Set_Gain_High(x)				x |	Lux_Gain_Mask
#define Lux_Test_Gain_High(x)			((x &	Lux_Gain_Mask) >> Lux_Gain_Pos) == 1 ? 0 : 1

// Defines and macros for interrupt controller manipulation
#define Lux_Interrupt_Test_Data			0x1F		// Turning on Level Interrupt as well as checking N config (from datasheet)
#define	Lux_Interrupt_Control_Mask		0x1F
#define Lux_Test_Intrp_Ctrl_Data(x)		(x & Lux_Interrupt_Control_Mask) == Lux_Interrupt_Test_Data ? 0 : 1

// Defines and macros for interrupt threshold registers manipulation
#define Lux_ThrLow_Low_Test_Data			0x05
#define Lux_ThrLow_High_Test_Data			0x0A
#define Lux_ThrHigh_Low_Test_Data			0x0F
#define Lux_ThrHigh_High_Test_Data		0xF0
#define Lux_Test_ThrLow_Low(x)				x == Lux_ThrLow_Low_Test_Data ? 0 : 1
#define Lux_Test_ThrLow_High(x)				x == Lux_ThrLow_High_Test_Data ? 0 : 1
#define Lux_Test_ThrHigh_Low(x)				x == Lux_ThrHigh_Low_Test_Data ? 0 : 1
#define Lux_Test_ThrHigh_High(x)				x == Lux_ThrHigh_High_Test_Data ? 0 : 1

// Defines and macros for verifying ID register of the sensor
#define Lux_Part_No				5				// Fixed value, from datasheet
#define Lux_Part_No_Pos		4
#define Lux_Part_No_Mask	(Lux_Part_No << Lux_Part_No_Pos)
#define Lux_Test_Part_No(x)		((x & Lux_Part_No_Mask) >> Lux_Part_No_Pos) == Lux_Part_No ? 0 : 1

//Macro that uses write() API for writing to I2C bus, returns 0 on success, 1 on failure
#define generic_write_lux_reg(t_val, t_bytes)				(write(lux_file_des, t_val, t_bytes)) == t_bytes ? 0 : 1

//Macro that uses read() API for reading from I2C bus, returns 0 on success, 1 on failure
#define generic_read_lux_reg(t_val, t_bytes)				(read(lux_file_des, t_val, t_bytes)) == t_bytes ? 0 : 1

// Defines and macros for using command register with high flexibility
#define Lux_Command_Bit_Pos			7
#define Lux_Command_Reg_Mask		(1 << Lux_Command_Bit_Pos)
#define Lux_Command_Address_Mask		(0x0F)
#define Lux_Command_Reg_Data(x)		((x & Lux_Command_Address_Mask) | Lux_Command_Reg_Mask)
#define Lux_Word_Mode_Bit_Pos		5
#define Lux_Word_Mode_Mask		Lux_Command_Reg_Mask | (1 << Lux_Word_Mode_Bit_Pos)
#define Lux_Command_Word_Data(x)		((x & Lux_Command_Address_Mask) | Lux_Word_Mode_Mask)
//Macro to write to command register
#define write_reg_cmd(x)															(generic_write_lux_reg(x, 1)) == 0 ? 0 : 1

//Macro to write to any other internal register, after writing to command register
#define lux_write_reg(x, t_bytes)															(generic_write_lux_reg(x, t_bytes)) == 0 ? 0 : 1

//Macro to read from any internal register, after writing to command register
#define lux_read_reg(x, t_bytes)															(generic_read_lux_reg(x, t_bytes)) == 0 ? 0 : 1

// Macros and defines for turning ON the power to the sensor
#define Lux_Control_Power_ON		0x03
#define Lux_Control_Pos					0
#define Lux_Control_Mask				((Lux_Control_Power_ON) << Lux_Control_Pos)
#define Lux_Test_Control_Power(x)		(x & Lux_Control_Mask) == Lux_Control_Power_ON ? 0 : 1

// Function prototypes
uint8_t custom_lux_reg_write(uint8_t r_addr, uint8_t r_val);
uint8_t custom_lux_reg_read(uint8_t r_addr, uint8_t *r_val);
uint8_t custom_test_lux_config(void);
uint8_t custom_lux_init(void);
uint8_t get_lux(float *l_data);

/**************************************************************************************************************
 * USAGE: This function contains all what the lux pthread will do.
 *
 * PARAMETERS:
 *            - NONE
 *
 * RETURNS: NONE
 *
 **************************************************************************************************************/
void * LuxThread(void * args);


/**************************************************************************************************************
 * USAGE: This function will be called initially when the Lux Thread is first initialized.
 *
 * PARAMETERS:
 *            - NONE
 *
 * RETURNS: NONE
 *
 **************************************************************************************************************/
uint8_t LuxThread_Init(void);


#endif /* LUXTHREAD_H_ */
