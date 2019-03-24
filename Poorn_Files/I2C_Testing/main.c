#include "main.h"
#include "LightSensor.h"
#include "TempSensor.h"
#include "LightSensor.c"
#include "TempSensor.c"

// Necessary variables for time stamps
struct timespec timespec_struct;
struct timeval current_time;

// This will indicate whether any valid user signal has been received or not
int flag = 0;

// Various Threads
pthread_t LightThread, TempThread;
pthread_mutex_t lock;

// Structure for passing file name. Can add other parameters for future use
struct thread_struct
{
	char *log_fname;
	char *message;
	int thread_id;
	float data;
};

void signal_function(int value)
{
	// Simply passes signal value to the main child thread 2 handler
	flag = value;
}

// For child thread 1
void *LightThread_function(void *thread_input)
{
	// Local file pointers
	static FILE *fptr;

	// Opening file for logging
	fptr = fopen(((struct thread_struct *)thread_input)->log_fname, "a");

	if(fptr)
	{
		printf("\nThread ID of Light Thread: %ld", syscall(SYS_gettid));
		printf("\nUse this for passing signal specifically to this thread\n");
		// Logging information
		gettimeofday(&current_time, 0);
		fprintf(fptr, "\n--->>> Light Thread Started at *%lu.%06lu* <<<---\n", current_time.tv_sec, current_time.tv_usec);
		fprintf(fptr, "Light Thread Identities\nLinux Thread ID: *%ld* POSIX Thread ID: *%lu*", syscall(SYS_gettid), pthread_self());
		fprintf(fptr, "\nTask: To process an input text file and display number of characters repetitions less than 100\n");

		pthread_mutex_lock(&lock);
		if(light_init() == 0)	fprintf(fptr, "Light Sensor Initiliazed Successfully\n");
		else
		{
			fprintf(fptr, "Light Sensor Initialization Failed\n");
			pthread_exit(0);
		}
		pthread_mutex_unlock(&lock);
	}

	// Handling failed attempt
	else
	{
		printf("\nError opening log file for Light Thread starting details\n");
		pthread_exit(0);
	}


	// Keep light thread running
	while(1)
	{
		// Wait for signal
		while(flag == 0);

		// If timer interrupt has passed signal, log cpu usage
		if(flag == SIGVTALRM)
		{
			// General message
			gettimeofday(&current_time, 0);

			pthread_mutex_lock(&lock);
			float lux = get_lux();
			pthread_mutex_unlock(&lock);

			//Instead of this fprintf, send message using SendToThread to Logger
			//Then check for message from socket
			//If there is a message, decode and send appropirate response
			fprintf(fptr, "Lux Level at *%lu.%06lu* is >>%f<<\n", current_time.tv_sec, current_time.tv_usec, lux);

			flag = 0;
		}

		// In case of user signals, log and kill the Temperature Thread thread
		else if(flag == SIGUSR1 || flag == SIGUSR2)
		{
			// Notifying user
			printf("\nUser Signal Passed - Killing Temperature Thread\n");

			gettimeofday(&current_time, 0);
			fprintf(fptr, "\n--->>> Temperature Thread Exited at *%lu.%06lu* <<<---\n", current_time.tv_sec, current_time.tv_usec);
			if(flag == SIGUSR1)	fprintf(fptr, "Exit Reason: User Signal 1 Received (%d)\n", flag);
			else	fprintf(fptr, "Exit Reason: User Signal 2 Received (%d)\n", flag);

			// Closing file
			fclose(fptr);

			// Immediately terminate the thread (unlike pthread_cancel)
			pthread_exit(0);

			// Break the infinite loop
			break;
		}
	}
}

// Child thread 2 function integrating CPU Usage functionality
// Prints CPU Usage details (copied from /proc/stat) and writes to cpu_stat_sourceon logger file
// Raises a flag and prints exit messages on receipt of user signal
void *TempThread_function(void *thread_input)
{
	// Local file pointer
	static FILE *fptr;

	// Opening logger file for starting information
	fptr = fopen(((struct thread_struct *)thread_input)->log_fname, "a");

	if(fptr)
	{
		// Gathering and printing useful information
		printf("\nThread ID of Temperature Thread: %ld", syscall(SYS_gettid));
		printf("\nUse this for passing signal specifically to this thread\n");
		gettimeofday(&current_time, 0);
		fprintf(fptr, "\n--->>> Temperature Thread Started at *%lu.%06lu* <<<---\n", current_time.tv_sec, current_time.tv_usec);
		fprintf(fptr, "\nTemperature Thread Identities\nLinux Thread ID: *%ld* POSIX Thread ID: *%lu*\n", syscall(SYS_gettid), pthread_self());

		pthread_mutex_lock(&lock);
		if(temp_init() == 0)	fprintf(fptr, "\nTemperature Sensor Initiliazed Successfully\n");
		else
		{
			fprintf(fptr, "\nTemperature Sensor Initialization Failed\n");
			pthread_exit(0);
		}

		if (write_tlow_reg(0x03,0x5551) < 0)
		{
			printf("write failed\n");
		}

		if (read_tlow_reg(0x03) < 0)
		{
			printf("read failed\n");
		}

			pthread_mutex_unlock(&lock);
		}

	// Handling failed attempt
	else
	{
		printf("\nError opening log file for Temperature Thread starting details\n");
		pthread_exit(0);
	}

	

	// Keep temperature thread running
	while(1)
	{
		// Wait for signal
		while(flag == 0);

		// If timer interrupt has passed signal, log cpu usage
		if(flag == SIGVTALRM)
		{

			// General message
			gettimeofday(&current_time, 0);

			pthread_mutex_lock(&lock);
			float temp = read_temp_data_reg(0);
			pthread_mutex_unlock(&lock);

			fprintf(fptr, "\nTemperature at *%lu.%06lu* is >>%f<<\n", current_time.tv_sec, current_time.tv_usec, temp);

			flag = 0;
		}

		// In case of user signals, log and kill the Temperature Thread thread
		else if(flag == SIGUSR1 || flag == SIGUSR2)
		{
			// Notifying user
			printf("\nUser Signal Passed - Killing Temperature Thread\n");

			gettimeofday(&current_time, 0);
			fprintf(fptr, "\n--->>> Temperature Thread Exited at *%lu.%06lu* <<<---\n", current_time.tv_sec, current_time.tv_usec);
			if(flag == SIGUSR1)	fprintf(fptr, "Exit Reason: User Signal 1 Received (%d)\n", flag);
			else	fprintf(fptr, "Exit Reason: User Signal 2 Received (%d)\n", flag);
			flag = 1;

			// Closing file pointer
			fclose(fptr);

			// Immediately terminate the thread (unlike pthread_cancel)
			pthread_exit(0);

			// Break the infinite loop
			break;

		}
	}
}

// Main, accepts arguments from command line
int main(int argc, char **argv)
{
	// Structure setting to be passed as argument to the thread
	struct thread_struct *tptr = (struct thread_struct *)malloc(sizeof(struct thread_struct));
	if(tptr)
	{
		// Local file pointer
		static FILE *fptr;

		// Get log_fname and add .txt after that
		if(argv[1] != 0)
		{
			tptr->log_fname = argv[1];
			strcat(tptr->log_fname,".txt");
		}
		else	tptr->log_fname = "logger.txt";

		// Print some useful information
		printf("\nProcess ID: %d Parent PID: %d\n", getpid(), getppid());

		// Log information of parent thread (main function in this case)
		clock_gettime(CLOCK_REALTIME, &timespec_struct);
		fptr = fopen(tptr->log_fname, "w+");

		if(fptr)
		{
			gettimeofday(&current_time, 0);
			fprintf(fptr, "--->>> Master Thread Created & Started at *%lu.%06lu* <<<---\n", current_time.tv_sec, current_time.tv_usec);
			fclose(fptr);
		}

		// Handling failed attempt
		else	printf("\nError opening log file for printing information about starting of main thread\n");

		if (pthread_mutex_init(&lock, NULL) != 0) 
		{ 
			printf("\n mutex init has failed\n"); 
			return -1; 
		}

		// Create and launch thread functions along with strcuture pointer as argument
	 	pthread_create(&LightThread, 0, LightThread_function, (void *)tptr);
	 	pthread_create(&TempThread, 0, TempThread_function, (void *)tptr);


		// Configuring timer and signal action
	  	struct sigaction custom_signal_action;
		struct itimerval custom_timer;

		// Set all initial values to 0 in the structure
		memset(&custom_signal_action, 0, sizeof (custom_signal_action));

		// Set signal action handler to point to the address of the target function (to execute on receiving signal)
		custom_signal_action.sa_handler = &signal_function;

		// Setting interval to 500ms
		custom_timer.it_interval.tv_sec = 0;
		custom_timer.it_interval.tv_usec = 500000;

		// Setting initial delay to 1s
		custom_timer.it_value.tv_sec = 1;
		custom_timer.it_value.tv_usec = 0;

		// Setting the signal action to kick in the handler function for these 3 signals
		sigaction (SIGVTALRM, &custom_signal_action, 0);
		sigaction (SIGUSR1, &custom_signal_action, 0);
		sigaction (SIGUSR2, &custom_signal_action, 0);

		// Starting timer
		setitimer (ITIMER_VIRTUAL, &custom_timer, 0);


		// Wait for threads to terminate
		pthread_join(LightThread, 0);
		pthread_join(TempThread, 0);

		// Opening log file to print information about exit of the main thread
		fptr = fopen(tptr->log_fname, "a");

		if(fptr)
		{
			gettimeofday(&current_time, 0);
			fprintf(fptr, "\n--->>> Master Thread Exited at *%lu.%06lu* <<<---\n", current_time.tv_sec, current_time.tv_usec);
			fclose(fptr);
		}

		// Handling failed attempt
		else	printf("\nError opening log file for printing information about exiting of main thread\n");
	}

	// Handling failed allocation
	else	printf("\nMalloc Failed...Exiting\n");
}

