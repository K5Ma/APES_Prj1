/*
*		File: TempThread.c
*		Purpose: The source file containing functionalities and thread of Temperature Sensor(TMP102)
*		Owners: Poorn Mehta & Khalid AlAwadhi
*		Last Modified: 3/28/2019
*/

#include "TempThread.h"
//#include "Global_Defines.h"
#include "POSIX_Qs.h"

pthread_mutex_t lock;

sig_atomic_t flag;
uint8_t LogKillSafe;
uint8_t AliveThreads;

//Initializing global variable - this will contain I2C bus's file descriptor
int temp_file_des;

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
				Log_error(Temp, "Invalid Register Address Supplied (Temp Sensor)", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		if(write_reg_ptr(&r_addr))		// Writing to pointer register first
		{
				Log_error(Temp, "write_reg_ptr", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		if(temp_write_reg(&r_arr))		// Writing to the actual register
		{
				Log_error(Temp, "temp_write_reg", errno, LOGGING_AND_LOCAL);
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
				Log_error(Temp, "write_reg_ptr", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		if(temp_read_reg(r_val))		// Reading the data, and storing in the pointer through macro
		{
				Log_error(Temp, "temp_read_reg", errno, LOGGING_AND_LOCAL);
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
				Log_error(Temp, "Write: Temp_THigh_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		// Reading back THigh Register
		if(custom_temp_reg_read(Temp_THigh_Reg, &temp_reg_return[0]))
		{
				Log_error(Temp, "Read: Temp_THigh_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		// Verifying THigh Register
		if(temp_reg_return[0] != Temp_High_Threshold)
		{
					Log_error(Temp, "THigh Setup", ENOMSG, LOGGING_AND_LOCAL);
					return 1;
		}
		sprintf(local_text, "\nTHigh Set at %d deg C Successfully\n", Temp_High_Threshold);
		SendToThreadQ(Temp, Logging, "INFO", local_text);

		// Writing TLow Register
		if(custom_temp_reg_write(Temp_TLow_Reg, Temp_Low_Threshold << 8))
		{
				Log_error(Temp, "Write: Temp_TLow_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		// Reading back TLow Register
		if(custom_temp_reg_read(Temp_TLow_Reg, &temp_reg_return[0]))
		{
				Log_error(Temp, "Read: Temp_TLow_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		// Verifying TLow Register
		if(temp_reg_return[0] != Temp_Low_Threshold)
		{
				Log_error(Temp, "TLow Setup", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		sprintf(local_text, "\nTLow Set at %d deg C Successfully\n", Temp_Low_Threshold);
		SendToThreadQ(Temp, Logging, "INFO", local_text);

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
				Log_error(Temp, "Read: Temp_Config_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying default value
		if((temp_config_return != Temp_Config_Default_1) && (temp_config_return != Temp_Config_Default_2))
		{
				sprintf(local_text, "Test: Default Config - Got %x Expected %x or %x",temp_config_return, Temp_Config_Default_1, Temp_Config_Default_2);
				Log_error(Temp, local_text, ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		SendToThreadQ(Temp, Logging, "INFO", "\nDefault Temp Config Check Succeeded\n");

		//Setting Shutdown Mode to ON
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Shutdown_On))
		{
				Log_error(Temp, "Write: Temp_Config_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		//Reading Shutdown Mode Status
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				Log_error(Temp, "Read: Temp_Config_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying Shutdown Mode Setup
		if(Temp_Test_Shutdown_On(temp_config_return))
		{
				sprintf(local_text, "Test: Shutdown Mode - Got %x Expected 1",((temp_config_return & Temp_Shutdown_Mask) >> Temp_Shutdown_Pos));
				Log_error(Temp, local_text, ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}

		//Setting Shutdown Mode to OFF
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Shutdown_Off))
		{
				Log_error(Temp, "Write: Temp_Config_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		//Reading Shutdown Mode Status
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				Log_error(Temp, "Read: Temp_Config_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying Shutdown Mode Setup
		if(Temp_Test_Shutdown_Off(temp_config_return))
		{
				sprintf(local_text, "Test: Shutdown Mode - Got %x Expected 0",((temp_config_return & Temp_Shutdown_Mask) >> Temp_Shutdown_Pos));
				Log_error(Temp, local_text, ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
//		SendToThreadQ(Temp, Logging, "INFO", "\nShutdown Mode ON & OFF Test Succeeded\n");

		//Fault Bits - Checking both bits by setting them to 1
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Fault_Test))
		{
				Log_error(Temp, "Write: Temp_Config_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		//Reading Fault Bits Status
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				Log_error(Temp, "Read: Temp_Config_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying Fault Bits Setup
		if(Temp_Test_Fault(temp_config_return))
		{
				sprintf(local_text, "Test: Fault Bits - Got %x Expected 3",((temp_config_return & Temp_Fault_Mask) >> Temp_Fault_Pos));
				Log_error(Temp, local_text, ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		SendToThreadQ(Temp, Logging, "INFO", "\nFault Bits Test Succeeded\n");

		//Setting Extended Mode to ON
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Extended_Set))
		{
				Log_error(Temp, "Write: Temp_Config_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		//Reading Extended Mode Status
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				Log_error(Temp, "Read: Temp_Config_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying Extended Mode Setup
		if(Temp_Test_Extended_Set(temp_config_return))
		{
				sprintf(local_text, "Test: Extended Mode Set - Got %x Expected 1",((temp_config_return & Temp_Extended_Mask) >> Temp_Extended_Pos));
				Log_error(Temp, local_text, ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}

		//Setting Extended Mode to OFF
	if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Extended_Clear))
		{
				Log_error(Temp, "Write: Temp_Config_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		//Reading Extended Mode Status
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				Log_error(Temp, "Read: Temp_Config_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying Extended Mode Setup
		if(Temp_Test_Extended_Clear(temp_config_return))
		{
				sprintf(local_text, "Test: Extended Mode Clear - Got %x Expected 0",((temp_config_return & Temp_Extended_Mask) >> Temp_Extended_Pos));
				Log_error(Temp, local_text, ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		SendToThreadQ(Temp, Logging, "INFO", "\nExtended Mode Set & Clear Test Succeeded\n");

		//Setting up Conversion Rate
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Write_Conversion_Test))
		{
				Log_error(Temp, "Write: Temp_Config_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		//Reading Conversion Rate
		if(custom_temp_reg_read(Temp_Config_Reg, &temp_reg_return[0]))
		{
				Log_error(Temp, "Read: Temp_Config_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
		//Verifying Conversion Rate Setup
		if(Temp_Test_Conversion(temp_config_return))
		{
				sprintf(local_text, "Test: Conversion Rate - Got %x Expected 0",((temp_config_return & Temp_Conversion_Mask) >> Temp_Conversion_Pos));
				Log_error(Temp, local_text, ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		SendToThreadQ(Temp, Logging, "INFO", "\nConversion Rate Test Succeeded\n");

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
				Log_error(Temp, "Read: Temp_Data_Reg", ENOMSG, LOGGING_AND_LOCAL);
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
				Log_error(Temp, "open(): I2C Bus", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		if(ioctl(temp_file_des, I2C_SLAVE, Temp_Addr) == -1)
		{
				Log_error(Temp, "ioctl(): I2C Bus", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		return 0;
}

void * TempThread(void * args)
{
	/* Init the Temp Thread */
	uint8_t resp;
	resp = TempThread_Init();
	if(resp)
	{
			Log_error(Temp, "Error while Initializing Temperature Sensor", ENOMSG, LOGGING_AND_LOCAL);
			Temp_Error_Retry = Temp_Max_Retries;
			Temp_Sensor_State = Sensor_Offline;
	}
	else
	{
			Temp_Error_Retry = Temp_No_Retry;
			Temp_Sensor_State = Sensor_Online;
	}

	/* Create the Temp Thread POSIX queue */
	mqd_t MQ;											//Message queue descriptor

	/* Initialize the queue attributes */
	struct mq_attr attr;
	attr.mq_flags = O_NONBLOCK;							/* Flags: 0 or O_NONBLOCK */
	attr.mq_maxmsg = 10;								/* Max. # of messages on queue */
	attr.mq_msgsize = sizeof(MsgStruct);				/* Max. message size (bytes) */
	attr.mq_curmsgs = 0;								/* # of messages currently in queue */


	/* Create the Temp Thread queue to get messages from other pThreads */
	MQ = mq_open(TEMP_QUEUE, O_CREAT | O_RDONLY | O_NONBLOCK | O_CLOEXEC, 0666, &attr);
	if(MQ == (mqd_t) -1)
	{
		Log_error(Temp, "mq_open()", errno, LOGGING_AND_LOCAL);
	}

	// Init Para
	uint8_t Temperature_Unit = Celsius;
	// Reception Structure from Socket
	MsgStruct MsgRecv;
	// Variable to store temperature
	float Temperature_C = 0;
	// Static variable to contain error messages
	static char local_text[150];

	char Temperature_Text[150];

	while(1)
	{
		/* Set alive bit */
		pthread_mutex_lock(&lock_var);
		AliveThreads |= TEMP_ALIVE;
		pthread_mutex_unlock(&lock_var);

		if((flag == Temperature_Signal) && (Temp_Sensor_State == Sensor_Online))
		{
				flag = 0;
				pthread_mutex_lock(&lock);
				resp = get_temp(&Temperature_C);
				pthread_mutex_unlock(&lock);

				if(resp)
				{
						Log_error(Temp, "Error while Reading Temperature", ENOMSG, LOGGING_AND_LOCAL);
						Temp_Error_Retry = Temp_Max_Retries;
						Temp_Sensor_State = Sensor_Offline;
				}
				else
				{
						float Temperature_F = (Temperature_C * 1.8) + 32;
						float Temperature_K = Temperature_C + 273.15;

						if(Temperature_Unit == Celsius)		sprintf(Temperature_Text, "Temperature is *%f* in C", Temperature_C);
						else if(Temperature_Unit == Fahrenheit)		sprintf(Temperature_Text, "Temperature is *%f* in F", Temperature_F);
						else if(Temperature_Unit == Kelvin)		sprintf(Temperature_Text, "Temperature is *%f* in K", Temperature_K);

						SendToThreadQ(Temp, Logging, "INFO", Temperature_Text);

						// Check if there is a message from socket
						int resp = mq_receive(MQ, &MsgRecv, sizeof(MsgStruct), NULL);
						if(resp != -1)
						{
								if(resp == sizeof(MsgStruct))
								{
										if(strcmp("TC",MsgRecv.Msg) == 0)
										{
												Temperature_Unit = Celsius;
												sprintf(Temperature_Text, "Temperature is *%f* in C", Temperature_C);
										}
										else if(strcmp("TF",MsgRecv.Msg) == 0)
										{
												Temperature_Unit = Fahrenheit;
												sprintf(Temperature_Text, "Temperature is *%f* in F", Temperature_F);
										}
										else if(strcmp("TK",MsgRecv.Msg) == 0)
										{
												Temperature_Unit = Kelvin;
												sprintf(Temperature_Text, "Temperature is *%f* in K", Temperature_K);
										}
										SendToThreadQ(Temp, Socket, "INFO", Temperature_Text);
								}
								else
								{
										sprintf(local_text, "From Socket Thread: Got %d Bytes Expected %d Bytes", resp, sizeof(MsgStruct));
										Log_error(Temp, local_text, ENOMSG, LOGGING_AND_LOCAL);
								}
						}
				}
		}

		/* Check for KILL signals */
		else if((flag == SIGUSR1) || (flag == SIGUSR2) || ((Temp_Sensor_State == Sensor_Offline) && (Temp_Error_Retry == Temp_No_Retry)))
		{
			if((flag == SIGUSR1) || (flag == SIGUSR2))		SendToThreadQ(Temp, Logging, "INFO", "User Signal Passed - Killing Temperature Thread");
			else		Log_error(Temp,"All Attempts to get the Temperature Sensor Online Failed... Killing Temperature Thread", ENOMSG, LOGGING_AND_LOCAL);

			if(mq_unlink(TEMP_QUEUE) != 0)
			{
				Log_error(Temp, "mq_unlink()", errno, LOGGING_AND_LOCAL);
			}
			else
			{
				SendToThreadQ(Temp, Logging, "INFO", "Successfully unlinked Temp queue!");
			}

			char TempTxt[150];
			if(flag == SIGUSR1)
			{
				sprintf(TempTxt, "Exit Reason: User Signal 1 Received (%d)", flag);
				SendToThreadQ(Temp, Logging, "INFO", TempTxt);
			}
			else if(flag == SIGUSR2)
			{
				sprintf(TempTxt, "Exit Reason: User Signal 2 Received (%d)", flag);
				SendToThreadQ(Temp, Logging, "INFO", TempTxt);
			}

			/* Decrement the LogKillSafe and clear the alive bit */
			pthread_mutex_lock(&lock_var);
			LogKillSafe--;
			AliveThreads &= ~TEMP_ALIVE;
			pthread_mutex_unlock(&lock_var);

			SendToThreadQ(Temp, Logging, "INFO", "Temp Thread has terminated successfully and will now exit");


			return 0;
		}
	}
}



uint8_t TempThread_Init(void)
{
		char Text[60];

		sprintf(Text, "Temp Thread successfully created! TID: %ld", syscall(SYS_gettid));
		SendToThreadQ(Temp, Logging, "INFO", Text);

		pthread_mutex_lock(&lock);
		if(custom_temp_init() == 0)		SendToThreadQ(Temp, Logging, "INFO", "\nTemperature Sensor Initiliazed Successfully\n");
		else
		{
				Log_error(Temp, "Temperature Sensor Initialization... Exiting", ENOMSG, LOGGING_AND_LOCAL);
				pthread_mutex_unlock(&lock);
				return 1;
		}

		// Resetting Config (To prevent shutdown from previously failed bootup)
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Config_Default_1) == 0)			SendToThreadQ(Temp, Logging, "INFO", "\nTemperature Sensor Resetted Successfully\n");
		else
		{
				Log_error(Temp, "Temperature Sensor Reset... Exiting", ENOMSG, LOGGING_AND_LOCAL);
				pthread_mutex_unlock(&lock);
				return 1;
		}

		// Setting thresholds
		if(custom_set_temp_thresholds() == 0)		SendToThreadQ(Temp, Logging, "INFO", "\nTemperature Sensor Thresholds Set Successfully\n");
		else
		{
				Log_error(Temp, "Temperature Sensor Thresholds... Exiting", ENOMSG, LOGGING_AND_LOCAL);
				pthread_mutex_unlock(&lock);
				return 1;
		}

		// BIST for Temp Sensor
		if(custom_test_temp_config() == 0)		SendToThreadQ(Temp, Logging, "INFO", "\nTemperature Sensor Built-in-self-Test Passed Successfully\n");
		else
		{
				Log_error(Temp, "Temperature Sensor Built-in-self-Test... Exiting", ENOMSG, LOGGING_AND_LOCAL);
				pthread_mutex_unlock(&lock);
				return 1;
		}

		// Resetting Config
		if(custom_temp_reg_write(Temp_Config_Reg, Temp_Config_Default_1) == 0)			SendToThreadQ(Temp, Logging, "INFO", "\nTemperature Sensor Resetted Successfully\n");
		else
		{
				Log_error(Temp, "Temperature Sensor Reset... Exiting", ENOMSG, LOGGING_AND_LOCAL);
				pthread_mutex_unlock(&lock);
				return 1;
		}

		pthread_mutex_unlock(&lock);

		SendToThreadQ(Temp, Logging, "INFO", "\nStarting Normal Operation\n");

		return 0;
}
