/*
 * debugConsole.c
 *
 *  Created on: 04-Apr-2019
 *      Author: gmahez
 *
 */
#include <string.h>
#include <stdlib.h>

#include "cmsis_os.h"
#include "debugConsole.h"

UART_HandleTypeDef huart2;          //!< There is a LL driver / Generated code of STM32 FW have limitation
                                    // ... We need to keep the variable name for uart configuration structer


SemaphoreHandle_t xMutexDebugUart = NULL;
QueueHandle_t gDebugConsoleQ;

static bool debugConsoleInit( void );

char tempBuff[50] = { 0 };
bool mDebugConInit = false;
uint8_t name[] = "Jithu...";


void debugconsoleTask(void)
{


    if( ! debugConsoleInit() )
    {
        while(1);           ///!< If uart initialization failed, don't run the task
    }

    for (;;)
    {
        debugTextValue("\nNOW I CAN PRINT TEST & VALUES\t", 157, DECIMAL );
        vTaskDelay(2000);

//        debugText("\nTHIS IS A TEXT MSG TO TEST uart INTERRUPT\t");
//        vTaskDelay(1000);
//
//        debugValue(879, DECIMAL );
//        vTaskDelay(1000);


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
     huart2.Instance = USART2;
     huart2.Init.BaudRate = 115200;
     huart2.Init.WordLength = UART_WORDLENGTH_8B;
     huart2.Init.StopBits = UART_STOPBITS_1;
     huart2.Init.Parity = UART_PARITY_NONE;
     huart2.Init.Mode = UART_MODE_TX_RX;
     huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
     huart2.Init.OverSampling = UART_OVERSAMPLING_16;
     if (HAL_UART_Init(&huart2) != HAL_OK)
     {
         returnValue = false;
       // Error_Handler(); NEED TO ADD error handling function.
     }else
     {
         __HAL_UART_ENABLE_IT(&huart2, UART_IT_TC);
     }



     mDebugConInit = returnValue;
     return ( returnValue );

}
/*********************************************************************************
 *Name :- debugText
 *Para1:- debugMsg
 *Return:-N/A
 *Details:-
 **********************************************************************************/
bool debugText( const char *debugMsg )
{
    char *msg =(char*) debugMsg;
    bool returnValue = false;

    if( mDebugConInit )
    {
        if( HAL_OK == HAL_UART_Transmit_IT(&huart2, (uint8_t *)msg, strlen(msg)) )
        {
            returnValue = true;
        }
    }

    return ( returnValue );
}
/*********************************************************************************
 *Name :- debugValue
 *Para1:- debugMsg
 *Para2:- value
 *Para3:- baseValue
 *Return:- PASS / FAIL
 *Details:-
 **********************************************************************************/
bool debugValue( uint32_t value, uint8_t baseValue)
{

    IntToText(value, baseValue, tempBuff);
    debugText( tempBuff );

    return 1;
}
/*********************************************************************************
 *Name :- debugTextValue
 *Para1:- debugMsg
 *Para2:- value
 *Para3:- baseValue
 *Return:-N/A
 *Details:-
 **********************************************************************************/
bool debugTextValue( const char *debugMsg, uint32_t value, uint8_t baseValue )
{
    bool returnValue = false;

    returnValue = debugText( debugMsg );
    vTaskDelay(100);
    debugValue( value, baseValue );

    return returnValue;
}
/*********************************************************************************
 *Name :- IntToText
 *Para1:- value
 *Para2:- base ( Numerical base )
 *Para3:- str  ( pointer to buffer )
 *Return:-N/A
 *Details:-
 **********************************************************************************/
bool IntToText(uint32_t value, uint8_t base, char * str )
{
    uint32_t temp = 0;
    uint8_t i = 0;
    uint32_t division = value;

    if( 0 == division )
    {
        *(str+i++) = temp|0x30;
    }else
    {
        while ( 0 != division )
        {
            temp = division%base;
            division /= base;

            if( ( base == HEX ) && (temp >= DECIMAL) )
                *(str+i++) = (temp - DECIMAL ) + 'A';
            else
                *(str+i++) = temp|0x30;
        }
    }

    *(str+i) = '\0';

    reverseStr(str, i);
    return true;
}
/*********************************************************************************
 *Name :- reverseStr
 *Para1:- pointer to string ( buffer)
 *Para2:- size of the string.
 *Return:-N/A
 *Details:-
 **********************************************************************************/
void reverseStr(char *str, uint8_t size)
{
    uint8_t i = size-1;
    uint8_t j = 0;
    uint8_t temp = i;

    do
    {
        temp     = *(str+j);
        *(str+j++) = *(str+i);
        *(str+i--) = temp;
        temp = i/2;
    }while( i > j );

}






