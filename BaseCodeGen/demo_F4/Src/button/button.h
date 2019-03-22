/*
 * button.h
 *
 *  Created on: 22-Mar-2019
 *      Author: gmahez
 */

#ifndef BUTTON_BUTTON_H_
#define BUTTON_BUTTON_H_

#include "cmsis_os.h"
#include "stm32f4xx_hal.h"


//!< Extern function from Button.c

void buttonTask(void const * argument);

#endif /* BUTTON_BUTTON_H_ */
