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

#define MAX_1DIGIT	9
#define MAX_2DIGIT	99
#define MAX_3DIGIT	999


typedef enum
{
	GREEN = 0,
	RED,
	MAXIMUM_LED
} eLedName_t;

typedef struct data
{
	uint16_t dummyStatus1;		//IC 13.2
	uint16_t dummyStatus2;		//IC 13.2
	uint16_t dummyStatus3;		//IC 13.2

	uint8_t digit1Volts;
	uint8_t digit2Volts;
	uint8_t digit3Volts;

	uint8_t digit1AmpsSpeed;
	uint8_t digit2AmpsSpeed;
	uint8_t digit3AmpsSpeed;

}ledStripData_t;

void ledStripVoldLvlUpdate ( void );
void ledStripCurrentLvlUpdate ( void );
void ledStripWeldProcessUpdate ( void );
void ledStripTopPanelUpdate ( void );

bool updateVoltageDisplay( float displayVolt );
bool updateCurrentSpeedDisplay( float currentSpeed );

//!< extern functions from led
void ledTask(void const *argument);

#endif /* LED_LED_H_ */
