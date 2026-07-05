/* cliDebug.c */
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <FreeRTOS.h>
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "timers.h"

#include "cliDebug.h"

#ifndef MAX_CMD_LENGTH
#define MAX_CMD_LENGTH  25
#endif

__interrupt void INT_debugPort_TX_ISR( void );

// 1. Define the global handle used locally
UART_HandleTypeDef debugPortConfig;

SemaphoreHandle_t xMutexDebugUart = NULL;    //!< Mutex for debugCLi Api's
#ifdef FORMATED_PRINTF
SemaphoreHandle_t xMutexPrintf = NULL;    //!< Mutex for debugCLi Api's
#endif


bool pushToTxFifo( uint8_t *sratptr, uint8_t len );
static bool debugConsoleFlushOut( void );
void addToRing( char *strPtr, unsigned int strLeg );

uint8_t debugOutBuffer[ CLI_RING_BUFF_SIZE ] = { 0 };

/*********************************************************************************
 *Name :- debugCliTask
 *Para1:- Task parameter pass by application
 *Return:-N/A
 *Details:-  Main task for Cli module. The intention is mainly for debug and test
 **********************************************************************************/
void debugCliTask( void *pvParameters )
{
    uint8_t testStr[] = "\r\nMesg From Debug CLI port Task.";


    while (1)
    {
        if ( ( true != txStatus ) && ( writePtr != readPtr ) )  //!< if something remaining to flush out on console, DO IT !
        {
            if ( UARTSpaceAvail( debugPort ) )
            {
                debugConsoleFlushOut();
            }
        }

//        HAL_UART_Transmit_IT( &debugPortConfig, testStr, 32 );
//        vTaskDelay( pdMS_TO_TICKS(1000) );
    }
}

/*********************************************************************************
 * Name :- cliTaskInit
 * Details:- Self-contained initialization for USART1 (No CubeMX dependence)
 **********************************************************************************/
char cliTaskInit( void )
{
    char returnValue = 0;

    if ( xTaskCreate( debugCliTask, ( const portCHAR* ) "CLI", 512, NULL, tskIDLE_PRIORITY, NULL ) != pdTRUE )
    {
        returnValue = true;
    }

    // 2. CRITICAL STEP: Assign the Hardware Register Instance
    debugPortConfig.Instance = USART1;

    // 3. HARDWARE LAYER INITIALIZATION (What CubeMX used to do in the background)

    // A. Enable Peripheral Clocks
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE(); // USART1 typically uses PA9 (TX) and PA10 (RX) on STM32F769

    // B. Configure GPIO Pins for Alterate Function (UART Mode)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1; // AF7 is standard for USART1 on STM32F7
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // C. Configure NVIC for Interrupts (Required since you use _IT functions)
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0); // Priority 5 or lower is safe for FreeRTOS API usage
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    // 4. PARAMETER CONFIGURATION
    debugPortConfig.Init.BaudRate = 115200;
    debugPortConfig.Init.WordLength = UART_WORDLENGTH_8B;
    debugPortConfig.Init.StopBits = UART_STOPBITS_1;
    debugPortConfig.Init.Parity = UART_PARITY_NONE;
    debugPortConfig.Init.Mode = UART_MODE_TX_RX;
    debugPortConfig.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    debugPortConfig.Init.OverSampling = UART_OVERSAMPLING_16;
    debugPortConfig.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    debugPortConfig.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&debugPortConfig) != HAL_OK)
    {
        // Re-try or Handle Error locally
        HAL_UART_Init(&debugPortConfig);
    }

        writePtr = ZERO;
        readPtr = ZERO;

    return returnValue;
}
/*********************************************************************************
 *Name :- cliPrintLabel
 *Para1:- N/A
 *Return:-N/A
 *Details:- Just print welcome Note in Console.
 **********************************************************************************/
void cliPrintLabel( void )
{
    debugText( "\r\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" );
    debugText( "\r\n*********************************************************" );
    debugText( "\r\n\n**********\t ALL SPACE\t***********************" );
    debugText( "\r\n**********\tMotion Table\t***********************" );
    debugText( "\r\n\nAPPLICATION VERSION\t:\t" );
    debugText( VERSION );
    debugText( "\r\nBOOT_VERSION\t\t:\t" );
    debugText( BOOT_VERSION );
    debugText( "\r\nHARDWARE_VERSION\t:\t" );
    debugText( HARDWARE_VERSION );
    debugText( "\r\n\n*********************************************************" );
    debugText( "\r\n##########################################################\r\n\n" );

    return;
}
/*********************************************************************************
 *Name :- debugText
 *Para1:- debugMsg
 *Return:-N/A
 *Details:- It is to print a given test in console.
 **********************************************************************************/
bool debugText( const char *debugMsg )
{

    char *msg = ( char* ) debugMsg;
    bool returnValue = false;

    if( NULL != xMutexDebugUart )
    {
        if ( pdTRUE == xSemaphoreTake( xMutexDebugUart, MIN_TIME_TO_MUTEX ) )
        {
            addToRing( msg, strlen( msg ) );
            returnValue = xSemaphoreGive( xMutexDebugUart );    //!< Semaphore give back properly
        }
    }

    return ( returnValue );
}

/*********************************************************************************
 *Name :- addToRing
 *Para1:- pointer to string ( buffer)
 *Para2:- size of the string.
 *Return:-N/A
 *Details:- Add given stream of data / byte to debugOutBuffer
 **********************************************************************************/
void addToRing( char *strPtr, unsigned int strLeg )
{
    unsigned int remaingBuff = ( CLI_RING_BUFF_SIZE - writePtr ) - ONE;

    if ( remaingBuff > strLeg )
    {
        memcpy( &debugOutBuffer[ writePtr ], strPtr, strLeg );
        writePtr += strLeg;
    }
    else
    {
        memcpy( &debugOutBuffer[ writePtr ], strPtr, remaingBuff );
        memcpy( debugOutBuffer, ( unsigned char* ) ( strPtr + remaingBuff ), ( strLeg - remaingBuff ) );

        writePtr = ( strLeg - remaingBuff );
    }

    txStatus = false;
}

/*********************************************************************************
 *Name :- debugConsoleFlushOut
 *Para1:- N/A
 *Return:- true = sent successfully ; false = fail to sent
 *Details:- Move
 **********************************************************************************/
static bool debugConsoleFlushOut( void )
{
    bool returnValue = true;

    if ( writePtr != readPtr )
    {
        if ( writePtr > readPtr )  //!< Normal case read ptr is higher and write ptr follows
        {
            if ( ( writePtr - readPtr ) >= FIFO_SIZE )
            {
                pushToTxFifo( ( debugOutBuffer + readPtr ), ( uint8_t ) FIFO_SIZE );
                readPtr += FIFO_SIZE;
            }
            else
            {
                pushToTxFifo( ( debugOutBuffer + readPtr ), ( uint8_t ) writePtr - readPtr );
                readPtr = writePtr;
                txStatus = true;    //!< Just update the flag for Tx complete.
            }

        }
        else            //!< Circular buff finished and start from beginning.
        {
            if ( ( CLI_RING_BUFF_SIZE - readPtr ) >= FIFO_SIZE )
            {
                pushToTxFifo( ( debugOutBuffer + readPtr ), ( uint8_t ) FIFO_SIZE );
                readPtr += FIFO_SIZE;
            }
            else
            {
                pushToTxFifo( ( debugOutBuffer + readPtr ), ( uint8_t ) ( CLI_RING_BUFF_SIZE - readPtr ) );
                readPtr = ZERO;
            }
        }
    }

    return returnValue;
}

/*********************************************************************************
 *Name :- INT_debugPort_TX_ISR
 *Para1:- N/A
 *Return:-
 *Details:- ISR routine for UART Transmit complete.
 **********************************************************************************/
__interrupt void INT_debugPort_TX_ISR( void )
{
    volatile uint32_t uartInt = UARTIntStatus( debugPort, true );
    volatile uint32_t uartError = UARTRxErrorGet( debugPort );

    if ( UART_INT_TX == ( uartInt & UART_INT_TX ) )
    {
        if ( writePtr != readPtr )          //!< If still something remaining to tx... then do it.
        {
            if ( UARTSpaceAvail( debugPort ) )
            {
                debugConsoleFlushOut();
            }
        }
        UARTIntClear( debugPort, UART_INT_TX );

    }

    if ( UART_INT_RX == ( uartInt & UART_INT_RX ) )     //!< process the receive interrupt
    {
        do
        {
            debugRxByteCallBack( ( uint8_t ) UARTCharGetNonBlocking( debugPort ) );
            UARTIntClear( debugPort, UART_INT_RX );
        }while( true == UARTCharsAvail(debugPort ));
    }

}
