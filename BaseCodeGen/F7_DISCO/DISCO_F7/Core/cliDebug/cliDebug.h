/*
 * debugCLi.h
 *
 *  Created on: 28-Oct-2024
 *      Author: maheswaran_gurusamy
 */

#ifndef DEBUGCLI_DEBUGCLI_H_
#define DEBUGCLI_DEBUGCLI_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "stm32f7xx_hal.h"
#include "macro.h"

/*******************************************************************
 *                      MACRO DEFINITION
 *******************************************************************/
//!< Debug console UART peripheral (STM32F769I-DISCO: USART1, PA9-Tx / PA10-Rx)
//!< Plain Rx/Tx interrupt mode only - no FIFO, no DMA.

#ifndef DEBUG_UART_INSTANCE
#define DEBUG_UART_INSTANCE    USART1
#endif

#ifndef CLI_RING_BUFF_SIZE
#define CLI_RING_BUFF_SIZE 2048
#endif

#ifndef MAX_TIME_TO_MUTEX
#define MAX_TIME_TO_MUTEX   5
#endif

#ifndef MIN_TIME_TO_MUTEX
#define MIN_TIME_TO_MUTEX   1
#endif

#ifndef MAX_CMD_LENGTH
#define MAX_CMD_LENGTH  25
#endif

#ifndef DEBUG_UART_GPIO_PORT
#define DEBUG_UART_GPIO_PORT   GPIOA
#endif

#ifndef DEBUG_UART_TX_PIN
#define DEBUG_UART_TX_PIN      GPIO_PIN_9
#endif

#ifndef DEBUG_UART_RX_PIN
#define DEBUG_UART_RX_PIN      GPIO_PIN_10
#endif

#ifndef DEBUG_UART_AF
#define DEBUG_UART_AF          GPIO_AF7_USART1
#endif

#ifndef DEBUG_UART_IRQn
#define DEBUG_UART_IRQn        USART1_IRQn
#endif

#ifndef DEBUG_UART_IRQ_PRIORITY
#define DEBUG_UART_IRQ_PRIORITY 5
#endif

#ifndef DEBUG_BAUDRATE
#define DEBUG_BAUDRATE 115200
#endif

#define FORMATED_PRINTF

/*******************************************************************
 *                      FUNCTION PROTOTYPES
 *******************************************************************/
char cliTaskInit( void );
void debugCliTask( void *pvParameters );

bool debugText( const char *debugMsg );
bool debugValue( int64_t value, uint8_t baseValue );
bool debugTextValue( const char *debugMsg, int64_t value, uint8_t baseValue );

//!< Utilities shared with other modules
bool IntToText( char *str, int64_t value, uint8_t base );
void reverseStr( char *str, uint8_t size );
bool mystrcmp( char *a, char *b );

#ifdef FORMATED_PRINTF
void myPrintf( const char *pstr, ... );
#else
#define myPrintf
#endif

void cliPrintLabel( void );

#endif /* DEBUGCLI_DEBUGCLI_H_ */
