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

#define KEY_DEBOUNCE_CNT  3


//!< This structure will support upto 32 key. if you
typedef struct{
    uint32_t    currentStatus;
    uint32_t    newStatus;
    uint32_t    latchedStatus;
    uint8_t     debounceCnt;
}keyReadStatus_t;


//!< Extern function from Button.c

void buttonTask(void const * argument);

#endif /* BUTTON_BUTTON_H_ */
