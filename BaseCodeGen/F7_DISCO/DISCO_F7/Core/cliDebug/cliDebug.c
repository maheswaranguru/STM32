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
#include "commands.h"

/*******************************************************************
 *                      LOCAL FUNCTION PROTOTYPES
 *******************************************************************/
void addToRing( char *strPtr, unsigned int strLeg );
void debugRxByteCallBack( uint8_t rxByte );

/*******************************************************************
 *                      GLOBAL / MODULE VARIABLES
 *******************************************************************/
UART_HandleTypeDef debugPortConfig;

uint8_t debugOutBuffer[ CLI_RING_BUFF_SIZE ] = { 0 };
char tempStrBuff[ 72 ] = { 0 };

volatile uint16_t writePtr = ZERO;
volatile uint16_t readPtr  = ZERO;
volatile bool     txStatus = true;    //!< true = Tx idle, false = a transmission is in progress
volatile static uint16_t txChunkLen = ZERO;    //!< Size of the block handed to HAL_UART_Transmit_IT

static uint8_t rxByteBuf = 0;         //!< Single byte scratch buffer handed to HAL_UART_Receive_IT

volatile static uint16_t RxByteCnt = ZERO;
volatile static bool     cmdRx_t = false;
volatile static uint8_t  command[ MAX_CMD_LENGTH ] = { 0 };
volatile static uint8_t  receivingCMD[ MAX_CMD_LENGTH ] = { 0 };

SemaphoreHandle_t xMutexDebugUart = NULL;

#ifdef FORMATED_PRINTF
SemaphoreHandle_t xMutexPrintf = NULL;
#endif

/*********************************************************************************
 *Name :- debugCliTask
 *Para1:- Task parameter pass by application
 *Return:-N/A
 *Details:-  Main task for Cli module. The intention is mainly for debug and test
 **********************************************************************************/
void debugCliTask( void *pvParameters )
{

    cliPrintLabel();

     while ( 1 )
     {
         if ( ( true == txStatus ) && ( writePtr != readPtr ) )
         {
             if ( writePtr > readPtr )
             {
                 txChunkLen = writePtr - readPtr;
             }
             else
             {
                 txChunkLen = CLI_RING_BUFF_SIZE - readPtr;
             }

             txStatus = false;

             if ( HAL_OK != HAL_UART_Transmit_IT( &debugPortConfig, ( uint8_t* ) &debugOutBuffer[ readPtr ], txChunkLen ) )
             {
                 txStatus = true;    //!< Failed to start, retry next loop pass
             }
         }

         if ( true == cmdRx_t )
         {
             cmdRx_t = false;
             executeCLiCommands( ( char* ) command );
         }

         vTaskDelay( 10 );
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
    }else
	{
    	HAL_UART_Receive_IT( &debugPortConfig, &rxByteBuf, 1 );

		xMutexDebugUart = xSemaphoreCreateMutex( );
		xMutexPrintf = xSemaphoreCreateMutex( );
		writePtr = ZERO;
		readPtr = ZERO;
	}

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
    debugText( "\r\n\n**************\t STM32F769 DISCO \t***********************" );
    debugText( "\r\n*************\t DRIVER DEVELOPMENT \t***********************" );
    debugText( "\r\n\nAPPLICATION VERSION\t:\t" );
    debugText( VERSION );
    debugText( "\r\nBOOT_VERSION\t\t    : " );
    debugText( BOOT_VERSION );
    debugText( "\r\nHARDWARE_VERSION\t\t  : " );
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

    if ( NULL != xMutexDebugUart )
    {
        if ( pdTRUE == xSemaphoreTake( xMutexDebugUart, MIN_TIME_TO_MUTEX ) )
        {
            addToRing( msg, strlen( msg ) );
            returnValue = xSemaphoreGive( xMutexDebugUart );
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
 *Details:- This will print a value in console with given base value.
 **********************************************************************************/
bool debugValue( int64_t value, uint8_t baseValue )
{
    bool returnValue = false;

    IntToText( tempStrBuff, value, baseValue );
    returnValue = debugText( tempStrBuff );

    return returnValue;
}

/*********************************************************************************
 *Name :- debugTextValue
 *Para1:- debugMsg
 *Para2:- value
 *Para3:- baseValue
 *Return:-N/A
 *Details:- This will print a test along with a value given with base value
 **********************************************************************************/
bool debugTextValue( const char *debugMsg, int64_t value, uint8_t baseValue )
{
    bool returnValue = false;

    returnValue = debugText( debugMsg );
    returnValue &= debugValue( value, baseValue );

    return returnValue;
}

/*********************************************************************************
 *Name :- addToRing
 *Para1:- pointer to string ( buffer)
 *Para2:- size of the string.
 *Return:-N/A
 *Details:- Add given stream of data / byte to debugOutBuffer. Does not start a
 *          transmission itself - debugCliTask() checks readPtr/writePtr and
 *          sends the pending block in one shot.
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
}

/*********************************************************************************
 *Name :- HAL_UART_TxCpltCallback
 *Para1:- debugPortConfig
 *Return:-N/A
 *Details:- Fired once the block started by debugCliTask() has fully gone out.
 *          Just advances readPtr and marks Tx idle; debugCliTask() will pick up
 *          any remaining (wrapped) data on its next pass.
 **********************************************************************************/
void HAL_UART_TxCpltCallback( UART_HandleTypeDef *debugPortConfig )
{
    if ( debugPortConfig->Instance == DEBUG_UART_INSTANCE )
    {
        readPtr += txChunkLen;
        if ( CLI_RING_BUFF_SIZE <= readPtr )
        {
            readPtr = ZERO;
        }

        txStatus = true;
    }
}

/*********************************************************************************
 *Name :- HAL_UART_RxCpltCallback
 *Para1:- debugPortConfig
 *Return:-N/A
 *Details:- Fired once a byte has been received. Hands it to debugRxByteCallBack
 *          then immediately re-arms the next single-byte receive.
 **********************************************************************************/
void HAL_UART_RxCpltCallback( UART_HandleTypeDef *debugUart )
{
    if ( debugUart->Instance == DEBUG_UART_INSTANCE )
    {
        debugRxByteCallBack( rxByteBuf );
        HAL_UART_Receive_IT( debugUart, &rxByteBuf, 1 );
    }
}

/*********************************************************************************
 *Name :- HAL_UART_ErrorCallback
 *Para1:- debugPortConfig
 *Return:-N/A
 *Details:- Framing / noise / overrun errors leave the HAL Rx state machine idle,
 *          so the receive must be re-armed here or the CLI would stop receiving.
 **********************************************************************************/
void HAL_UART_ErrorCallback( UART_HandleTypeDef *debugUart )
{
    if ( debugUart->Instance == DEBUG_UART_INSTANCE )
    {
        HAL_UART_Receive_IT( debugUart, &rxByteBuf, 1 );
    }
}

/*********************************************************************************
 *Name :- debugRxByteCallBack
 *Para1:- Received Byte
 *Return:- N/A
 *Details:-UART receive interrupt call back function
 **********************************************************************************/
void debugRxByteCallBack( uint8_t rxByte )
{
    receivingCMD[ RxByteCnt ] = rxByte;

    if ( ( '\n' == rxByte ) || ( '\r' == rxByte ) )
    {
        if ( ZERO != RxByteCnt )
        {
            strncpy( ( char* ) command, ( char* ) receivingCMD, ( RxByteCnt ) );
            command[ RxByteCnt ] = '\0';
            RxByteCnt = ZERO;
            cmdRx_t = true;
        }
        else
        {
            RxByteCnt = ZERO;
        }
    }
    else
    {
        if ( '\b' != rxByte )
        {
            if ( ( MAX_CMD_LENGTH - ONE ) <= RxByteCnt++ )
            {
                RxByteCnt = ZERO;
            }
        }
        else
        {
            if ( ZERO != RxByteCnt )    //!< Guard against underflow: RxByteCnt is unsigned
            {
                RxByteCnt--;
            }
        }
    }

    return;
}

/*********************************************************************************
 *Name :- IntToText
 *Para1:- value
 *Para2:- base ( Numerical base )
 *Para3:- str  ( pointer to buffer )
 *Return:-N/A
 *Details:- convert given value to string
 **********************************************************************************/
bool IntToText( char *str, int64_t value, uint8_t base )
{
    int64_t temp = ZERO;
    uint8_t i = ZERO;
    int64_t division = value;
    uint8_t baseValue = base;

    if ( ( DECIMAL != baseValue ) && ( HEX != baseValue ) && ( BINARY != baseValue ) )
    {
        baseValue = DECIMAL;
    }

    if ( ZERO == division )
    {
        *( str + i++ ) = temp | '0';
        *( str + i ) = '\0';
    }
    else if ( ZERO != baseValue )
    {
        if ( ZERO > division )
        {
            division = abs( division );
        }

        while ( ZERO != division )
        {
            temp = division % baseValue;
            division /= baseValue;

            if ( ( HEX == baseValue ) && ( DECIMAL <= temp ) )
            {
                *( str + i++ ) = ( temp - DECIMAL ) + 'A';
            }
            else
            {
                *( str + i++ ) = temp | 0x30;
            }
        }

        if ( HEX == baseValue )
        {
            *( str + i++ ) = 'x';
            *( str + i++ ) = '0';
            *( str + i++ ) = ' ';
        }
        else if ( BINARY == baseValue )
        {
            *( str + i++ ) = '.';
            *( str + i++ ) = 'b';
            *( str + i++ ) = ' ';
        }

        if ( ( ZERO > value ) && ( DECIMAL == baseValue ) )
        {
            *( str + i++ ) = '-';
        }

        *( str + i ) = '\0';

        reverseStr( str, i );
    }

    return true;
}

/*********************************************************************************
 *Name :- reverseStr
 *Para1:- pointer to string ( buffer)
 *Para2:- size of the string.
 *Return:-N/A
 *Details:- To reverse the string.
 **********************************************************************************/
void reverseStr( char *str, uint8_t size )
{
    uint8_t i = size - 1;
    uint8_t j = ZERO;
    uint8_t temp = i;

    do
    {
        temp = *( str + j );
        *( str + j ) = *( str + i );
        *( str + i ) = temp;

        if ( ZERO == i )
        {
            break;
        }

        i = i - 1;
        j = j + 1;
        temp = ( unsigned char ) i / 2;
    }
    while ( i > j );
}

/*********************************************************************************
 *Name :- mystrcmp
 *Para1:- string 1 to compare
 *Para2:- string 2 to compare
 *Return:-N/A
 *Details:- My own string compare without case sensitive
 ******************************************************************************/
bool mystrcmp( char *a, char *b )
{
    bool returnValue = false;
    char i = 0;

    while ( ( ( *( a + i ) != '\0' ) || ( *( b + i ) != '\0' ) ) && ( returnValue != 1 ) )
    {
        if ( *( a + i ) != *( b + i ) )
        {
            if ( *( a + i ) < *( b + i ) )
            {
                if ( ( *( a + i ) + 0x20 ) != ( *( b + i ) ) )
                {
                    returnValue = true;
                }
            }
            else
            {
                if ( ( *( a + i ) ) != ( *( b + i ) + 0x20 ) )
                {
                    returnValue = true;
                }
            }
        }
        i++;
    }

    if ( ( *( a + i ) != '\0' ) && ( *( b + i ) != '\0' ) )
    {
        returnValue = true;
    }

    return ( returnValue );
}

#ifdef FORMATED_PRINTF
/*********************************************************************************
 *Name :- myPrintf
 *Para1:- N/A
 *Return:-
 *Details:- This is Formated printf - It will take considerable memory
 *          so can be include to compile or NOT.
 **********************************************************************************/
void myPrintf( const char *pstr, ... )
{
    va_list va;
    char buffer[ 100 ] = { 0 };
    int strSize = 0;

    if ( NULL == xMutexPrintf )
    {
        return;
    }

    if ( pdTRUE == xSemaphoreTake( xMutexPrintf, MIN_TIME_TO_MUTEX ) )
    {
        va_start( va, pstr );
        strSize = vsnprintf( buffer, sizeof( buffer ), pstr, va );
        va_end( va );

        if ( 0 < strSize )
        {
            debugText( buffer );
        }

        xSemaphoreGive( xMutexPrintf );
    }
}
#endif
