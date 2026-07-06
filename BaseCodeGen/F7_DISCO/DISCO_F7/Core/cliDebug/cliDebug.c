/*
 * cliDebug.c
 *
 *  Created on: 28-Oct-2024
 *      Author: maheswaran_gurusamy
 */

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
UART_HandleTypeDef debugPortConfig;    //!< Debug console UART handle ( USART1, PA9-Tx / PA10-Rx )

uint8_t debugOutBuffer[ CLI_RING_BUFF_SIZE ] = { 0 };    //!< Tx ring buffer
char tempStrBuff[ 72 ] = { 0 };                          //!< Scratch buffer for IntToText()

volatile uint16_t writePtr = ZERO;    //!< Next free slot to write into debugOutBuffer
volatile uint16_t readPtr  = ZERO;    //!< Next byte pending transmission out of debugOutBuffer
volatile bool     txStatus = true;    //!< true = Tx idle, false = a transmission is in progress
volatile static uint16_t txChunkLen = ZERO;    //!< Size of the block last handed to HAL_UART_Transmit_IT

static uint8_t rxByteBuf = 0;    //!< Single byte scratch buffer handed to HAL_UART_Receive_IT

volatile static uint16_t RxByteCnt = ZERO;                          //!< Number of bytes gathered for the in-progress command line
volatile static bool     cmdRx_t = false;                           //!< Set true once a full command line ( \r or \n terminated ) is ready
volatile static uint8_t  command[ MAX_CMD_LENGTH ] = { 0 };         //!< Completed command line handed off to executeCLiCommands()
volatile static uint8_t  receivingCMD[ MAX_CMD_LENGTH ] = { 0 };    //!< In-progress command line, filled byte by byte from the Rx ISR

SemaphoreHandle_t xMutexDebugUart = NULL;    //!< Guards addToRing() against concurrent debugText() callers

#ifdef FORMATED_PRINTF
SemaphoreHandle_t xMutexPrintf = NULL;    //!< Guards the shared myPrintf() scratch buffer
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
        //!< Tx is driven from here, not from addToRing(): once idle, send the
        //!< whole contiguous run of pending bytes in a single HAL_UART_Transmit_IT()
        //!< call rather than re-triggering one byte at a time.
        if ( ( true == txStatus ) && ( writePtr != readPtr ) )
        {
            if ( writePtr > readPtr )
            {
                //!< Normal case - no wrap between readPtr and writePtr
                txChunkLen = writePtr - readPtr;
            }
            else
            {
                //!< writePtr has wrapped back to the start of the buffer ahead of
                //!< readPtr - send only up to the physical end of the buffer for
                //!< now, the remainder ( now at the front ) is picked up on a
                //!< later pass once readPtr itself wraps.
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
 *Name :- cliTaskInit
 *Para1:- N/A
 *Return:- returnValue ( 0 = success, non-zero = failure )
 *Details:- Self-contained USART1 ( PA9-Tx / PA10-Rx ) init in interrupt mode -
 *          no FIFO, no DMA, no CubeMX pinout dependency. Hardware and the sync
 *          primitives debugCliTask() depends on are fully set up BEFORE the
 *          task is created, so it can never run ahead of something not ready.
 **********************************************************************************/
char cliTaskInit( void )
{
    char returnValue = 0;
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    //!< 1. Tell the handle which USART instance it owns
    debugPortConfig.Instance = DEBUG_UART_INSTANCE;

    //!< 2. Enable the peripheral and GPIO port clocks
    __HAL_RCC_USART1_CLK_ENABLE( );
    __HAL_RCC_GPIOA_CLK_ENABLE( );    //!< USART1 uses PA9 ( Tx ) / PA10 ( Rx ) on STM32F769I-DISCO

    //!< 3. Configure the Tx/Rx pins for the USART1 alternate function
    GPIO_InitStruct.Pin       = DEBUG_UART_TX_PIN | DEBUG_UART_RX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = DEBUG_UART_AF;
    HAL_GPIO_Init( DEBUG_UART_GPIO_PORT, &GPIO_InitStruct );

    //!< 4. UART parameters - 115200 8N1, no flow control, pure interrupt mode
    debugPortConfig.Init.BaudRate               = DEBUG_BAUDRATE;
    debugPortConfig.Init.WordLength             = UART_WORDLENGTH_8B;
    debugPortConfig.Init.StopBits               = UART_STOPBITS_1;
    debugPortConfig.Init.Parity                 = UART_PARITY_NONE;
    debugPortConfig.Init.Mode                   = UART_MODE_TX_RX;
    debugPortConfig.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    debugPortConfig.Init.OverSampling           = UART_OVERSAMPLING_16;
    debugPortConfig.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    debugPortConfig.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if ( HAL_OK != HAL_UART_Init( &debugPortConfig ) )
    {
        //!< One retry in case of a transient issue. If it fails again, this MUST
        //!< be reported to the caller rather than silently continuing - otherwise
        //!< the CLI looks "initialized" while the UART was never actually configured.
        if ( HAL_OK != HAL_UART_Init( &debugPortConfig ) )
        {
            returnValue = true;
        }
    }

    if ( false == returnValue )
    {
        //!< 5. NVIC - required since HAL_UART_Transmit_IT()/HAL_UART_Receive_IT() rely on it
        HAL_NVIC_SetPriority( DEBUG_UART_IRQn, DEBUG_UART_IRQ_PRIORITY, 0 );
        HAL_NVIC_EnableIRQ( DEBUG_UART_IRQn );

        //!< 6. Arm the very first Rx byte. Its return value MUST be checked -
        //!<    if this silently fails, RXNEIE never gets set in USART1->CR1 and
        //!<    the CLI will never receive another byte again with no indication why.
        if ( HAL_OK != HAL_UART_Receive_IT( &debugPortConfig, &rxByteBuf, 1 ) )
        {
            returnValue = true;
        }
    }

    if ( false == returnValue )
    {
        xMutexDebugUart = xSemaphoreCreateMutex( );
        xMutexPrintf    = xSemaphoreCreateMutex( );

        if ( ( NULL == xMutexDebugUart ) || ( NULL == xMutexPrintf ) )
        {
            returnValue = true;
        }
    }

    writePtr = ZERO;
    readPtr  = ZERO;

    if ( false == returnValue )
    {
        //!< Only start the CLI task once the UART and mutexes it depends on are
        //!< fully ready - creating it any earlier risks debugCliTask() running
        //!< ( on preemption ) before debugPortConfig / the mutexes exist.
        if ( pdTRUE != xTaskCreate( debugCliTask, ( const portCHAR* ) "CLI", 512, NULL, tskIDLE_PRIORITY, NULL ) )
        {
            returnValue = true;
        }
    }

    return ( returnValue );
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
    //!< Bytes free between writePtr and the physical end of the buffer. The
    //!< trailing "-ONE" means the very last array slot is never used at the
    //!< wrap boundary - a deliberate one-byte margin from the original design.
    unsigned int remaingBuff = ( CLI_RING_BUFF_SIZE - writePtr ) - ONE;

    if ( remaingBuff > strLeg )
    {
        //!< Whole string fits before the end of the buffer - no wrap needed
        memcpy( &debugOutBuffer[ writePtr ], strPtr, strLeg );
        writePtr += strLeg;
    }
    else
    {
        //!< String crosses the end of the buffer - copy the part that fits at
        //!< the tail, then wrap the remainder back to the start ( index 0 )
        memcpy( &debugOutBuffer[ writePtr ], strPtr, remaingBuff );
        memcpy( debugOutBuffer, ( unsigned char* ) ( strPtr + remaingBuff ), ( strLeg - remaingBuff ) );

        writePtr = ( strLeg - remaingBuff );
    }

    //!< NOTE: this does not check writePtr against readPtr, so a producer that
    //!< outruns the UART ( buffer fills faster than it drains ) will silently
    //!< overwrite bytes that haven't been transmitted yet. Not an issue at
    //!< typical CLI text volumes, but worth knowing if heavy myPrintf() use is
    //!< ever added.
}

/*********************************************************************************
 *Name :- HAL_UART_TxCpltCallback
 *Para1:- huart
 *Return:-N/A
 *Details:- Fired once the block started by debugCliTask() has fully gone out.
 *          Just advances readPtr and marks Tx idle; debugCliTask() will pick up
 *          any remaining (wrapped) data on its next pass.
 **********************************************************************************/
void HAL_UART_TxCpltCallback( UART_HandleTypeDef *huart )
{
    if ( huart->Instance == DEBUG_UART_INSTANCE )
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
 *Para1:- huart
 *Return:-N/A
 *Details:- Fired once a byte has been received. Hands it to debugRxByteCallBack
 *          then immediately re-arms the next single-byte receive.
 **********************************************************************************/
void HAL_UART_RxCpltCallback( UART_HandleTypeDef *huart )
{
    if ( huart->Instance == DEBUG_UART_INSTANCE )
    {
        debugRxByteCallBack( rxByteBuf );
        HAL_UART_Receive_IT( huart, &rxByteBuf, 1 );
    }
}

/*********************************************************************************
 *Name :- HAL_UART_ErrorCallback
 *Para1:- huart
 *Return:-N/A
 *Details:- Framing / noise / overrun errors leave the HAL Rx state machine idle,
 *          so the receive must be re-armed here or the CLI would stop receiving.
 **********************************************************************************/
void HAL_UART_ErrorCallback( UART_HandleTypeDef *huart )
{
    if ( huart->Instance == DEBUG_UART_INSTANCE )
    {
        HAL_UART_Receive_IT( huart, &rxByteBuf, 1 );
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
    //!< Always stage the byte first - harmless even for '\r'/'\n'/backspace,
    //!< since those cases either reset RxByteCnt or overwrite this slot next time.
    receivingCMD[ RxByteCnt ] = rxByte;

    if ( ( '\n' == rxByte ) || ( '\r' == rxByte ) )
    {
        //!< Line terminator - hand the completed line over to the CLI task,
        //!< but only if something was actually typed ( ignore a bare Enter ).
        if ( ZERO != RxByteCnt )
        {
            strncpy( ( char* ) command, ( char* ) receivingCMD, ( RxByteCnt ) );
            command[ RxByteCnt ] = '\0';    //!< strncpy() only copies RxByteCnt bytes - this terminates it
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
            //!< Ordinary character - append it, wrapping the line back to the
            //!< start if it ever grows too long for the command buffer.
            if ( ( MAX_CMD_LENGTH - ONE ) <= RxByteCnt++ )
            {
                RxByteCnt = ZERO;
            }
        }
        else
        {
            //!< Backspace - step back one character. RxByteCnt is unsigned, so
            //!< this MUST be guarded: decrementing at 0 wraps to 65535 and the
            //!< next byte would be written far outside receivingCMD[].
            if ( ZERO != RxByteCnt )
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

    //!< Fall back to decimal if an unsupported base was requested
    if ( ( DECIMAL != baseValue ) && ( HEX != baseValue ) && ( BINARY != baseValue ) )
    {
        baseValue = DECIMAL;
    }

    if ( ZERO == division )
    {
        //!< Special-case zero - the digit-extraction loop below never runs for it
        *( str + i++ ) = temp | '0';
        *( str + i ) = '\0';
    }
    else if ( ZERO != baseValue )
    {
        if ( ZERO > division )
        {
            division = abs( division );
        }

        //!< Extract digits least-significant-first; the buffer is reversed afterwards
        while ( ZERO != division )
        {
            temp = division % baseValue;
            division /= baseValue;

            if ( ( HEX == baseValue ) && ( DECIMAL <= temp ) )
            {
                *( str + i++ ) = ( temp - DECIMAL ) + 'A';    //!< 10..15 -> 'A'..'F'
            }
            else
            {
                *( str + i++ ) = temp | 0x30;    //!< 0..9 -> ASCII '0'..'9'
            }
        }

        //!< Append the base suffix ( still in reverse order at this point )
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

        reverseStr( str, i );    //!< Flip into the correct, human-readable order
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

    //!< Classic two-pointer swap, closing in from both ends towards the middle
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

    //!< Walk both strings together until either terminates, or a genuine
    //!< ( case-insensitive ) mismatch is found
    while ( ( ( *( a + i ) != '\0' ) || ( *( b + i ) != '\0' ) ) && ( returnValue != 1 ) )
    {
        if ( *( a + i ) != *( b + i ) )
        {
            //!< Characters differ - check whether it's just a case difference
            //!< ( ASCII lower/upper case letters are exactly 0x20 apart )
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

    //!< Loop can only exit early ( before both hit '\0' ) via a mismatch, but
    //!< this re-check also catches the case where one string is a strict
    //!< prefix of the other ( e.g. "help" vs "helpme" ).
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
        strSize = vsnprintf( buffer, sizeof( buffer ), pstr, va );    //!< Bounded - vsprintf() would not clip at buffer's size
        va_end( va );

        if ( 0 < strSize )
        {
            debugText( buffer );
        }

        xSemaphoreGive( xMutexPrintf );
    }
}
#endif
