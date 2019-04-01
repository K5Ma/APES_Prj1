/*
*	File: main.c
*	Purpose: Unit Test APES Project 1 - 2019 
*	Owners: Poorn Mehta & Khalid AlAwadhi
* 
* 
*	PURPOSE OF TEST:
*	The purpose of this test is to test our start-up opeartion of the Temp sensor.  
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

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


/***************************************
 *  Thread Numbering Enum:             *
 *  Used for source and destination    *   
 ***************************************/
typedef enum
{
	Main = 1,
	Logging = 2,
	Socket = 3,
	Temp = 4,
	Lux = 5
} Sources;


/***************************************
 *        Message Structure            *
 ***************************************/
typedef struct MsgStruct 
{
	uint8_t Source;
	uint8_t Dest;
	char LogLevel[150];				//Expected values: INFO | WARNING | ERROR | CRITICAL
	char Msg[150];
} MsgStruct;



/***************************************
 *  Log_error() Function Parameters:   *
 ***************************************/
#define LOCAL_ONLY					0x03 



double GetCurrentTime()
{
	/* Declaring structure for time */
	struct timeval time;
	
	/* Get current time and save it */
	gettimeofday(&time, 0);
	
	/* Combine the Secs with uSecs by typecasting
     * Long Int to Double and return as 1 Double value */
	return (double)(time.tv_sec)+(((double)(time.tv_usec))/1000000);
}


void Log_error(uint8_t Src, char* Err_Msg, int errnum, uint8_t SendToLogging)
{
	char Error_Log[150];										//Stores the complete error message
	char ErrMsg_strerror[150];									//Stores the strerror_r error message
	
	strerror_r(errnum, ErrMsg_strerror, 150);					//Get error via a thread-safe function
	
	sprintf(Error_Log, "%s: %s", Err_Msg, ErrMsg_strerror);		//Combine user message with the strerror
	
	
	/* Get name of source */
	char* Source_text; 
	switch(Src)
	{
		case Main:
			Source_text = "Main Thread";
			break;

		case Logging:
			Source_text = "Logging Thread";
			break;

		case Socket:
			Source_text = "Socket Thread";
			break;

		case Temp:
			Source_text = "Temp Thread";
			break;

		case Lux:
			Source_text = "Lux Thread";
			break;

		default:
			Source_text = "Unknown Thread";
			break;
	}
			
			

			printf("[%lf] Error in Thread '%s' => %s\n\n", GetCurrentTime(), Source_text, Error_Log);

}



//Initializing global variable - this will contain I2C bus's file descriptor
int temp_file_des;


//I2C Address of the Sensor
#define Temp_Addr		0x48

#define Temp_Normal					0x00
#define Temp_Warning_High		0x01
#define Temp_Warning_Low		0x02

#define Temp_Max_Retries			10
#define Temp_No_Retry					0

//Internal Addresses of different Registers
#define Temp_Data_Reg			0x00
#define Temp_Config_Reg		0x01
#define Temp_TLow_Reg			0x02
#define Temp_THigh_Reg		0x03

//Default expected value of the Configuration Register
//Both of the following are possibilities
#define Temp_Config_Default_1		0x6080
#define Temp_Config_Default_2		0x60A0

//Macro that uses write() API for writing to I2C bus, returns 0 on success, 1 on failure
#define generic_write_temp_reg(t_val, t_bytes)				(write(temp_file_des, t_val, t_bytes)) == t_bytes ? 0 : 1

//Macro that uses read() API for reading from I2C bus, returns 0 on success, 1 on failure
#define generic_read_temp_reg(t_val, t_bytes)				(read(temp_file_des, t_val, t_bytes)) == t_bytes ? 0 : 1

//Macro to write to pointer register
#define write_reg_ptr(x)															(generic_write_temp_reg(x, 1)) == 0 ? 0 : 1

//Macro to write to any other internal register, after writing to pointer register
#define temp_write_reg(x)															(generic_write_temp_reg(x, 3)) == 0 ? 0 : 1

//Macro to read from any internal register, after writing to pointer register
#define temp_read_reg(x)															(generic_read_temp_reg(x, 2)) == 0 ? 0 : 1

//Shutdown Mode Defines and Macros
#define Temp_Shutdown_Pos			8		// Position of the bit controlling Shutdown Mode
#define Temp_Shutdown_Mask		(1 << Temp_Shutdown_Pos)	// Mask for multiple purposes
#define Temp_Write_Shutdown_On		(Temp_Config_Default_1 | Temp_Shutdown_Mask)	// Writing 1 to the bit sets the mode ON
#define Temp_Write_Shutdown_Off		(Temp_Config_Default_1 & (~Temp_Shutdown_Mask))		// Writing 0 to the bit sets the mode OFF
#define Temp_Test_Shutdown_On(x)		((x & Temp_Shutdown_Mask) >> Temp_Shutdown_Pos) == 1 ? 0 : 1		// Macro that returns 0 on success (Shutdown mode is ON), 1 on failure
#define Temp_Test_Shutdown_Off(x)		((x & Temp_Shutdown_Mask) >> Temp_Shutdown_Pos) == 0 ? 0 : 1		// Macro that returns 0 on success (Shutdown mode is OFF), 1 on failure

//Fault Bits Defines and Macros
#define Temp_Fault_Pos			11	// Position of the least significant fault bit
#define Temp_Fault_Mask			(3 << Temp_Fault_Pos)		// Masks both fault bits
#define Temp_Write_Fault_Test		(Temp_Config_Default_1 | Temp_Fault_Mask)	// Setting both bits 1 just for the sake of testing since this will flip the both bits from default
#define Temp_Test_Fault(x)		((x & Temp_Fault_Mask) >> Temp_Fault_Pos) == 3 ? 0 : 1		// Macro that returns 0 on success (Fault Bits are Successfully set), 1 on failure

//Extended Mode Defines and Macros
#define Temp_Extended_Pos			4		// Position of the bit controlling Extended Mode
#define Temp_Extended_Mask		(1 << Temp_Extended_Pos)	// Mask for multiple purposes
#define Temp_Write_Extended_Set		(Temp_Config_Default_1 | Temp_Extended_Mask)	// Writing 1 to the bit sets the mode ON
#define Temp_Write_Extended_Clear		(Temp_Config_Default_1 & (~Temp_Extended_Mask))		// Writing 0 to the bit sets the mode OFF
#define Temp_Test_Extended_Set(x)		((x & Temp_Extended_Mask) >> Temp_Extended_Pos) == 1 ? 0 : 1	// Macro that returns 0 on success (Extended mode is Set), 1 on failure
#define Temp_Test_Extended_Clear(x)		((x & Temp_Extended_Mask) >> Temp_Extended_Pos) == 0 ? 0 : 1	// Macro that returns 0 on success (Extended mode is Clear), 1 on failure

//Conversion Rate	Defines and Macros
#define Temp_Conversion_Pos			6		// Position of the least significant conversion rate bit
#define Temp_Conversion_Mask			(3 << Temp_Conversion_Pos)		//Masks both conversion rate bits
#define Temp_Write_Conversion_Test		(Temp_Config_Default_1 ^ Temp_Conversion_Mask)	// This will inverse both bits from default
#define Temp_Test_Conversion(x)		((x & Temp_Conversion_Mask) >> Temp_Conversion_Pos) == 1 ? 0 : 1		// Macro that returns 0 on success (Conversion Rate Successfully set), 1 on failure

#define Temp_High_Threshold		30		// Temperature in degree C, this defines the highest temperature crossing which - the alert pin will flip
#define Temp_Low_Threshold		20		// Temperature in degree C, this defines the lowest temperature crossing which - the alert pin will flip


uint8_t Temp_Warning = Temp_Normal;
uint8_t Temp_Error_Retry;
uint8_t Temp_Sensor_State;

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
				Log_error(Temp, "Invalid Register Address Supplied (Temp Sensor)", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		if(write_reg_ptr(&r_addr))		// Writing to pointer register first
		{
				Log_error(Temp, "write_reg_ptr", errno, LOCAL_ONLY);
				return 1;
		}
		if(temp_write_reg(&r_arr))		// Writing to the actual register
		{
				Log_error(Temp, "temp_write_reg", errno, LOCAL_ONLY);
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
				Log_error(Temp, "write_reg_ptr", errno, LOCAL_ONLY);
				return 1;
		}
		if(temp_read_reg(r_val))		// Reading the data, and storing in the pointer through macro
		{
				Log_error(Temp, "temp_read_reg", errno, LOCAL_ONLY);
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
		static char local_text[150];

		// Writing THigh Register
		if(custom_temp_reg_write(Temp_THigh_Reg, Temp_High_Threshold << 8))
		{
				Log_error(Temp, "Write: Temp_THigh_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		// Reading back THigh Register
		if(custom_temp_reg_read(Temp_THigh_Reg, &temp_reg_return[0]))
		{
				Log_error(Temp, "Read: Temp_THigh_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		// Verifying THigh Register
		if(temp_reg_return[0] != Temp_High_Threshold)
		{
					Log_error(Temp, "THigh Setup", ENOMSG, LOCAL_ONLY);
					return 1;
		}
		sprintf(local_text, "THigh Set at %d deg C Successfully\n\n", Temp_High_Threshold);
		printf(local_text);

		// Writing TLow Register
		if(custom_temp_reg_write(Temp_TLow_Reg, Temp_Low_Threshold << 8))
		{
				Log_error(Temp, "Write: Temp_TLow_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		// Reading back TLow Register
		if(custom_temp_reg_read(Temp_TLow_Reg, &temp_reg_return[0]))
		{
				Log_error(Temp, "Read: Temp_TLow_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		// Verifying TLow Register
		if(temp_reg_return[0] != Temp_Low_Threshold)
		{
				Log_error(Temp, "TLow Setup", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		sprintf(local_text, "TLow Set at %d deg C Successfully\n\n", Temp_Low_Threshold);
		printf(local_text);

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
		static char local_text[150];

		//First Reading without Writing
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				Log_error(Temp, "Read: Temp_Config_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying default value
		if((temp_config_return != Temp_Config_Default_1) && (temp_config_return != Temp_Config_Default_2))
		{
				sprintf(local_text, "Test: Default Config - Got %x Expected %x or %x",temp_config_return, Temp_Config_Default_1, Temp_Config_Default_2);
				Log_error(Temp, local_text, ENOMSG, LOCAL_ONLY);
				return 1;
		}
		printf("Default Temp Config Check Succeeded\n\n");

		//Setting Shutdown Mode to ON
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Shutdown_On))
		{
				Log_error(Temp, "Write: Temp_Config_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		//Reading Shutdown Mode Status
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				Log_error(Temp, "Read: Temp_Config_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying Shutdown Mode Setup
		if(Temp_Test_Shutdown_On(temp_config_return))
		{
				sprintf(local_text, "Test: Shutdown Mode - Got %x Expected 1",((temp_config_return & Temp_Shutdown_Mask) >> Temp_Shutdown_Pos));
				Log_error(Temp, local_text, ENOMSG, LOCAL_ONLY);
				return 1;
		}

		//Setting Shutdown Mode to OFF
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Shutdown_Off))
		{
				Log_error(Temp, "Write: Temp_Config_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		//Reading Shutdown Mode Status
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				Log_error(Temp, "Read: Temp_Config_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying Shutdown Mode Setup
		if(Temp_Test_Shutdown_Off(temp_config_return))
		{
				sprintf(local_text, "Test: Shutdown Mode - Got %x Expected 0",((temp_config_return & Temp_Shutdown_Mask) >> Temp_Shutdown_Pos));
				Log_error(Temp, local_text, ENOMSG, LOCAL_ONLY);
				return 1;
		}
//		SendToThreadQ(Temp, Logging, "INFO", "\nShutdown Mode ON & OFF Test Succeeded\n");

		//Fault Bits - Checking both bits by setting them to 1
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Fault_Test))
		{
				Log_error(Temp, "Write: Temp_Config_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		//Reading Fault Bits Status
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				Log_error(Temp, "Read: Temp_Config_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying Fault Bits Setup
		if(Temp_Test_Fault(temp_config_return))
		{
				sprintf(local_text, "Test: Fault Bits - Got %x Expected 3",((temp_config_return & Temp_Fault_Mask) >> Temp_Fault_Pos));
				Log_error(Temp, local_text, ENOMSG, LOCAL_ONLY);
				return 1;
		}
		printf("Fault Bits Test Succeeded\n\n");

		//Setting Extended Mode to ON
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Extended_Set))
		{
				Log_error(Temp, "Write: Temp_Config_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		//Reading Extended Mode Status
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				Log_error(Temp, "Read: Temp_Config_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying Extended Mode Setup
		if(Temp_Test_Extended_Set(temp_config_return))
		{
				sprintf(local_text, "Test: Extended Mode Set - Got %x Expected 1",((temp_config_return & Temp_Extended_Mask) >> Temp_Extended_Pos));
				Log_error(Temp, local_text, ENOMSG, LOCAL_ONLY);
				return 1;
		}

		//Setting Extended Mode to OFF
	if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Extended_Clear))
		{
				Log_error(Temp, "Write: Temp_Config_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		//Reading Extended Mode Status
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				Log_error(Temp, "Read: Temp_Config_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying Extended Mode Setup
		if(Temp_Test_Extended_Clear(temp_config_return))
		{
				sprintf(local_text, "Test: Extended Mode Clear - Got %x Expected 0",((temp_config_return & Temp_Extended_Mask) >> Temp_Extended_Pos));
				Log_error(Temp, local_text, ENOMSG, LOCAL_ONLY);
				return 1;
		}
		printf("Extended Mode Set & Clear Test Succeeded\n\n");

		//Setting up Conversion Rate
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Conversion_Test))
		{
				Log_error(Temp, "Write: Temp_Config_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		//Reading Conversion Rate
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				Log_error(Temp, "Read: Temp_Config_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying Conversion Rate Setup
		if(Temp_Test_Conversion(temp_config_return))
		{
				sprintf(local_text, "Test: Conversion Rate - Got %x Expected 0",((temp_config_return & Temp_Conversion_Mask) >> Temp_Conversion_Pos));
				Log_error(Temp, local_text, ENOMSG, LOCAL_ONLY);
				return 1;
		}
		printf("Conversion Rate Test Succeeded\n\n");

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
				Log_error(Temp, "Read: Temp_Data_Reg", ENOMSG, LOCAL_ONLY);
				return 1;
		}
		*t_data = ((temp_reg_return[0] << 8) | temp_reg_return[1]) >> 4;
		*t_data *= 0.0625;

		// Checking whether the temperature is below or above set temp Thresholds
		if(*t_data < Temp_Low_Threshold)		Temp_Warning = Temp_Warning_Low;
		else if(*t_data > Temp_High_Threshold)			Temp_Warning = Temp_Warning_High;
		else		Temp_Warning = Temp_Normal;

		return 0;
}
#define I2C_BUS		(char *)"/dev/i2c-2"

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
				Log_error(Temp, "open(): I2C Bus", errno, LOCAL_ONLY);
				return 1;
		}
		if(ioctl(temp_file_des, I2C_SLAVE, Temp_Addr) == -1)
		{
				Log_error(Temp, "ioctl(): I2C Bus", errno, LOCAL_ONLY);
				return 1;
		}
		return 0;
}


uint8_t TempThread_Init(void)
{
		if(custom_temp_init() == 0)		printf("Temperature Sensor Initiliazed Successfully\n\n");
		else
		{
				Log_error(Temp, "Temperature Sensor Initialization... Exiting", ENOMSG, LOCAL_ONLY);
				return 1;
		}

		// Resetting Config (To prevent shutdown from previously failed bootup)
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Config_Default_1) == 0)			printf("Temperature Sensor Resetted Successfully\n\n");
		else
		{
				Log_error(Temp, "Temperature Sensor Reset... Exiting", ENOMSG, LOCAL_ONLY);
				return 1;
		}

		// Setting thresholds
		if(custom_set_temp_thresholds() == 0)		printf("Temperature Sensor Thresholds Set Successfully\n\n");
		else
		{
				Log_error(Temp, "Temperature Sensor Thresholds... Exiting", ENOMSG, LOCAL_ONLY);
				return 1;
		}

		// BIST for Temp Sensor
		if(custom_test_temp_config() == 0)		printf("Temperature Sensor Built-in-self-Test Passed Successfully\n\n");
		else
		{
				Log_error(Temp, "Temperature Sensor Built-in-self-Test... Exiting", ENOMSG, LOCAL_ONLY);
				return 1;
		}

		// Resetting Config
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Config_Default_1) == 0)			printf("Temperature Sensor Resetted Successfully\n\n");
		else
		{
				Log_error(Temp, "Temperature Sensor Reset... Exiting", ENOMSG, LOCAL_ONLY);
				return 1;
		}

		printf("Starting Normal Operation\n\n");

		return 0;
}



int main(int argc, char *argv[])
{
	
	if(TempThread_Init())
	{
		Log_error(Main,"Attempt to get the Temperature Sensor Online Failed... Exiting TempThread_Init()", errno, LOCAL_ONLY);
	}
	else
	{
		printf("Temperature Sensor is Now Online...\n\n");
	}
	
	
	printf("Main pThread SUCCESS: All tests passed!\n\n");
}










//
// MAIN PTHREAD CREATE 
//int main(int argc, char *argv[])
//{
//	/* Create the needed pThreads */
//	pthread_t Log_pThread, Socket_pThread, Temp_pThread, Lux_pThread;
//
//
//	/* Create Logging pThread */
//	if(pthread_create(&Log_pThread, NULL, &LoggingThread, NULL) != 0)
//	{
//		printf("Main pThread ERROR: Could not create Logging Thread!\n\n");
//	}
//	else
//	{
//		printf("Main pThread SUCCESS: Created Logging Thread!\n\n");
//	}
//	
//	/* Create Socket pThread */
//	if(pthread_create(&Socket_pThread, NULL, &SocketThread, NULL) != 0)
//	{
//		printf("Main pThread ERROR: Could not create Socket Thread!\n\n");
//	}
//	else
//	{
//		printf("Main pThread SUCCESS: Created Socket Thread!\n\n");
//	}
//
//	/* Create Temp pThread */
//	if(pthread_create(&Temp_pThread, NULL, &TempThread, NULL) != 0)
//	{
//		printf("Main pThread ERROR: Could not create Temp Thread!\n\n");
//	}
//	else
//	{
//		printf("Main pThread SUCCESS: Created Temp Thread!\n\n");
//	}
//	
//	/* Create Lux pThread */
//	if(pthread_create(&Lux_pThread, NULL, &LuxThread, NULL) != 0)
//	{
//		printf("Main pThread ERROR: Could not create Lux Thread!\n\n");
//	}
//	else
//	{
//		printf("Main pThread SUCCESS: Created Lux Thread!\n\n");
//	}
//	
//	/* Wait for pThreads to finish */
//	pthread_join(Log_pThread, NULL);
//	pthread_join(Socket_pThread, NULL);
//	pthread_join(Temp_pThread, NULL);
//	pthread_join(Lux_pThread, NULL);
//	
//	printf("Main pThread SUCCESS: All tests passed!\n\n");
//}