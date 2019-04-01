/*
*	File: My_Time.c
*	Purpose: The source file contains a simple function that gets the time
*	Owners: Poorn Mehta & Khalid AlAwadhi
*	Spring 2019
*/

#include "My_Time.h"
#include <sys/time.h>
#include <stdio.h>


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