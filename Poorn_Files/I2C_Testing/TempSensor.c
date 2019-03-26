#include "TempSensor.h"

/*
*		File: TempSensor.c
*		Purpose: To interface with Temperature Sensor (TMP102) on I2C bus, on BeagleBoneGreen, running custom linux kernel (platform by Buildroot)
*		Owner: Poorn Mehta
*		Date: 3/26/2019
*/

//Initializing global variable - this will contain I2C bus's file descriptor
int temp_file_des = -1;

/*
*		Function to write to any internal register of Temperature Sensor
*		Returns 1 on failure, 0 on success
*		First Parameter is Internal Address of the Register
*		Second Parameter is the value to be written (12bit, left justified)
*/
uint8_t custom_temp_reg_write(uint8_t r_addr, uint16_t r_val)
{
		static uint8_t r_arr[3];
		r_arr[0] = r_addr;
		r_arr[1] = r_val >> 8;
		r_arr[2] = r_val & 0x00FF;
		if((r_addr < 1) || (r_addr > 3))		// Checking whether the passed register address is valid and writeable or not
		{
				perror("\nInvalid Register Address Supplied (Temp Sensor)\n");
				return 1;
		}
		if(write_reg_ptr(&r_addr))		// Writing to pointer register first
		{
				perror("\nError in write_reg_ptr\n");
				return 1;
		}
		if(write_reg(&r_arr))		// Writing to the actual register
		{
				perror("\nError in write_reg_conf\n");
				return 1;
		}
		return 0;
}

/*
*		Function to read from any internal register of Temperature Sensor
*		Returns 1 on failure, 0 on success
*		First Parameter is Internal Address of the Register
*		Second Parameter is the pointer to destination variable(12bit, left justified)
*/
uint8_t custom_temp_reg_read(uint8_t r_addr, uint8_t *r_val)
{
		if(write_reg_ptr(&r_addr))		// Writing to pointer register
		{
				perror("\nError in write_reg_ptr\n");
				return 1;
		}
		if(read_reg(r_val))		// Reading the data, and storing in the pointer through macro
		{
				perror("\nError in write_reg_conf\n");
				return 1;
		}
		return 0;
}

/*
*		Function to set temperature thresholds provided by defines in the header file
*		Returns 1 on failure, 0 on success
*		No Parameters
*/
uint8_t custom_set_temp_thresholds(void)
{
		static uint8_t temp_reg_return[2];

		// Writing THigh Register
		if(custom_temp_reg_write(Temp_THigh_Reg, Temp_High_Threshold << 8))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		// Reading back THigh Register
		if(custom_temp_reg_read(Temp_THigh_Reg, &temp_reg_return[0]))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		// Verifying THigh Register
		if(temp_reg_return[0] != Temp_High_Threshold)
		{
					perror("\nTHigh Setting Failed\n");
					return 1;
		}
		printf("\nTHigh Set at %d deg C Successfully\n", Temp_High_Threshold);

		// Writing TLow Register
		if(custom_temp_reg_write(Temp_TLow_Reg, Temp_Low_Threshold << 8))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		// Reading back TLow Register
		if(custom_temp_reg_read(Temp_TLow_Reg, &temp_reg_return[0]))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		// Verifying TLow Register
		if(temp_reg_return[0] != Temp_Low_Threshold)
		{
				perror("\nTLow Setting Failed\n");
				return 1;
		}
		printf("\nTLow Set at %d deg C Successfully\n", Temp_Low_Threshold);

		return 0;
}

/*
*		Function to test all settings of Temperature Sensor through Configuration Register
*		Returns 1 on failure, 0 on success
*		No Parameters
*/
uint8_t custom_test_temp_config(void)
{
		static uint8_t temp_reg_return[2];
		static uint16_t temp_config_return;

		//First Reading without Writing
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying default value
		if(temp_config_return != Temp_Config_Default)
		{
				perror("\nDefault Temp Config Check Failed\n");
				printf("\nExpected: %x Got: %x\n",Temp_Config_Default , temp_config_return);
				return 1;
		}
		printf("\nDefault Temp Config Check Succeeded\n");

		//Setting Shutdown Mode to ON
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Shutdown_On))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		//Reading Shutdown Mode Status
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying Shutdown Mode Setup
		if(Temp_Test_Shutdown_On(temp_config_return))
		{
				perror("\nShutdown Mode ON Test Failed\n");
				return 1;
		}
		printf("\nShutdown Mode ON Test Succeeded\n");

		//Setting Shutdown Mode to OFF
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Shutdown_Off))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		//Reading Shutdown Mode Status
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying Shutdown Mode Setup
		if(Temp_Test_Shutdown_Off(temp_config_return))
		{
				perror("\nShutdown Mode OFF Test Failed\n");
				return 1;
		}
		printf("\nShutdown Mode OFF Test Succeeded\n");

		//Fault Bits - Checking both bits by setting them to 1
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Fault_Test))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		//Reading Fault Bits Status
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying Fault Bits Setup
		if(Temp_Test_Fault(temp_config_return))
		{
				perror("\nFault Bits Test Failed\n");
				return 1;
		}
		printf("\nFault Bits Test Succeeded\n");

		//Setting Extended Mode to ON
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Extended_Set))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		//Reading Extended Mode Status
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1]
		//Verifying Extended Mode Setup
		if(Temp_Test_Extended_Set(temp_config_return))
		{
				perror("\nExtended Mode Set Test Failed\n");
				return 1;
		}
		printf("\nExtended Mode Set Test Succeeded\n");

		//Setting Extended Mode to OFF
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Extended_Clear))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		//Reading Extended Mode Status
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying Extended Mode Setup
		if(Temp_Test_Extended_Clear(temp_config_return))
		{
				perror("\nExtended Mode Clear Test Failed\n");
				return 1;
		}
		printf("\nExtended Mode Clear Test Succeeded\n");

		//Setting up Conversion Rate
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Conversion_Test))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		//Reading Conversion Rate
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying Conversion Rate Setup
		if(Temp_Test_Conversion(temp_config_return))
		{
				perror("\nConversion Rate Test Failed\n");
				return 1;
		}
		printf("\nConversion Rate Test Succeeded\n");

		return 0;
}

/*
*		Function to read temperature in deg C
*		Returns 1 on failure, 0 on success
*		First Parameter is the pointer to destination variable(float)
*/
uint8_t get_temp(float *t_data)
{
		static uint8_t temp_reg_return[2];
		if(custom_temp_reg_read(Temp_Data_Reg, &temp_reg_return[0]))
		{
				perror("\nFailed: custom_temp_reg_write\n");
				return 1;
		}
		*t_data = ((temp_reg_return[0] << 8) | temp_reg_return[1]) >> 4;
		*t_data *= 0.0625;
		return 0;
}

/*
*		Function to initialize temperature sensor
*		Returns 1 on failure, 0 on success
*		No Parameters
*/
uint8_t custom_temp_init(void)
{
		temp_file_des = open(I2C_BUS, O_RDWR);
		if(temp_file_des == -1)
		{
				perror("\nIn Temp - open() with I2C_BUS failed\n");
				return 1;
		}
		if(ioctl(temp_file_des, I2C_SLAVE, Temp_Addr) == -1)
		{
				perror("\nIn Temp - ioctl() with I2C_BUS failed\n");
				return 1;
		}
		return 0;
}

// Main Function
int main(void)
{
		if(custom_temp_init())		perror("\nError while Initializing Temperature Sensor\n");
		else		printf("\n>>>>>>>Temp Init Done<<<<<<<\n");

		if(custom_set_temp_thresholds())	perror("\nError while setting thresholds\n");
		else		printf("\n>>>>>>>Temp Thresholds Set<<<<<<<\n");

		if(custom_test_temp_config())	perror("\nError while testing config\n");
		else		printf("\n>>>>>>>Temp Config Test Succeeded<<<<<<<\n");
		//Resetting Config
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Config_Default))		perror("\nFailed: custom_temp_reg_write\n");

		float t2;
		uint8_t t_warning;
		int i;
		for(i = 0; i < 20; i ++)
		{
				if(get_temp(&t2))		perror("\nError while getting Temperature\n");
				else			printf("\nCurrent Temp: %f", t2);
		}
}
