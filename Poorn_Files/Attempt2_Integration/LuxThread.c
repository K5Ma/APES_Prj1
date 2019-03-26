#include "LuxThread.h"
#include "Global_Defines.h"
#include "POSIX_Qs.h"

/*
*		File: LuxThread.c
*		Purpose: To interface with Light Sensor (APDS9301) on I2C bus, on BeagleBoneGreen, running custom linux kernel (platform by Buildroot)
*		Owner: Poorn Mehta
*		Date: 3/26/2019
*/

// This will indicate whether any valid user signal has been received or not
sig_atomic_t flag;

pthread_mutex_t lock;

//Initializing global variable - this will contain I2C bus's file descriptor
int lux_file_des = -1;

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
				printf("\nInvalid Register Address Supplied (Lux Sensor)\n");
				return 1;
		}

		if(write_reg_cmd(&lux_data_write))		// Writing to command register first
		{
				printf("\nError in write_reg_cmd\n");
				return 1;
		}
		if(lux_write_reg(&r_val, 1))		// Writing to the actual register
		{
				printf("\nError in lux_write_reg\n");
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
				printf("\nError in write_reg_cmd\n");
				return 1;
		}
		if(lux_read_reg(r_val, 1))		// Reading the data, and storing in the pointer through macro
		{
				printf("\nError in lux_read_reg\n");
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

		// Powering ON the Sensor by writing 0x03 to Control Register
		if(custom_lux_reg_write(Lux_Control_Reg, Lux_Control_Power_ON))
		{
				printf("\nError in custom_lux_reg_write\n");
				return 1;
		}
		// Reading back Control Register
		if(custom_lux_reg_read(Lux_Control_Reg, &lux_reg_return))
		{
				printf("\nError in custom_lux_reg_read\n");
				return 1;
		}
		// Verifying value 0x03
		if(Lux_Test_Control_Power(lux_reg_return))
		{
				printf("\nLux Power ON Test Failed %x\n", lux_reg_return);
				return 1;
		}
		printf("\nLux Power ON Test Completed Successfully\n");

		// Setting High Gain and High Integration Time, and Verifying the same
		if(custom_lux_reg_write(Lux_Timing_Reg, Lux_Set_Gain_High(Lux_High_Integration_Time)))
		{
				printf("\nError in custom_lux_reg_write\n");
				return 1;
		}
		// Reading back Timing Register
		if(custom_lux_reg_read(Lux_Timing_Reg, &lux_reg_return))
		{
				printf("\nError in custom_lux_reg_read\n");
				return 1;
		}
		// Verifying values
		if((Lux_Test_High_Int_Time(lux_reg_return)) || (Lux_Test_Gain_High(lux_reg_return)))
		{
				printf("\nLux Gain and Integration Time Test Failed %x\n", lux_reg_return);
				return 1;
		}
		printf("\nLux Gain and Integration Time Test Completed Successfully\n");

		//Testing Interrupt Control Register with Test Data 0x0F
		if(custom_lux_reg_write(Lux_Intrp_Ctrl_Reg, Lux_Interrupt_Test_Data))
		{
				printf("\nError in custom_lux_reg_write\n");
				return 1;
		}
		// Reading back Interrupt Control Register
		if(custom_lux_reg_read(Lux_Intrp_Ctrl_Reg, &lux_reg_return))
		{
				printf("\nError in custom_lux_reg_read\n");
				return 1;
		}
		// Verifying values
		if(Lux_Test_Intrp_Ctrl_Data(lux_reg_return))
		{
				printf("\nLux Interrupt Control Register Test Failed %x\n", lux_reg_return);
				return 1;
		}
		printf("\nLux Interrupt Control Register Test Completed Successfully\n");
		// Reverting back
		if(custom_lux_reg_write(Lux_Intrp_Ctrl_Reg, 0))
		{
				printf("\nError in custom_lux_reg_write\n");
				return 1;
		}

		// Interrupt Thresholds - Low and High Test with predefined random data
		// Using Word Mode
		static uint8_t lux_data_write[2] = {0};

		// Writing TLow
		lux_data_write[0] = Lux_Command_Word_Data(Lux_ThrLow_Low_Reg);
		if(write_reg_cmd(&lux_data_write[0]))		// Writing to command register first
		{
				printf("\nError in write_reg_cmd\n");
				return 1;
		}
		lux_data_write[0] = Lux_ThrLow_Low_Test_Data;
		lux_data_write[1] = Lux_ThrLow_High_Test_Data;
		if(lux_write_reg(&lux_data_write[0], 2))
		{
				printf("\nError in lux_write_reg\n");
				return 1;
		}
		// Reading TLow
		lux_data_write[0] = Lux_Command_Word_Data(Lux_ThrLow_Low_Reg);
		if(write_reg_cmd(&lux_data_write[0]))
		{
				printf("\nError in write_reg_cmd\n");
				return 1;
		}
		if(lux_read_reg(&lux_data_write[0], 2))
		{
				printf("\nError in lux_read_reg\n");
				return 1;
		}
		// Verifying TLow
		if((Lux_Test_ThrLow_Low(lux_data_write[0]) == Lux_ThrLow_Low_Test_Data) || (Lux_Test_ThrLow_High(lux_data_write[1]) == Lux_ThrLow_High_Test_Data))
		{
				printf("\nLux Interrupt Threshold TLow Test failed\n");
				return 1;
		}
		printf("\nLux Interrupt Threshold TLow Test Completed Successfully\n");

		// Writing THigh
		lux_data_write[0] = Lux_Command_Word_Data(Lux_ThrHigh_Low_Reg);
		if(write_reg_cmd(&lux_data_write[0]))
		{
				printf("\nError in write_reg_cmd\n");
				return 1;
		}
		lux_data_write[0] = Lux_ThrHigh_Low_Test_Data;
		lux_data_write[1] = Lux_ThrHigh_High_Test_Data;
		if(lux_write_reg(&lux_data_write[0], 2))
		{
				printf("\nError in lux_write_reg\n");
				return 1;
		}
		// Reading THigh
		lux_data_write[0] = Lux_Command_Word_Data(Lux_ThrHigh_Low_Reg);
		if(write_reg_cmd(&lux_data_write[0]))
		{
				printf("\nError in write_reg_cmd\n");
				return 1;
		}
		if(lux_read_reg(&lux_data_write[0], 2))
		{
				printf("\nError in lux_read_reg\n");
				return 1;
		}
		// Verifying THigh
		if((Lux_Test_ThrHigh_Low(lux_data_write[0]) == Lux_ThrHigh_Low_Test_Data) || (Lux_Test_ThrHigh_High(lux_data_write[1]) == Lux_ThrHigh_High_Test_Data))
		{
				printf("\nLux Interrupt Threshold THigh Test failed\n");
				return 1;
		}
		printf("\nLux Interrupt Threshold THigh Test Completed Successfully\n");

		// Reading ID Register for testing part and revision number
		if(custom_lux_reg_read(Lux_ID_Reg, &lux_reg_return))
		{
				printf("\nError in custom_lux_reg_read\n");
				return 1;
		}
		// Verifying values (part number)
		if(Lux_Test_Part_No(lux_reg_return))
		{
				printf("\nLux ID Register Register Test Failed %x\n", lux_reg_return);
				return 1;
		}
		printf("\nLux ID Register Test Succeeded\n");

		return 0;
}

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
				printf("\nIn Lux - open() with I2C_BUS failed\n");
				return 1;
		}
		if(ioctl(lux_file_des, I2C_SLAVE, Lux_Addr) == -1)
		{
				printf("\nIn Lux - ioctl() with I2C_BUS failed\n");
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

		// Reading Ch0
		lux_data_write[0] = Lux_Command_Word_Data(Lux_Data0_Low);
		if(write_reg_cmd(&lux_data_write[0]))
		{
				printf("\nError in write_reg_cmd\n");
				return 1;
		}
		if(lux_read_reg(&lux_data_write[0], 2))
		{
				printf("\nError in lux_read_reg\n");
				return 1;
		}
		lux_ch0 = (float)((lux_data_write[1] << 8) | lux_data_write[0]);

		// Reading Ch1
		lux_data_write[0] = Lux_Command_Word_Data(Lux_Data1_Low);
		if(write_reg_cmd(&lux_data_write[0]))
		{
				printf("\nError in write_reg_cmd\n");
				return 1;
		}
		if(lux_read_reg(&lux_data_write[0], 2))
		{
				printf("\nError in lux_read_reg\n");
				return 1;
		}
		lux_ch1 = (float)((lux_data_write[1] << 8) | lux_data_write[0]);

		ratio = lux_ch1 / lux_ch0;

		// Calculation is Based on Datasheet
		if((ratio > 0) && (ratio <= 0.5))		*l_data = (0.0304 * lux_ch0) - (0.062 * lux_ch0 * pow(ratio, 1.4));
		else if((ratio > 0.5) && (ratio <= 0.61))		*l_data = (0.0224 * lux_ch0) - (0.031 * lux_ch1);
		else if((ratio > 0.61) && (ratio <= 0.80))		*l_data = (0.0128 * lux_ch0) - (0.0153 * lux_ch1);
		else if((ratio > 0.80) && (ratio <= 1.30))		*l_data = (0.00146 * lux_ch0) - (0.00112 * lux_ch1);
		else		*l_data = 0;

		return 0;
}


void * LuxThread(void * args)
{
			/* Init the Lux Thread */
			LuxThread_Init();

			/* Create the Lux Thread POSIX queue */
			mqd_t MQ;											//Message queue descriptor

			/* Initialize the queue attributes */
			struct mq_attr attr;
			attr.mq_flags = O_NONBLOCK;									/* Flags: 0 or O_NONBLOCK */
			attr.mq_maxmsg = 10;								/* Max. # of messages on queue */
			attr.mq_msgsize = sizeof(MsgStruct);				/* Max. message size (bytes) */
			attr.mq_curmsgs = 0;								/* # of messages currently in queue */

			/* Create the Lux Thread queue to get messages from other pThreads */
			MQ = mq_open(LUX_QUEUE, O_CREAT | O_RDONLY | O_NONBLOCK | O_CLOEXEC, 0666, &attr);
			if(MQ == (mqd_t) -1)
			{
					perror("!! ERROR in LuxThread => mq_open()");
			}

			// Reception Structure from Socket
			MsgStruct MsgRecv;

			float Lux_Value;

			uint8_t resp;

			char Lux_Text[60];

			while(1)
			{
					// Wait for signal
					while((flag == 0) || (flag == Temperature_Signal));

					// If timer interrupt has passed signal, log cpu usage
					if(flag == Lux_Signal)
					{
							flag = 0;
							pthread_mutex_lock(&lock);
							resp = get_lux(&Lux_Value);
							pthread_mutex_unlock(&lock);
							if(resp)
							{
									sprintf(Lux_Text, "\nError while Reading Temperature\n");
									SendToThreadQ(Lux, Logging, "ERROR", Lux_Text);
							}
							else
							{
									sprintf(Lux_Text, "\nLux is *%f*\n", Lux_Value);

									SendToThreadQ(Lux, Logging, "INFO", Lux_Text);

									// Check if there is a message from socket
									int resp = mq_receive(MQ, &MsgRecv, sizeof(MsgStruct), NULL);
									if(resp != -1)
									{
										if(resp == sizeof(MsgStruct))
										{
												if(strcmp("LX",MsgRecv.Msg) == 0)
												{
														sprintf(Lux_Text, "Lux is *%f*", Lux_Value);
												}
												SendToThreadQ(Lux, Socket, "INFO", Lux_Text);
										}
										else	printf("\n>>>>>>>>>Expected:%d Got:%d<<<<<<<<<<<\n", sizeof(MsgStruct), resp);
									}
							}
					}

					// In case of user signals, log and kill the Lux Thread thread
					else if(flag == SIGUSR1 || flag == SIGUSR2)
					{
							// Notifying user
							printf("\nUser Signal Passed - Killing Lux Thread\n");

							if(mq_unlink(LUX_QUEUE) != 0)
							{
									perror("!! ERROR in Lux Thread => mq_unlink()");
							}

							printf("\n--->>>Lux Thread Exited<<<---\n");
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



void LuxThread_Init()
{
		char Text[60];

		pthread_mutex_lock(&lock);
		if(custom_lux_init() == 0)	sprintf(Text, "\nLux Sensor Initiliazed Successfully\n");
		else
		{
				printf("\nLux Sensor Initialization Failed\n");
				pthread_mutex_unlock(&lock);
				pthread_exit(0);
		}
		pthread_mutex_unlock(&lock);

		SendToThreadQ(Lux, Logging, "INFO", Text);
}
