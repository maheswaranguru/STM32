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
#include "queue.h"

#define KEY_POLLING_INTERVEL    10          //!< (1/configTICK_RATE_HZ) Second - ( currently it is X*1ms)
#define KEY_DEBOUNCE_CNT        3          //!< debounce time = KEY_POLLING_INTERVEL * KEY_DEBOUNCE_CNT

typedef enum
{
    RELEASED = 0, PRESSED
} keyStat_t;

//!< This structure will support upto 32 key. if you
typedef struct
{
    uint32_t currentStatus;
    uint32_t newStatus;
    uint32_t latchedStatus;
    uint32_t whichKey;
    keyStat_t keyStat;
    uint8_t debounceCnt;
} keyReadStatus_t;

typedef enum
{
    SWITCH = 0,
    MAXIMUM_BUTTON
}eKeyName_t;

//!< Extern function from Button.c

void buttonTask(void const * argument);

//!< Extern Variable
extern QueueHandle_t gKeyDetectQ;

#endif /* BUTTON_BUTTON_H_ */
