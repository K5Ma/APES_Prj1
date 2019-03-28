#include "LuxThread.h"
#include "Global_Defines.h"
#include "POSIX_Qs.h"

// This will indicate whether any valid user signal has been received or not
sig_atomic_t flag;

pthread_mutex_t lock;

uint8_t Lux_Warning = Lux_State_Day;

//Initializing global variable - this will contain I2C bus's file descriptor
int lux_file_des;

uint8_t Lux_Error_Retry;

uint8_t Lux_Sensor_State;

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
				Log_error(Lux, "Invalid Register Address Supplied (Lux Sensor)", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}

		if(write_reg_cmd(&lux_data_write))		// Writing to command register first
		{
				Log_error(Lux, "write_reg_cmd", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		if(lux_write_reg(&r_val, 1))		// Writing to the actual register
		{
				Log_error(Lux, "lux_write_reg", errno, LOGGING_AND_LOCAL);
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
				Log_error(Lux, "write_reg_cmd", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		if(lux_read_reg(r_val, 1))		// Reading the data, and storing in the pointer through macro
		{
				Log_error(Lux, "lux_read_reg", errno, LOGGING_AND_LOCAL);
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
				Log_error(Lux, "Write: Lux_Control_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		// Reading back Control Register
		if(custom_lux_reg_read(Lux_Control_Reg, &lux_reg_return))
		{
				Log_error(Lux, "Read: Lux_Control_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		// Verifying value 0x03
		if(Lux_Test_Control_Power(lux_reg_return))
		{
				sprintf(local_text, "Test: Power ON - Got %x Expected %x",(lux_reg_return & Lux_Control_Mask), Lux_Control_Power_ON);
				Log_error(Lux, local_text, ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		SendToThreadQ(Lux, Logging, "INFO", "\nPower ON Test Completed Successfully\n");

		// Setting High Gain and High Integration Time, and Verifying the same
		if(custom_lux_reg_write(Lux_Timing_Reg, Lux_Set_Gain_High(Lux_High_Integration_Time)))
		{
				Log_error(Lux, "Write: Lux_Timing_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		// Reading back Timing Register
		if(custom_lux_reg_read(Lux_Timing_Reg, &lux_reg_return))
		{
				Log_error(Lux, "Read: Lux_Timing_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		// Verifying values
		if((Lux_Test_High_Int_Time(lux_reg_return)) || (Lux_Test_Gain_High(lux_reg_return)))
		{
				sprintf(local_text, "Test: Gain and Integration Time - Got %x Expected %x",lux_reg_return, (Lux_High_Integration_Time | Lux_Gain_Mask));
				Log_error(Lux, local_text, ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		SendToThreadQ(Lux, Logging, "INFO", "\nGain and Integration Time Test Completed Successfully\n");

		//Testing Interrupt Control Register with Test Data 0x0F
		if(custom_lux_reg_write(Lux_Intrp_Ctrl_Reg, Lux_Interrupt_Test_Data))
		{
				Log_error(Lux, "Write: Lux_Intrp_Ctrl_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		// Reading back Interrupt Control Register
		if(custom_lux_reg_read(Lux_Intrp_Ctrl_Reg, &lux_reg_return))
		{
				Log_error(Lux, "Read: Lux_Intrp_Ctrl_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		// Verifying values
		if(Lux_Test_Intrp_Ctrl_Data(lux_reg_return))
		{
				sprintf(local_text, "Test: Interrupt Control Register - Got %x Expected %x",(lux_reg_return & Lux_Interrupt_Control_Mask), Lux_Interrupt_Test_Data);
				Log_error(Lux, local_text, ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		SendToThreadQ(Lux, Logging, "INFO", "\nInterrupt Control Register Test Completed Successfully\n");
		// Reverting back
		if(custom_lux_reg_write(Lux_Intrp_Ctrl_Reg, 0))
		{
				Log_error(Lux, "Write: Lux_Intrp_Ctrl_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}

		// Interrupt Thresholds - Low and High Test with predefined random data
		// Using Word Mode
		static uint8_t lux_data_write[2] = {0};

		// Writing TLow
		lux_data_write[0] = Lux_Command_Word_Data(Lux_ThrLow_Low_Reg);
		if(write_reg_cmd(&lux_data_write[0]))		// Writing to command register first
		{
				Log_error(Lux, "write_reg_cmd", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		lux_data_write[0] = Lux_ThrLow_Low_Test_Data;
		lux_data_write[1] = Lux_ThrLow_High_Test_Data;
		if(lux_write_reg(&lux_data_write[0], 2))
		{
				Log_error(Lux, "lux_write_reg", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		// Reading TLow
		lux_data_write[0] = Lux_Command_Word_Data(Lux_ThrLow_Low_Reg);
		if(write_reg_cmd(&lux_data_write[0]))
		{
				Log_error(Lux, "write_reg_cmd", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		if(lux_read_reg(&lux_data_write[0], 2))
		{
				Log_error(Lux, "lux_read_reg", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		// Verifying TLow
		if((Lux_Test_ThrLow_Low(lux_data_write[0])) || (Lux_Test_ThrLow_High(lux_data_write[1])))
		{
				sprintf(local_text, "Test: Interrupt Threshold TLow - Got %x & %x Expected %x & %x",lux_data_write[0], lux_data_write[1], Lux_ThrLow_Low_Test_Data, Lux_ThrLow_High_Test_Data);
				Log_error(Lux, local_text, ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		SendToThreadQ(Lux, Logging, "INFO", "\nInterrupt Threshold TLow Test Completed Successfully\n");

		// Writing THigh
		lux_data_write[0] = Lux_Command_Word_Data(Lux_ThrHigh_Low_Reg);
		if(write_reg_cmd(&lux_data_write[0]))
		{
				Log_error(Lux, "write_reg_cmd", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		lux_data_write[0] = Lux_ThrHigh_Low_Test_Data;
		lux_data_write[1] = Lux_ThrHigh_High_Test_Data;
		if(lux_write_reg(&lux_data_write[0], 2))
		{
				Log_error(Lux, "lux_write_reg", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		// Reading THigh
		lux_data_write[0] = Lux_Command_Word_Data(Lux_ThrHigh_Low_Reg);
		if(write_reg_cmd(&lux_data_write[0]))
		{
				Log_error(Lux, "write_reg_cmd", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		if(lux_read_reg(&lux_data_write[0], 2))
		{
				Log_error(Lux, "lux_read_reg", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		// Verifying THigh
		if((Lux_Test_ThrHigh_Low(lux_data_write[0])) || (Lux_Test_ThrHigh_High(lux_data_write[1])))
		{
				sprintf(local_text, "Test: Interrupt Threshold THigh - Got %x & %x Expected %x & %x",lux_data_write[0], lux_data_write[1], Lux_ThrHigh_Low_Test_Data, Lux_ThrHigh_High_Test_Data);
				Log_error(Lux, local_text, ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		SendToThreadQ(Lux, Logging, "INFO", "\nInterrupt Threshold THigh Test Completed Successfully\n");

		// Reading ID Register for testing part and revision number
		if(custom_lux_reg_read(Lux_ID_Reg, &lux_reg_return))
		{
				Log_error(Lux, "Read: Lux_ID_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		// Verifying values (part number)
		if(Lux_Test_Part_No(lux_reg_return))
		{
				printf("\nLux ID Register Register Test Failed %x\n", lux_reg_return);
				sprintf(local_text, "Test: Interrupt Threshold THigh - Got %x Expected %x",((lux_reg_return & Lux_Part_No_Mask) >> Lux_Part_No_Pos), Lux_Part_No);
				Log_error(Lux, local_text, ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}
		SendToThreadQ(Lux, Logging, "INFO", "\nID Register Test Succeeded\n");

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
				Log_error(Lux, "open(): I2C Bus", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		if(ioctl(lux_file_des, I2C_SLAVE, Lux_Addr) == -1)
		{
				Log_error(Lux, "ioctl(): I2C Bus", errno, LOGGING_AND_LOCAL);
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
				Log_error(Lux, "Write: Lux_Control_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}

		// Setting High Gain and High Integration Time
		if(custom_lux_reg_write(Lux_Timing_Reg, Lux_Set_Gain_High(Lux_High_Integration_Time)))
		{
				Log_error(Lux, "Write: Lux_Timing_Reg", ENOMSG, LOGGING_AND_LOCAL);
				return 1;
		}

		// Reading Ch0
		lux_data_write[0] = Lux_Command_Word_Data(Lux_Data0_Low);
		if(write_reg_cmd(&lux_data_write[0]))
		{
				Log_error(Lux, "write_reg_cmd", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		if(lux_read_reg(&lux_data_write[0], 2))
		{
				Log_error(Lux, "lux_read_reg", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		lux_ch0 = (float)((lux_data_write[1] << 8) | lux_data_write[0]);

		// Reading Ch1
		lux_data_write[0] = Lux_Command_Word_Data(Lux_Data1_Low);
		if(write_reg_cmd(&lux_data_write[0]))
		{
				Log_error(Lux, "write_reg_cmd", errno, LOGGING_AND_LOCAL);
				return 1;
		}
		if(lux_read_reg(&lux_data_write[0], 2))
		{
				Log_error(Lux, "lux_read_reg", errno, LOGGING_AND_LOCAL);
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


void * LuxThread(void * args)
{
		/* Init the Lux Thread */
		if(LuxThread_Init())
		{
			Log_error(Lux, "Error while Initializing Lux Sensor", ENOMSG, LOGGING_AND_LOCAL);
			Lux_Error_Retry = Lux_Max_Retries;
			Lux_Sensor_State = Sensor_Offline;
		}
		else
		{
			Lux_Error_Retry = Lux_No_Retry;
			Lux_Sensor_State = Sensor_Online;
		}

		/* Create the Lux Thread POSIX queue */
		mqd_t MQ;											//Message queue descriptor

		/* Initialize the queue attributes */
		struct mq_attr attr;
		attr.mq_flags = O_NONBLOCK;							/* Flags: 0 or O_NONBLOCK */
		attr.mq_maxmsg = 10;								/* Max. # of messages on queue */
		attr.mq_msgsize = sizeof(MsgStruct);				/* Max. message size (bytes) */
		attr.mq_curmsgs = 0;								/* # of messages currently in queue */

		/* Create the Lux Thread queue to get messages from other pThreads */
		MQ = mq_open(LUX_QUEUE, O_CREAT | O_RDONLY | O_NONBLOCK | O_CLOEXEC, 0666, &attr);
		if(MQ == (mqd_t) -1)
		{
			Log_error(Lux, "mq_open()", errno, LOGGING_AND_LOCAL);
		}


		// Reception Structure from Socket
		MsgStruct MsgRecv;
		// Variable to store lux level
		float Lux_Value = 0;
		// Static variable to contain error messages
		static local_text[150];

		char Lux_Text[150];
		uint8_t resp;


		while(1)
		{
				// Wait for signal
				while((flag == 0) || (flag == Temperature_Signal));

				if((flag == Lux_Signal) && (Lux_Sensor_State == Sensor_Online))
				{
						flag = 0;
						pthread_mutex_lock(&lock);
						resp = get_lux(&Lux_Value);
						pthread_mutex_unlock(&lock);

						if(resp)
						{
								Log_error(Lux, "\nError while Reading Lux\n", ENOMSG, LOGGING_AND_LOCAL);
								Lux_Error_Retry = Lux_Max_Retries;
								Lux_Sensor_State = Sensor_Offline;
						}
						else
						{
							sprintf(Lux_Text, "Lux is *%f*", Lux_Value);
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
									else
									{
											sprintf(local_text, "From Socket Thread: Got %d Bytes Expected %d Bytes", resp, sizeof(MsgStruct));
											Log_error(Lux, local_text, ENOMSG, LOGGING_AND_LOCAL);
									}
							}
						}
				}

				// In case of user signals, log and kill the Lux Thread thread
				else if((flag == SIGUSR1) || (flag == SIGUSR2) || ((Lux_Sensor_State == Sensor_Offline) && (Lux_Error_Retry == Lux_No_Retry)))
				{
						// Notifying user
						if((flag == SIGUSR1) || (flag == SIGUSR2))		SendToThreadQ(Lux, Logging, "INFO", "User Signal Passed - Killing Lux Thread");
						else		Log_error(Lux,"All Attempts to get the Lux Sensor Online Failed... Killing Lux Thread", ENOMSG, LOGGING_AND_LOCAL);

						if(mq_unlink(LUX_QUEUE) != 0)			Log_error(Lux, "mq_unlink()", errno, LOGGING_AND_LOCAL);

						SendToThreadQ(Lux, Logging, "INFO", "--->>>Lux Thread Exited<<<---");
						if(flag == SIGUSR1)
						{
								//printf("Exit Reason: User Signal 1 Received (%d)\n", flag);
								sprintf(Lux_Text, "Exit Reason: User Signal 1 Received (%d)", flag);
								SendToThreadQ(Lux, Logging, "INFO", Lux_Text);
						}
						else
						{
								//printf("Exit Reason: User Signal 2 Received (%d)\n", flag);
								sprintf(Lux_Text, "Exit Reason: User Signal 2 Received (%d)", flag);
								SendToThreadQ(Lux, Logging, "INFO", Lux_Text);
						}

						// Immediately terminate the thread (unlike pthread_cancel)
						pthread_exit(0);

						// Break the infinite loop
						break;
				}
		}
		printf("DEBUG: LUX PTHREAD HAS FINISHED AND WILL EXIT\n");
}



uint8_t LuxThread_Init(void)
{
		char Text[60];

		sprintf(Text, "Lux Thread successfully created! TID: %ld", syscall(SYS_gettid));
		SendToThreadQ(Lux, Logging, "INFO", Text);

		pthread_mutex_lock(&lock);
		if(custom_lux_init() == 0)	SendToThreadQ(Lux, Logging, "INFO", "\nLux Sensor Initiliazed Successfully\n");
		else
		{
				Log_error(Lux, "Lux Sensor Initialization... Exiting", ENOMSG, LOGGING_AND_LOCAL);
				pthread_mutex_unlock(&lock);
				return 1;
		}

		// BIST for Lux Sensor
		if(custom_test_lux_config() == 0)		SendToThreadQ(Lux, Logging, "INFO", "\nLux Sensor Built-in-self-Test Passed Successfully\n");
		else
		{
				Log_error(Lux, "Lux Sensor Built-in-self-Test... Exiting", ENOMSG, LOGGING_AND_LOCAL);
				pthread_mutex_unlock(&lock);
				return 1;
		}

		pthread_mutex_unlock(&lock);

		SendToThreadQ(Lux, Logging, "INFO", "\nStarting Normal Operation\n");

		return 0;
}
