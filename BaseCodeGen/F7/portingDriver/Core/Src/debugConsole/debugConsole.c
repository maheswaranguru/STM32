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

UART_HandleTypeDef debugPort;          //!< There is a LL driver / Generated code of STM32 FW have limitation
                                    // ... We need to keep the variable name for uart configuration structer


SemaphoreHandle_t xMutexDebugUart = NULL;
QueueHandle_t gDebugConsoleQ;

static bool debugConsoleInit( void );

char tempBuff[50] = {0};
bool mDebugConInit = false;

uint8_t number =0;

void debugconsoleTask(void)
{


    if( ! debugConsoleInit() )
    {
        while(1);           ///!< If uart initialization failed, don't run the task
    }else
    {
    	if( HAL_UART_STATE_READY == HAL_UART_GetState( &debugPort ) )
		{
			debugText("\n******************************************** ");
			 vTaskDelay(100);
			debugText("\nTHIS IS EDGE FEEDER HMI FOR INTEGRATION EVENT ");
			 vTaskDelay(100);
			debugText("\n#########    SUPER BASIC VERSIOM    ######### ");
			 vTaskDelay(100);
			debugText("\n**********************************************\n");
			 vTaskDelay(100);
		}else
		{
			while(1);
		}
    }

    for (;;)
    {
//        debugTextValue("\nNOW I CAN PRINT DECIMAL\t", number, DECIMAL );
//        vTaskDelay(10);
//        debugTextValue("\nNOW I CAN PRINT HEX\t", number, HEX );
//        vTaskDelay(10);
//        debugTextValue("\nNOW I CAN PRINT BINARY\t", number, BINARY );
//        number++;

    	if( HAL_UART_ERROR_NONE != HAL_UART_GetError(&debugPort) )
    	{
    		HAL_UART_DeInit(&debugPort);
    		debugConsoleInit();
    	}

        vTaskDelay(1000);

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
     debugPort.Instance = DEBUG_UART;
     debugPort.Init.BaudRate = 115200;
     debugPort.Init.WordLength = UART_WORDLENGTH_8B;
     debugPort.Init.StopBits = UART_STOPBITS_1;
     debugPort.Init.Parity = UART_PARITY_NONE;
     debugPort.Init.Mode = UART_MODE_TX_RX;
     debugPort.Init.HwFlowCtl = UART_HWCONTROL_NONE;
     debugPort.Init.OverSampling = UART_OVERSAMPLING_16;
     if (HAL_UART_Init(&debugPort) != HAL_OK)
     {
         returnValue = false;
       // Error_Handler(); NEED TO ADD error handling function.
     }else
     {
        // __HAL_UART_ENABLE_IT(&debugPort, UART_IT_TC);
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
        //if( HAL_OK == HAL_UART_Transmit_IT(&debugPort, (uint8_t *)msg, strlen(msg)) )
        if( HAL_OK == HAL_UART_Transmit(&debugPort, (uint8_t *)msg, strlen(msg), 1000) )
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

    while(   HAL_UART_STATE_READY != HAL_UART_GetState(&debugPort) );

    IntToText(value, baseValue, tempBuff);
    debugText( tempBuff );

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
        *(str+i) = '\0';
    }else
    {
        while ( 0 != division )
        {
            temp = division%base;
            division /= base;

            if( ( base == HEX ) && (temp >= DECIMAL) )
            {
                *(str+i++) = (temp - DECIMAL ) + 'A';
             }
            else
            {
                *(str+i++) = temp|0x30;
            }
        }
       	if( base == HEX )
		{
			*(str+i++) = 'x';
			*(str+i++) = '0';
			*(str+i++) = ' ';
		}else if( base == BINARY )
		{
			*(str+i++) = '.';
			*(str+i++) = 'b';
			*(str+i++) = ' ';
		}


        *(str+i) = '\0';
        reverseStr(str, i);
    }


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
        *(str+j) = *(str+i);
        *(str+i) = temp;
        if( i == 0 ) break;		//!<  NEED TO FIND A BETTER WAY TO HANDLE THE UNDERFLOW !
        i = i-1;
        j = j +1;
        temp = (unsigned char) i/2;
    }while( i > j );

}






