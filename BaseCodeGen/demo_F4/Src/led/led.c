/*
 * led.c
 *
 *  Created on: 22-Mar-2019
 *      Author: gmahez
 */

#include "led.h"
#include "sysConfigEsab.h"

#include "button.h"

keyReadStatus_t* mkeyDetectedPtr;

void ledTask(void const * argument)
{
	static int status = 0x0001000;
	static int count =0;


  for(;;)
  {
      while( 0 != uxQueueMessagesWaiting( gKeyDetectQ ))
      {
          if( pdPASS == xQueueReceive( gKeyDetectQ, &mkeyDetectedPtr, 0) )
          {
              GPIOD->ODR = status;
              if( count++ >= 3 )
              {
                  status = 0x0001000;
                  count = 0;
              }else
              {
                  status = (status<<1);
              }

          }
      }


	  /*
	  HAL_GPIO_TogglePin(GPIOD, LD4_Pin);
	  GPIOD = GPIOD*/
      //  vTaskDelay(500);
  }


}
