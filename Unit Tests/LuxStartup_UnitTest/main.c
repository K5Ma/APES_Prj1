/*
*	File: main.c
*	Purpose: Unit Test APES Project 1 - 2019 
*	Owners: Poorn Mehta & Khalid AlAwadhi
* 
* 
*	PURPOSE OF TEST:
*	The purpose of this test is to test our start-up opeartion of the Lux sensor.  
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
int lux_file_des;

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



uint8_t Lux_Error_Retry;

uint8_t Lux_Sensor_State;

uint8_t Lux_Warning = Lux_State_Day;

/*
*		Function to write to any internal register of Light Sensor
*		Returns 1 on failure, 0 on success
*		First Parameter is Internal Address of the Register
*		Second Parameter is the value to be written
*/
uint8_t custom_lux_reg_write(uint8_t r_addr, uint8_t r_val)
{
		static uint8_t lux_data_write = 0;
		lux_data_write = Lux_Command_Reg_Data(r_addr);

		if((r_addr == Lux_Res_Reg1) || (r_addr == Lux_Res_Reg2) || (r_addr == Lux_Res_Reg3) || (r_addr == Lux_Res_Reg4))		// Checking whether the passed register address is valid and writeable or not
		{
				Log_error(Lux, "Invalid Register Address Supplied (Lux Sensor)", ENOMSG, 1);
				return 1;
		}

		if(write_reg_cmd(&lux_data_write))		// Writing to command register first
		{
				Log_error(Lux, "write_reg_cmd", errno, 1);
				return 1;
		}
		if(lux_write_reg(&r_val, 1))		// Writing to the actual register
		{
				Log_error(Lux, "lux_write_reg", errno, 1);
				return 1;
		}
		return 0;
}

/*
*		Function to read from any internal register of Light Sensor
*		Returns 1 on failure, 0 on success
*		First Parameter is Internal Address of the Register
*		Second Parameter is the pointer to destination variable
*/
uint8_t custom_lux_reg_read(uint8_t r_addr, uint8_t *r_val)
{
		static uint8_t lux_data_write = 0;
		lux_data_write = Lux_Command_Reg_Data(r_addr);

		if(write_reg_cmd(&lux_data_write))		// Writing to command register
		{
				Log_error(Lux, "write_reg_cmd", errno, 1);
				return 1;
		}
		if(lux_read_reg(r_val, 1))		// Reading the data, and storing in the pointer through macro
		{
				Log_error(Lux, "lux_read_reg", errno, 1);
				return 1;
		}
		return 0;
}

/*
*		Function to test all settings of Temperature Sensor through manipulation of various registers
*		Returns 1 on failure, 0 on success
*		No Parameters
*/
uint8_t custom_test_lux_config(void)
{
		static uint8_t lux_reg_return;
		static char local_text[150];

		// Powering ON the Sensor by writing 0x03 to Control Register
		if(custom_lux_reg_write(Lux_Control_Reg, Lux_Control_Power_ON))
		{
				Log_error(Lux, "Write: Lux_Control_Reg", ENOMSG, 1);
				return 1;
		}
		// Reading back Control Register
		if(custom_lux_reg_read(Lux_Control_Reg, &lux_reg_return))
		{
				Log_error(Lux, "Read: Lux_Control_Reg", ENOMSG, 1);
				return 1;
		}
		// Verifying value 0x03
		if(Lux_Test_Control_Power(lux_reg_return))
		{
				sprintf(local_text, "Test: Power ON - Got %x Expected %x",(lux_reg_return & Lux_Control_Mask), Lux_Control_Power_ON);
				Log_error(Lux, local_text, ENOMSG, 1);
				return 1;
		}
		printf("Power ON Test Completed Successfully\n\n");

		// Setting High Gain and High Integration Time, and Verifying the same
		if(custom_lux_reg_write(Lux_Timing_Reg, Lux_Set_Gain_High(Lux_High_Integration_Time)))
		{
				Log_error(Lux, "Write: Lux_Timing_Reg", ENOMSG, 1);
				return 1;
		}
		// Reading back Timing Register
		if(custom_lux_reg_read(Lux_Timing_Reg, &lux_reg_return))
		{
				Log_error(Lux, "Read: Lux_Timing_Reg", ENOMSG, 1);
				return 1;
		}
		// Verifying values
		if((Lux_Test_High_Int_Time(lux_reg_return)) || (Lux_Test_Gain_High(lux_reg_return)))
		{
				sprintf(local_text, "Test: Gain and Integration Time - Got %x Expected %x",lux_reg_return, (Lux_High_Integration_Time | Lux_Gain_Mask));
				Log_error(Lux, local_text, ENOMSG, 1);
				return 1;
		}
		printf("Gain and Integration Time Test Completed Successfully\n\n");

		//Testing Interrupt Control Register with Test Data 0x0F
		if(custom_lux_reg_write(Lux_Intrp_Ctrl_Reg, Lux_Interrupt_Test_Data))
		{
				Log_error(Lux, "Write: Lux_Intrp_Ctrl_Reg", ENOMSG, 1);
				return 1;
		}
		// Reading back Interrupt Control Register
		if(custom_lux_reg_read(Lux_Intrp_Ctrl_Reg, &lux_reg_return))
		{
				Log_error(Lux, "Read: Lux_Intrp_Ctrl_Reg", ENOMSG, 1);
				return 1;
		}
		// Verifying values
		if(Lux_Test_Intrp_Ctrl_Data(lux_reg_return))
		{
				sprintf(local_text, "Test: Interrupt Control Register - Got %x Expected %x",(lux_reg_return & Lux_Interrupt_Control_Mask), Lux_Interrupt_Test_Data);
				Log_error(Lux, local_text, ENOMSG, 1);
				return 1;
		}
		printf("Interrupt Control Register Test Completed Successfully\n\n");
		// Reverting back
		if(custom_lux_reg_write(Lux_Intrp_Ctrl_Reg, 0))
		{
				Log_error(Lux, "Write: Lux_Intrp_Ctrl_Reg", ENOMSG, 1);
				return 1;
		}

		// Interrupt Thresholds - Low and High Test with predefined random data
		// Using Word Mode
		static uint8_t lux_data_write[2] = {0};

		// Writing TLow
		lux_data_write[0] = Lux_Command_Word_Data(Lux_ThrLow_Low_Reg);
		if(write_reg_cmd(&lux_data_write[0]))		// Writing to command register first
		{
				Log_error(Lux, "write_reg_cmd", errno, 1);
				return 1;
		}
		lux_data_write[0] = Lux_ThrLow_Low_Test_Data;
		lux_data_write[1] = Lux_ThrLow_High_Test_Data;
		if(lux_write_reg(&lux_data_write[0], 2))
		{
				Log_error(Lux, "lux_write_reg", errno, 1);
				return 1;
		}
		// Reading TLow
		lux_data_write[0] = Lux_Command_Word_Data(Lux_ThrLow_Low_Reg);
		if(write_reg_cmd(&lux_data_write[0]))
		{
				Log_error(Lux, "write_reg_cmd", errno, 1);
				return 1;
		}
		if(lux_read_reg(&lux_data_write[0], 2))
		{
				Log_error(Lux, "lux_read_reg", errno, 1);
				return 1;
		}
		// Verifying TLow
		if((Lux_Test_ThrLow_Low(lux_data_write[0])) || (Lux_Test_ThrLow_High(lux_data_write[1])))
		{
				sprintf(local_text, "Test: Interrupt Threshold TLow - Got %x & %x Expected %x & %x",lux_data_write[0], lux_data_write[1], Lux_ThrLow_Low_Test_Data, Lux_ThrLow_High_Test_Data);
				Log_error(Lux, local_text, ENOMSG, 1);
				return 1;
		}
		printf("Interrupt Threshold TLow Test Completed Successfully\n\n");

		// Writing THigh
		lux_data_write[0] = Lux_Command_Word_Data(Lux_ThrHigh_Low_Reg);
		if(write_reg_cmd(&lux_data_write[0]))
		{
				Log_error(Lux, "write_reg_cmd", errno, 1);
				return 1;
		}
		lux_data_write[0] = Lux_ThrHigh_Low_Test_Data;
		lux_data_write[1] = Lux_ThrHigh_High_Test_Data;
		if(lux_write_reg(&lux_data_write[0], 2))
		{
				Log_error(Lux, "lux_write_reg", errno, 1);
				return 1;
		}
		// Reading THigh
		lux_data_write[0] = Lux_Command_Word_Data(Lux_ThrHigh_Low_Reg);
		if(write_reg_cmd(&lux_data_write[0]))
		{
				Log_error(Lux, "write_reg_cmd", errno, 1);
				return 1;
		}
		if(lux_read_reg(&lux_data_write[0], 2))
		{
				Log_error(Lux, "lux_read_reg", errno, 1);
				return 1;
		}
		// Verifying THigh
		if((Lux_Test_ThrHigh_Low(lux_data_write[0])) || (Lux_Test_ThrHigh_High(lux_data_write[1])))
		{
				sprintf(local_text, "Test: Interrupt Threshold THigh - Got %x & %x Expected %x & %x",lux_data_write[0], lux_data_write[1], Lux_ThrHigh_Low_Test_Data, Lux_ThrHigh_High_Test_Data);
				Log_error(Lux, local_text, ENOMSG, 1);
				return 1;
		}
		printf("Interrupt Threshold THigh Test Completed Successfully\n\n");

		// Reading ID Register for testing part and revision number
		if(custom_lux_reg_read(Lux_ID_Reg, &lux_reg_return))
		{
				Log_error(Lux, "Read: Lux_ID_Reg", ENOMSG, 1);
				return 1;
		}
		// Verifying values (part number)
		if(Lux_Test_Part_No(lux_reg_return))
		{
				printf("\nLux ID Register Register Test Failed %x\n", lux_reg_return);
				sprintf(local_text, "Test: Interrupt Threshold THigh - Got %x Expected %x",((lux_reg_return & Lux_Part_No_Mask) >> Lux_Part_No_Pos), Lux_Part_No);
				Log_error(Lux, local_text, ENOMSG, 1);
				return 1;
		}
		printf("ID Register Test Succeeded\n\n");

		return 0;
}
#define I2C_BUS		(char *)"/dev/i2c-2"

/*
*		Function to initialize light sensor
*		Returns 1 on failure, 0 on success
*		No Parameters
*/
uint8_t custom_lux_init(void)
{
		lux_file_des = open(I2C_BUS, O_RDWR);
		if(lux_file_des == -1)
		{
				Log_error(Lux, "open(): I2C Bus", errno, 1);
				return 1;
		}
		if(ioctl(lux_file_des, I2C_SLAVE, Lux_Addr) == -1)
		{
				Log_error(Lux, "ioctl(): I2C Bus", errno, 1);
				return 1;
		}
		return 0;
}

/*
*		Function to read light level in lumens
*		Returns 1 on failure, 0 on success
*		First Parameter is the pointer to destination variable(float)
*/
uint8_t get_lux(float *l_data)
{
		static float lux_ch0 = 0;
		static float lux_ch1 = 0;
		static float ratio = 0;

		// Using Word Mode
		static uint8_t lux_data_write[2] = {0};

		// The following two steps - repeatedly turning on power, and setting gain & timing, aren't
		// absolutely necessary - but sometimes the sensors gives out 0 lux reading without these

		// Powering ON the Sensor by writing 0x03 to Control Register
		if(custom_lux_reg_write(Lux_Control_Reg, Lux_Control_Power_ON))
		{
				Log_error(Lux, "Write: Lux_Control_Reg", ENOMSG, 1);
				return 1;
		}

		// Setting High Gain and High Integration Time
		if(custom_lux_reg_write(Lux_Timing_Reg, Lux_Set_Gain_High(Lux_High_Integration_Time)))
		{
				Log_error(Lux, "Write: Lux_Timing_Reg", ENOMSG, 1);
				return 1;
		}

		// Reading Ch0
		lux_data_write[0] = Lux_Command_Word_Data(Lux_Data0_Low);
		if(write_reg_cmd(&lux_data_write[0]))
		{
				Log_error(Lux, "write_reg_cmd", errno, 1);
				return 1;
		}
		if(lux_read_reg(&lux_data_write[0], 2))
		{
				Log_error(Lux, "lux_read_reg", errno, 1);
				return 1;
		}
		lux_ch0 = (float)((lux_data_write[1] << 8) | lux_data_write[0]);

		// Reading Ch1
		lux_data_write[0] = Lux_Command_Word_Data(Lux_Data1_Low);
		if(write_reg_cmd(&lux_data_write[0]))
		{
				Log_error(Lux, "write_reg_cmd", errno, 1);
				return 1;
		}
		if(lux_read_reg(&lux_data_write[0], 2))
		{
				Log_error(Lux, "lux_read_reg", errno, 1);
				return 1;
		}
		lux_ch1 = (float)((lux_data_write[1] << 8) | lux_data_write[0]);

		ratio = (lux_ch1 / lux_ch0);

		// Calculation is Based on Datasheet
		if((ratio > 0) && (ratio <= 0.5))		*l_data = (0.0304 * lux_ch0) - (0.062 * lux_ch0 * pow(ratio, 1.4));
		else if((ratio > 0.5) && (ratio <= 0.61))		*l_data = (0.0224 * lux_ch0) - (0.031 * lux_ch1);
		else if((ratio > 0.61) && (ratio <= 0.80))		*l_data = (0.0128 * lux_ch0) - (0.0153 * lux_ch1);
		else if((ratio > 0.80) && (ratio <= 1.30))		*l_data = (0.00146 * lux_ch0) - (0.00112 * lux_ch1);
		else		*l_data = 0;

		// Checking whether it's day or night
		if(*l_data <= Lux_Night_Level)		Lux_Warning = Lux_State_Night;
		else			Lux_Warning = Lux_State_Day;

		return 0;
}



uint8_t LuxThread_Init(void)
{
		if(custom_lux_init() == 0)	printf("Lux Sensor Initiliazed Successfully\n\n");
		else
		{
				Log_error(Lux, "Lux Sensor Initialization... Exiting", ENOMSG, 1);
				return 1;
		}
		// BIST for Lux Sensor
		if(custom_test_lux_config() == 0)		printf("Lux Sensor Built-in-self-Test Passed Successfully\n\n");
		else
		{
				Log_error(Lux, "Lux Sensor Built-in-self-Test... Exiting", ENOMSG, 1);
				return 1;
		}

		printf("Starting Normal Operation\n\n");

		return 0;
}




int main(int argc, char *argv[])
{
	
	if(LuxThread_Init())
	{
		Log_error(Main,"Attempt to get the Lux Sensor Online Failed... Exiting LuxThread_Init()", errno, LOCAL_ONLY);
	}
	else
	{
		printf("Lux Sensor is Now Online...\n\n");
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