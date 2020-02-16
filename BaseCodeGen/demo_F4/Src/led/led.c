/*
 * led.c
 *
 *  Created on: 22-Mar-2019
 *      Author: gmahez
 */

#include <gpioWrapper.h>

#include "led.h"
#include "sysConfigEsab.h"

#include "button.h"


/*****************************************************************************************
 * @@@@@@@@@@@@@@@@@@   LED GPIO CONFIGURATION @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
******************************************************************************************/
//!< This could be modifiable to match platform driver support.
const gpioPinConfig_t led [ MAXIMUM_LED ] =
{
        { GPIOD, { GPIO_PIN_12, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0 } },
        { GPIOD, { GPIO_PIN_13, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0 } },
        { GPIOD, { GPIO_PIN_14, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0 } },
        { GPIOD, { GPIO_PIN_15, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0 } },

};
//#define PIN_INIT( a )  { HAL_GPIO_Init(key [ a ].port, (GPIO_InitTypeDef *) &key [ a ].pinConfig); }
/*@@@@@@@@@@@@@@@@@@@@@ CONFIGURATION END @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void intLed( void );
void ledTurnOnAll( void );
void ledTurnOffAll( void );
void ledTurnOn(  eLedName_t ledName  );
void ledTurnOff(  eLedName_t ledName  );

keyReadStatus_t* mkeyDetectedPtr;

void ledTask(void const * argument)
{
    static char status = 0;
    (void) argument;            //!< Just ignore the parameter.

    intLed();               //!< Initialize Led


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
