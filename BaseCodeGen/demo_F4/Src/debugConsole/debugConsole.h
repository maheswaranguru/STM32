/*
 * debugConsole.h
 *
 *  Created on: 09-Apr-2019
 *      Author: gmahez
 */

#ifndef DEBUGCONSOLE_DEBUGCONSOLE_H_
#define DEBUGCONSOLE_DEBUGCONSOLE_H_

#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

#include "sysConfigEsab.h"


extern SemaphoreHandle_t xMutexDebugUart;

void debugconsoleTask(void);

#endif /* DEBUGCONSOLE_DEBUGCONSOLE_H_ */
