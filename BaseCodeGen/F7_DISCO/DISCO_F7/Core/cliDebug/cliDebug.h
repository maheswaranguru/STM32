/*
 * cliDebug.h
 *
 *  Created on: 04-Jul-2026
 *      Author: Maheswaran Gurusamy
 */

#ifndef CLIDEBUG_CLIDEBUG_H_
#define CLIDEBUG_CLIDEBUG_H_

#include<stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "stm32f7xx_hal.h"
#include "macro.h"

char cliTaskInit( void );
void debugCliTask( void *pvParameters );

bool debugText( const char *debugMsg );
bool debugValue( int64_t value, uint8_t baseValue );
bool debugTextValue( const char *debugMsg, int64_t value, uint8_t baseValue );

//!< Utilities share with other modules
bool IntToText( char *str, int64_t value, uint8_t base );
void reverseStr( char *str, uint8_t size );
bool mystrcmp( char *a, char *b );


#endif /* CLIDEBUG_CLIDEBUG_H_ */
