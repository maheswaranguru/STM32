/*
 * led.h
 *
 *  Created on: 22-Mar-2019
 *      Author: gmahez
 */
//#include "sysConfigEsab.h"

#include <stdint.h>

#ifndef LED_LED_H_
#define LED_LED_H_

#define LED_ON      1
#define LED_OFF     0

typedef enum
{
	GREEN = 0, MAXIMUM_LED
} eLedName_t;

typedef struct data
{
	uint8_t vl2p 	:1;		//IC 13.2
	uint8_t vl3p 	:1;
	uint8_t vl4p 	:1;
	uint8_t vl5p 	:1;
	uint8_t vout 	:4;

	uint8_t vl5m 	:1;
	uint8_t vl4m 	:1;
	uint8_t vl3m 	:1;
	uint8_t vl2m 	:1;
	uint8_t vl1m 	:1;
	uint8_t vL0G 	:1;
	uint8_t vL0W 	:1;
	uint8_t l1p 	:1;

	uint8_t al2p 	:1;			//!C13.1
	uint8_t al3p 	:1;
	uint8_t al4p 	:1;
	uint8_t al5p 	:1;
	uint8_t aOut 	:4;

	uint8_t al5m 	:1;
	uint8_t al4m 	:1;
	uint8_t al3m 	:1;
	uint8_t al2m 	:1;
	uint8_t al1m 	:1;
	uint8_t aL0G 	:1;
	uint8_t aL0W 	:1;
	uint8_t al1p 	:1;
	//IC-8 and IC-9 panel top part Status indication
	uint32_t StatusIndLedsTop;
	//IC-6 and IC-7 panel top part Status indication
	uint32_t StatusIndLedsBottom;

	uint8_t digit1AmpsSpeed;
	uint8_t digit2AmpsSpeed;
	uint8_t digit3AmpsSpeed;

	uint8_t digit1Volts;
	uint8_t digit2Volts;
	uint8_t digit3Volts;


}ledStripData_t;

void ledStripVoltUpdate ( void );
void ledStripCurrentUpdate ( void );
void ledStripVoldLvlUpdate ( void );
void ledStripCurrentLvlUpdate ( void );
void ledStripWeldProcessUpdate ( void );
void ledStripTopPanelUpdate ( void );
void ledStripVoltUpdate ( void );
void ledStripVoltUpdate ( void );
void ledStripVoltUpdate ( void );
void ledStripVoltUpdate ( void );
void ledStripVoltUpdate ( void );
void ledStripVoltUpdate ( void );
void ledStripVoltUpdate ( void );
void ledStripVoltUpdate ( void );
void ledStripVoltUpdate ( void );
void ledStripVoltUpdate ( void );
void ledStripVoltUpdate ( void );


//!< extern functions from led
void ledTask(void const *argument);

#endif /* LED_LED_H_ */
