/*
 * led.h
 *
 *  Created on: 22-Mar-2019
 *      Author: gmahez
 */

#ifndef LED_LED_H_
#define LED_LED_H_

#define LED_ON      1
#define LED_OFF     0

typedef enum
{
    GREEN= 0,
    ORANGE,
    RED,
    BLUE,
    MAXIMUM_LED
}eLedName_t;


//!< extern functions from led
void ledTask(void const * argument);

#endif /* LED_LED_H_ */
