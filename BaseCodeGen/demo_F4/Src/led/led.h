/*
 * led.h
 *
 *  Created on: 22-Mar-2019
 *      Author: gmahez
 */

#ifndef LED_LED_H_
#define LED_LED_H_

#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

typedef enum
{
    LED_STATE_RED1,
    LED_STATE_RED2,
    LED_STATE_BLUE,
    LED_STATE_GREEN
} ledStat_t;

//!< extern functions from led
void ledTask(void const * argument);

#endif /* LED_LED_H_ */
