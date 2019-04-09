/*
 * debugConsole.c
 *
 *  Created on: 04-Apr-2019
 *      Author: gmahez
 */
#include "cmsis_os.h"
#include "debugConsole.h"

SemaphoreHandle_t xMutexDebugUart = NULL;


static bool debugConsoleInit( void );


UART_HandleTypeDef huart2;


void debugconsoleTask(void)
{
    if( ! debugConsoleInit() )
    {
        while(1);
    }

    for (;;)
    {
        if( xSemaphoreTake( xMutexDebugUart, 10 ) == pdTRUE )
        {

        }

        vTaskDelay(1);
    }
}

/*********************************************************************************
 *Name :- buttonTask
 *Para1:- argument
 *Return:-N/A
 *Details:-  Main task body for button..
 **********************************************************************************/
static bool debugConsoleInit( void )
{

}
