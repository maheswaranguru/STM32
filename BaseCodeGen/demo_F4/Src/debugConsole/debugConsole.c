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


UART_HandleTypeDef configUart2;


void debugconsoleTask(void)
{
    if( ! debugConsoleInit() )
    {
        while(1);           ///!< If uart initialization faild don't run the task
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
 *Name :- debugConsoleInit
 *Para1:- N/A
 *Return:-N/A
 *Details:-  Initialize the uart used for debug.
 **********************************************************************************/
static bool debugConsoleInit( void )
{

    bool returnValue = true;
     /* USER CODE END USART2_Init  */
     configUart2.Instance = USART2;
     configUart2.Init.BaudRate = 115200;
     configUart2.Init.WordLength = UART_WORDLENGTH_8B;
     configUart2.Init.StopBits = UART_STOPBITS_1;
     configUart2.Init.Parity = UART_PARITY_NONE;
     configUart2.Init.Mode = UART_MODE_TX_RX;
     configUart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
     configUart2.Init.OverSampling = UART_OVERSAMPLING_16;
     if (HAL_UART_Init(&configUart2) != HAL_OK)
     {
         returnValue = false;
       // Error_Handler(); NEED TO ADD error handling function.
     }

     return ( returnValue );
}
