/*
*	File: GPIO_PINs.c
*	Purpose: This source file has functions that handle interacting with the BeagleBone Ports and PINs
*	Owners: Poorn Mehta & Khalid AlAwadhi
*	Spring 2019
*/

#include "GPIO_PINs.h"

#include <stdio.h>
#include <stdlib.h>


void Init_PIN_Output(uint8_t Port, uint8_t PIN)
{
	
	/* Calculate the absolute PIN number */
	uint8_t Abs_PIN = (32*(Port)) + (PIN);
	
	/* Executes terminal command within the program in order to access the 
	 * BB PINs as the way to interact with them is through files */
	/* Generate CMD */
	char TerminalCMD[100];
	sprintf(TerminalCMD, "cd ~/../sys/class/gpio && echo %u > export", Abs_PIN);

	system(TerminalCMD);
}



void PIN_Set_Value(uint8_t Port, uint8_t PIN, bool Value)
{
	/* Calculate the absolute PIN number */
	uint8_t Abs_PIN = (32*(Port)) + (PIN);
	
	/* Executes terminal command within the program in order to access the 
	 * BB PINs as the way to interact with them is through files */
	/* Generate CMDs */
	char TerminalCMD[100];
	
	sprintf(TerminalCMD, "cd ~/../sys/class/gpio/gpio%u && echo out > direction", Abs_PIN);
	system(TerminalCMD);
	
	sprintf(TerminalCMD, "cd ~/../sys/class/gpio/gpio%u  && echo %d > value", Abs_PIN, Value);
	system(TerminalCMD);
}