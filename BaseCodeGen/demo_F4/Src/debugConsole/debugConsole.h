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
#include "stm32f4xx_hal_uart.h"

#include "sysConfigEsab.h"

#define DECIMAL 10
#define HEX     16
#define BINARY  2


extern QueueHandle_t gDebugConsoleQ;
extern SemaphoreHandle_t xMutexDebugUart;


void debugconsoleTask(void);

bool debugText( const char *debugMsg );
bool debugValue( uint32_t value, uint8_t baseValue );
bool debugTextValue( const char *debugMsg, uint32_t value, uint8_t baseValue );


bool IntToText(uint32_t value, uint8_t base, char * str );
void reverseStr(char *str, uint8_t size);

//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

#endif /* DEBUGCONSOLE_DEBUGCONSOLE_H_ */
