#include "TempThread.h"

#include "Global_Defines.h"
#include "My_Time.h"
#include "POSIX_Qs.h"

/*
*		File: TempThread.c
*		Purpose: To interface with Temperature Sensor (TMP102) on I2C bus, on BeagleBoneGreen, running custom linux kernel (platform by Buildroot)
*		Owner: Poorn Mehta
*		Date: 3/26/2019
*/

// This will indicate whether any valid user signal has been received or not
sig_atomic_t flag;

pthread_mutex_t lock;

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
		if(temp_write_reg(&r_arr))		// Writing to the actual register
		{
				perror("\nError in temp_write_reg\n");
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
		if(temp_read_reg(r_val))		// Reading the data, and storing in the pointer through macro
		{
				perror("\nError in temp_read_reg\n");
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
		if((temp_config_return != Temp_Config_Default_1) && (temp_config_return != Temp_Config_Default_2))
		{
				perror("\nDefault Temp Config Check Failed\n");
				printf("\nGot: %x\n", temp_config_return);
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
		temp_config_return = (temp_reg_return[0] << 8) | temp_reg_return[1];
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

// Here
void * TempThread(void * args)
{
			/* Init the Temp Thread */
			TempThread_Init();

			/* Create the Temp Thread POSIX queue */
			mqd_t MQ;											//Message queue descriptor

			/* Initialize the queue attributes */
			struct mq_attr attr;
			attr.mq_flags = O_NONBLOCK;									/* Flags: 0 or O_NONBLOCK */
			attr.mq_maxmsg = 10;								/* Max. # of messages on queue */
			attr.mq_msgsize = sizeof(MsgStruct);				/* Max. message size (bytes) */
			attr.mq_curmsgs = 0;								/* # of messages currently in queue */

			/* Create the Temp Thread queue to get messages from other pThreads */
			MQ = mq_open(TEMP_QUEUE, O_CREAT | O_RDONLY | O_NONBLOCK | O_CLOEXEC, 0666, &attr);
			if(MQ == (mqd_t) -1)
			{
					perror("!! ERROR in TempThread => mq_open()");
			}

			// Init Para
			uint8_t Temperature_Unit = Celsius;

			// Reception Structure from Socket
			MsgStruct MsgRecv;

			float Temperature_C;

			char Temperature_Text[60];

			uint8_t resp;

			while(1)
			{
					// Wait for signal
					while((flag == 0) || (flag == Lux_Signal));

					// If timer interrupt has passed signal, log cpu usage
					if(flag == Temperature_Signal)
					{
							flag = 0;
							pthread_mutex_lock(&lock);
							resp = get_temp(&Temperature_C);
							pthread_mutex_unlock(&lock);
							if(resp)
							{
									sprintf(Temperature_Text, "\nError while Reading Temperature\n");
									SendToThreadQ(Temp, Logging, "ERROR", Temperature_Text);
							}
							else
							{
									float Temperature_F = (Temperature_C * 1.8) + 32;
									float Temperature_K = Temperature_C + 273.15;

									if(Temperature_Unit == Celsius)		sprintf(Temperature_Text, "\nTemperature is *%f* in C\n", Temperature_C);
									else if(Temperature_Unit == Fahrenheit)		sprintf(Temperature_Text, "\nTemperature is *%f* in F\n", Temperature_F);
									else if(Temperature_Unit == Kelvin)	sprintf(Temperature_Text, "\nTemperature is *%f* in K\n", Temperature_K);

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
										else	printf("\n>>>>>>>>>Expected:%d Got:%d<<<<<<<<<<<\n", sizeof(MsgStruct), resp);
									}
							}
					}

					// In case of user signals, log and kill the Temperature Thread thread
					else if(flag == SIGUSR1 || flag == SIGUSR2)
					{
							// Notifying user
							printf("\nUser Signal Passed - Killing Temperature Thread\n");

							if(mq_unlink(TEMP_QUEUE) != 0)
							{
									perror("!! ERROR in Temperature Thread => mq_unlink()");
							}

							printf("\n--->>>Temperature Thread Exited<<<---\n");
							if(flag == SIGUSR1)	printf("Exit Reason: User Signal 1 Received (%d)\n", flag);
							else	printf("Exit Reason: User Signal 2 Received (%d)\n", flag);
//							flag = 1;

							// Immediately terminate the thread (unlike pthread_cancel)
							pthread_exit(0);

							// Break the infinite loop
							break;

					}
			}
}

///////////////////////////////////////////////////////////////////

// Gathering and printing useful information
/*
printf("\nThread ID of Temperature Thread: %ld", syscall(SYS_gettid));
printf("\nUse this for passing signal specifically to this thread\n");
printf("\n--->>> Temperature Thread Started<<<---\n");
printf("\nTemperature Thread Identities\nLinux Thread ID: *%ld* POSIX Thread ID: *%lu*\n", syscall(SYS_gettid), pthread_self());
*/


//////////////////////////////////////////////////////////////////

void TempThread_Init()
{
	char Text[60];

	pthread_mutex_lock(&lock);
	if(custom_temp_init() == 0)	sprintf(Text, "\nTemperature Sensor Initiliazed Successfully\n");
	else
	{
		printf("\nTemperature Sensor Initialization Failed\n");
		pthread_mutex_unlock(&lock);
		pthread_exit(0);
	}
	pthread_mutex_unlock(&lock);

	SendToThreadQ(Temp, Logging, "INFO", Text);
}
