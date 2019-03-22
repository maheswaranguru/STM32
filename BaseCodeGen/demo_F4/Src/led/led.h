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

//!< extern functions from led
void ledTask(void const * argument);

#endif /* LED_LED_H_ */
