/*
*	File: GPIO_PINs.h
*	Purpose: This header file has functions that handle interacting with the BeagleBone Ports and PINs
*	Owners: Poorn Mehta & Khalid AlAwadhi
*	Spring 2019
*/

#ifndef GPIO_PINS_H_
#define GPIO_PINS_H_

#include <stdint.h>
#include <stdbool.h>

/**************************************************************************************************************
 * USAGE: This function will set a chosen PIN as OUTPUT on the BeagleBone Black
 *
 * PARAMETERS:
 *            - uint8_t Port => Port number (GPIO'X')
 *            - uint8_t PIN => PIN number
 *
 * RETURNS: NONE
 *
 **************************************************************************************************************/
void Init_PIN_Output(uint8_t Port, uint8_t PIN);


/**************************************************************************************************************
 * USAGE: This function will set a chosen PIN value to a 1 or a 0 on the BeagleBone Black
 *
 * PARAMETERS:
 *            - uint8_t Port => Port number (GPIO'X')
 *            - uint8_t PIN => PIN number
 *            - bool Value => Set PIN to ON (1/TRUE) or OFF (0/FALSE)
 *
 * RETURNS: NONE
 *
 **************************************************************************************************************/
void PIN_Set_Value(uint8_t Port, uint8_t PIN, bool Value);


#endif /* GPIO_PINS_H_ */