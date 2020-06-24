/*
 * led.c
 *
 *  Created on: 22-Mar-2019
 *      Author: gmahez
 */
#include <stdio.h>
#include "gpioWrapper.h"
#include "debugConsole.h"

#include "led.h"

//#include "button.h"

#define LE_Pin GPIO_PIN_3
#define LE_GPIO_Port GPIOE
#define OE_Pin GPIO_PIN_4
#define OE_GPIO_Port GPIOE


#define HB_ELAPSE_TIME_MS	2		//!<
#define TASK_EXE_DELAY		100

/*****************************************************************************************
 * @@@@@@@@@@@@@@@@@@   LED GPIO CONFIGURATION @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
******************************************************************************************/
//!< This could be modifiable to match platform driver support.
const gpioPinConfig_t led [ MAXIMUM_LED ] =
{
        { GPIOC, { GPIO_PIN_8, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0 } },
        { GPIOC, { GPIO_PIN_9, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0 } },

};
//#define PIN_INIT( a )  { HAL_GPIO_Init(key [ a ].port, (GPIO_InitTypeDef *) &key [ a ].pinConfig); }
/*@@@@@@@@@@@@@@@@@@@@@ CONFIGURATION END @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void intLed( void );
void ledTurnOnAll( void );
void ledTurnOffAll( void );
void ledTurnOn(  eLedName_t ledName  );
void ledTurnOff(  eLedName_t ledName  );
void ledToggle( eLedName_t ledName );

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#define LEDSTRIP_SPI_PORT SPI4
#define MAXIMUM_DIGIT 3

void intLedStrip( void );
void selfTestPowerOnLedStrip( void );
bool updateLedStrip( ledStripData_t * newData );
bool updateSevenSegDecoder( char *buf , float value );

SPI_HandleTypeDef ledStripSpiStruct;
ledStripData_t newDataLedStrip;
bool ledStripUpdate_f = false;

char digit[50] = { 0 };

#define INCREMENTTIME 10
uint8_t num = 0;

/*######################################################################################################*/
/*-----------------------------  Task Start Here  ------------------------------------------------------*/
/*######################################################################################################*/

void ledTask(void const * argument)
{
	uint16_t HeartbeatTime = 0;
	uint16_t incrementTime = 0;
    (void) argument;            //!< Just ignore the parameter.

    float fnum = 1.12;

    char formatStr2[20];
     sprintf(formatStr2, "%s.%dlf", "%", 1);


    intLed();               //!< Initialize Led
    intLedStrip();
    ledTurnOffAll();
    ledToggle( GREEN );

    num = 0;
    sprintf(digit, "%.2f",  fnum );

  for(;;)
  {
	  if( HeartbeatTime++ >= HB_ELAPSE_TIME_MS)
	  {
		ledToggle( GREEN );
		ledToggle( RED );
		HeartbeatTime = 0;
	  }

	   if ( incrementTime++ >=INCREMENTTIME  )
	   {
		   incrementTime = 0;

		   if ( num++ >= 1000 )
			 num = 0;

		   updateVoltageDisplay( num );
		   updateCurrentSpeedDisplay( num );

		   if( false != ledStripUpdate_f)
		   {
			   updateLedStrip( &newDataLedStrip );
			   ledStripUpdate_f = false;
		   }
		   debugTextValue( "\nValue =", num, DECIMAL );
	   }

	  vTaskDelay(100);


  }//!< for ever loop for task


}

/*************************************************************************************
 *Name  :intLed
 *Para1 :N/A
 *Return:N/A
 *Note: Initialize all LED pins configured on 'led' structure.
 ************************************************************************************/
void intLed(void)
{
    uint8_t i = 0;

    for ( i = 0; i < MAXIMUM_LED; i++ )
    {
        PIN_INIT( (led[ i ].port), &(led[i].pinConfig));      //!< Macro defined in buttonWrapper.h. May be need to change if required.
    }
}

/*************************************************************************************
 *Name  :ledTurnOnAll
 *Para1 :N/A
 *Return:N/A
 *Note: Turn ON all LEDs
 ************************************************************************************/
void ledTurnOnAll( void )
{
    uint8_t i = 0;

    for ( i = 0; i < MAXIMUM_LED; i++ )
    {
        WRITE_PIN((led[ i ].port), led[i].pinConfig.Pin, LED_ON );
    }
}
/*************************************************************************************
 *Name  :ledTurnOffAll
 *Para1 :N/A
 *Return:N/A
 *Note: Turn OFF all LEDs
 ************************************************************************************/
void ledTurnOffAll( void )
{
    uint8_t i = 0;

    for ( i = 0; i < MAXIMUM_LED; i++ )
    {
        WRITE_PIN((led[i].port), led[i].pinConfig.Pin, LED_OFF );
    }
}
/*************************************************************************************
 *Name  :ledTurnOn
 *Para1 :LED number
 *Return:N/A
 *Note: toggle particular LED. LED number(param1) should come from eLedName_t
 ************************************************************************************/
void ledTurnOn( eLedName_t ledName )
{

    WRITE_PIN((led[ledName].port), led[ledName].pinConfig.Pin, LED_ON );

}
/*************************************************************************************
 *Name  :ledTurnOff
 *Para1 :N/A
 *Return:N/A
 *Note: Turn OFF all LEDs
 ************************************************************************************/
void ledTurnOff( eLedName_t ledName )
{
    WRITE_PIN((led[ledName].port), led[ledName].pinConfig.Pin, LED_OFF );
}
/*************************************************************************************
 *Name  :ledToggle
 *Para1 :N/A
 *Return:N/A
 *Note: Turn OFF all LEDs
 ************************************************************************************/
void ledToggle( eLedName_t ledName )
{
	TOGGLE_PIN((led[ledName].port), led[ledName].pinConfig.Pin  );
}

/*************************************************************************************
 *Name  :ledTurnOff
 *Para1 :N/A
 *Return:N/A
 *Note: Turn OFF all LEDs
 ************************************************************************************/
void intLedStrip( void )
{
	  ledStripSpiStruct.Instance = LEDSTRIP_SPI_PORT;
	  ledStripSpiStruct.Init.Mode = SPI_MODE_MASTER;
	  ledStripSpiStruct.Init.Direction = SPI_DIRECTION_2LINES;
	  ledStripSpiStruct.Init.DataSize = SPI_DATASIZE_8BIT;
	  ledStripSpiStruct.Init.CLKPolarity = SPI_POLARITY_LOW;
	  ledStripSpiStruct.Init.CLKPhase = SPI_PHASE_1EDGE;
	  ledStripSpiStruct.Init.NSS = SPI_NSS_SOFT;
	  ledStripSpiStruct.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	  ledStripSpiStruct.Init.FirstBit = SPI_FIRSTBIT_MSB;
	  ledStripSpiStruct.Init.TIMode = SPI_TIMODE_DISABLE;
	  ledStripSpiStruct.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	  ledStripSpiStruct.Init.CRCPolynomial = 7;
	  if (HAL_SPI_Init(&ledStripSpiStruct) != HAL_OK)
	  {
	    while(1);
	  }else
	  {
		  selfTestPowerOnLedStrip();
	  }
}
/*************************************************************************************
 *Name  :selfTestPowerOnLedStrip
 *Para1 :N/A
 *Return:N/A
 *Note: running display
 ************************************************************************************/
void selfTestPowerOnLedStrip( void )
{

#define TOTAL_LED 96

#if 0
	uint32_t msb32Bit = 0;
	uint64_t lsb64Bit = 0;
	uint8_t i = 0;

	lsb64Bit = 0x01;
	msb32Bit = 0x00;


	for( i=0; i<64; i++)
	{
		WRITE_PIN( LE_GPIO_Port, LE_Pin, LOW );
		WRITE_PIN( OE_GPIO_Port, OE_Pin, LOW );
		vTaskDelay(1);

		HAL_SPI_Transmit( &ledStripSpiStruct, (uint8_t*)&lsb64Bit, (uint16_t)sizeof(lsb64Bit), 1000 );
		HAL_SPI_Transmit( &ledStripSpiStruct, (uint8_t*)&msb32Bit, (uint16_t)sizeof(msb32Bit), 1000 );

		lsb64Bit<<=1;
		vTaskDelay(1);
		WRITE_PIN( LE_GPIO_Port, LE_Pin, HIGH );
		vTaskDelay(1);


		vTaskDelay(200);
	}
*/
	lsb64Bit = 0x00;
	msb32Bit = 0x01;

/*	for( i=0; i<32; i++)
	{
		WRITE_PIN( LE_GPIO_Port, LE_Pin, LOW );
		WRITE_PIN( OE_GPIO_Port, OE_Pin, LOW );
		vTaskDelay(1);

		HAL_SPI_Transmit( &ledStripSpiStruct, (uint8_t*)&lsb64Bit, (uint16_t)sizeof(lsb64Bit), 1000 );
		HAL_SPI_Transmit( &ledStripSpiStruct, (uint8_t*)&msb32Bit, (uint16_t)sizeof(msb32Bit), 1000 );

		msb32Bit<<=1;
		vTaskDelay(1);
		WRITE_PIN( LE_GPIO_Port, LE_Pin, HIGH );
		vTaskDelay(1);


		vTaskDelay(100);
	}*/


	   newDataLedStrip.dummyStatus1 = 0;
	   newDataLedStrip.dummyStatus2 = 0;
	   newDataLedStrip.dummyStatus3 = 0;
	   newDataLedStrip.digit1Volts  = 0;
	   newDataLedStrip.digit2Volts = 0;
	   newDataLedStrip.digit3Volts = 0;
	   newDataLedStrip.digit1AmpsSpeed = 0;
	   newDataLedStrip.digit2AmpsSpeed = 0;
	   newDataLedStrip.digit3AmpsSpeed = 0x00;
	   updateLedStrip( &newDataLedStrip );
	   vTaskDelay(3000);
	   newDataLedStrip.digit3AmpsSpeed = 0x77;
	   updateLedStrip( &newDataLedStrip );
	   vTaskDelay(3000);
	   newDataLedStrip.digit3AmpsSpeed = 0x06;
	   updateLedStrip( &newDataLedStrip );
	   vTaskDelay(3000);
	   newDataLedStrip.digit3AmpsSpeed = 0x5B;
	   updateLedStrip( &newDataLedStrip );
	   vTaskDelay(3000);
	   newDataLedStrip.digit3AmpsSpeed = 0x4F;
	   updateLedStrip( &newDataLedStrip );
	   vTaskDelay(3000);
	   newDataLedStrip.digit3AmpsSpeed = 0x66;
	   updateLedStrip( &newDataLedStrip );
	   vTaskDelay(3000);
	   newDataLedStrip.digit3AmpsSpeed = 0x6D;
	   updateLedStrip( &newDataLedStrip );
	   vTaskDelay(3000);
	   newDataLedStrip.digit3AmpsSpeed = 0x7D;
	   updateLedStrip( &newDataLedStrip );
	   vTaskDelay(3000);
	   newDataLedStrip.digit3AmpsSpeed = 0x07;
	   updateLedStrip( &newDataLedStrip );
	   vTaskDelay(3000);
	   newDataLedStrip.digit3AmpsSpeed = 0x7F;
	   updateLedStrip( &newDataLedStrip );
	   vTaskDelay(3000);
	   newDataLedStrip.digit3AmpsSpeed = 0x67;
	   updateLedStrip( &newDataLedStrip );
	   vTaskDelay(3000);

#endif
	   newDataLedStrip.digit3AmpsSpeed = 0x00;
	   updateLedStrip( &newDataLedStrip );
	   vTaskDelay(3000);

}
/*************************************************************************************
 *Name  :updateLedStrip
 *Para1 : data structure pointer
 *Return: if communication success it returns true, else false.
 *Note	:
 ************************************************************************************/
bool updateLedStrip( ledStripData_t * newData )
{
	bool retValue = true;

	WRITE_PIN( LE_GPIO_Port, LE_Pin, LOW );
	WRITE_PIN( OE_GPIO_Port, OE_Pin, HIGH );
	vTaskDelay(1);
	if( HAL_OK !=  HAL_SPI_Transmit( &ledStripSpiStruct, (uint8_t*)newData, (uint16_t)sizeof(ledStripData_t), 1000 ) )
	{
		retValue = false;
	}
	vTaskDelay(1);
	WRITE_PIN( OE_GPIO_Port, OE_Pin, LOW );
	WRITE_PIN( LE_GPIO_Port, LE_Pin, HIGH );
	vTaskDelay(1);

	return( retValue );
}

/*************************************************************************************
 *Name  : updateVoltageDisplay
 *Para1 : voltage value  need to display
 *Return:
 *Note: running display
 ************************************************************************************/
bool updateVoltageDisplay( float displayVolt )
{
	bool retValue = true;


	retValue = updateSevenSegDecoder( digit, displayVolt );

	if( false != retValue)
	{
		newDataLedStrip.dummyStatus1 = 0;
		newDataLedStrip.dummyStatus2 = 0;
		newDataLedStrip.dummyStatus3 = 0;

		newDataLedStrip.digit1Volts = digit[2];
		newDataLedStrip.digit2Volts = digit[1];
		newDataLedStrip.digit3Volts = digit[0];
	}

	ledStripUpdate_f = true;

	return( retValue );
}
/*************************************************************************************
 *Name  : updateCurrentSpeedDisplay
 *Para1 : voltage value  need to display
 *Return:
 *Note: running display
 ************************************************************************************/
bool updateCurrentSpeedDisplay( float currentSpeed )
{

	bool retValue = true;


	retValue = updateSevenSegDecoder( digit, currentSpeed );

	if( false != retValue)
	{
		newDataLedStrip.dummyStatus1 = 0;
		newDataLedStrip.dummyStatus2 = 0;
		newDataLedStrip.dummyStatus3 = 0;

		newDataLedStrip.digit1AmpsSpeed = digit[2];
		newDataLedStrip.digit2AmpsSpeed = digit[1];
		newDataLedStrip.digit3AmpsSpeed = digit[0];
	}

	ledStripUpdate_f = true;

	return( retValue );
}

/*************************************************************************************
 *Name  : updateSevenSegEncode
 *Para1 : buffer to store
 *Para2 :
 *Return: if valid value return true, else false.
 *Note	: form a string with 7-segment encoder data.
 ************************************************************************************/
bool updateSevenSegDecoder( char *buf , float value )
{
	const uint8_t decode7Segment[10] = {0x77, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };		//!< This is only mapped for PS HMI board.!!!
	uint8_t temp = 0;

	uint8_t i = 0;
	uint8_t currentDig = 0;
	uint8_t maxdig = MAXIMUM_DIGIT;
	bool retValue = true;

	uint16_t wholeNum = 0;
	uint16_t fractNum = 0;


	if( ( value >= 1000 ) && ( 0 > value ) )
	{
		retValue = false;

	}else
	{
/*		if( value < 10 )			//!< I just defining resolution for my display digit/ segment
		{
			sprintf(buf, "%.2f", value );

		}else if( value < 100)
		{
			sprintf(buf, "%.1f", value );

		}else  if( ( value >= 100) && ( value < 1000 ) )
		{
			sprintf(buf, "%f", value );

		}else
		{
			sprintf(buf, "%d", 999);
		}

*/
		if( value < 10 )			//!< I just defining resolution for my display digit/ segment
		{
			fractNum = (int)(value * 100) % 100;
			wholeNum = value;
			sprintf(buf, "%d.%d", wholeNum,fractNum );

		}else if( value < 100)
		{
			fractNum = (int)(value * 10) % 10;
			wholeNum = value;
			sprintf(buf, "%d.%d",  wholeNum,fractNum );

		}else  if( ( value >= 100) && ( value < 1000 ) )
		{
			wholeNum = ( uint16_t ) value;
			sprintf(buf, "%d",  wholeNum );
		}else
		{
			sprintf(buf, "%d", 999);
		}


		for( i=0; i<maxdig; i++ )
		{
			temp =  *(buf+i);

			if( ( ( '.' == temp ) || ( ',' == temp )  )  &&  ( i != 0 ) )   //!< the character may be '.' / ',' and should not be first.
			{
				*(buf+(currentDig-1)) |= 0x80;
				maxdig++;
			}else
			{
				temp = ( temp &0x0F );
				temp = decode7Segment[temp];
				*(buf+currentDig++) = temp;
			}
		}
	}

	return retValue;
}

