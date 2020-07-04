/*
 * led.c
 *
 *  Created on: 22-Mar-2019
 *      Author: gmahez
 */

#include <gpioWrapper.h>

#include "led.h"

#include "button.h"


/*****************************************************************************************
 * @@@@@@@@@@@@@@@@@@   LED GPIO CONFIGURATION @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
******************************************************************************************/
//!< This could be modifiable to match platform driver support.
const gpioPinConfig_t led [ MAXIMUM_LED ] =
{
        { GPIOA, { GPIO_PIN_5, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0 } },

};
//#define PIN_INIT( a )  { HAL_GPIO_Init(key [ a ].port, (GPIO_InitTypeDef *) &key [ a ].pinConfig); }
/*@@@@@@@@@@@@@@@@@@@@@ CONFIGURATION END @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void intLed( void );
void ledTurnOnAll( void );
void ledTurnOffAll( void );
void ledTurnOn(  eLedName_t ledName  );
void ledTurnOff(  eLedName_t ledName  );

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#define LEDSTRIP_SPI_PORT SPI1

void selfTestPowerOnLedStrip( void );

SPI_HandleTypeDef ledStripSpiStruct;


//#define SEND_BYTE(a)  { HAL_SPI_Transmit_IT( &LEDSTRIP_SPI_PORT, )

ledStripData_t ledStripData;
const uint8_t ledStripDataSize = sizeof( ledStripData_t );


void intLedStrip( void );
void updateLedStrip( void );


SPI_HandleTypeDef ledStripSpiPort;
ledStripData_t ledStripData;


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

keyMsgData_t mkeyDetectedPtr;

void ledTask(void const * argument)
{
    static char status = 0;
    (void) argument;            //!< Just ignore the parameter.

    intLed();               //!< Initialize Led
    intLedStrip();


  for(;;)
  {
      while( 0 != uxQueueMessagesWaiting( gKeyDetectQ ))
      {
          if( pdPASS == xQueueReceive( gKeyDetectQ, &mkeyDetectedPtr, 0) )
          {

              if( 0 == status )
              {
                  status = 1;
                  ledTurnOnAll();
              }else
              {
                  status = 0;
                  ledTurnOffAll();
              }
          }
      } //!< While for msg Rx
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
	  ledStripSpiStruct.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	  ledStripSpiStruct.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
	  if (HAL_SPI_Init(&ledStripSpiStruct) != HAL_OK)
	  {
	    while(1);
	  }else
	  {
		  selfTestPowerOnLedStrip();
	  }
}
/*************************************************************************************
 *Name  :ledTurnOff
 *Para1 :N/A
 *Return:N/A
 *Note: Turn OFF all LEDs
 ************************************************************************************/
void updateLedStrip( void )
{

	uint8_t *ledStripDataPtr = (uint8_t *) &ledStripData;

	HAL_SPI_Transmit_IT( &ledStripSpiStruct, ledStripDataPtr, ledStripDataSize );

}
/*************************************************************************************
 *Name  :selfTestPowerOnLedStrip
 *Para1 :N/A
 *Return:N/A
 *Note: running display
 ************************************************************************************/
void selfTestPowerOnLedStrip( void )
{

#define TOTAL_LED 108

	uint64_t msb64Bit = 0;
	uint64_t lsb64Bit = 0;
	uint8_t i = 0;

	lsb64Bit = 0x01;
	for(i=0; i<64; i++)
	{
		HAL_SPI_Transmit( &ledStripSpiStruct, (uint8_t* ) &lsb64Bit, (uint16_t)sizeof(lsb64Bit), 1000 );
		lsb64Bit<<=1;
		HAL_SPI_Transmit( &ledStripSpiStruct, (uint8_t* ) &msb64Bit, (uint16_t)sizeof(msb64Bit), 1000  );

		vTaskDelay(1000);
	}

	msb64Bit = 0x01;
	for(i=0; i<(TOTAL_LED-64); i++)
	{
		HAL_SPI_Transmit( &ledStripSpiStruct, (uint8_t *) &lsb64Bit, (uint16_t)sizeof(lsb64Bit), 1000  );
		HAL_SPI_Transmit( &ledStripSpiStruct, (uint8_t *) &msb64Bit, (uint16_t)sizeof(msb64Bit), 1000  );
		msb64Bit<<=1;

		vTaskDelay(1000);
	}


}
