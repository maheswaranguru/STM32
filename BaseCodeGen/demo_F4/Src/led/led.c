/*
 * led.c
 *
 *  Created on: 22-Mar-2019
 *      Author: gmahez
 */

#include "led.h"


void ledTask(void const * argument)
{
	static int status = 0;


  for(;;)
  {
	  if ( status )
		{
		  GPIOD->ODR |= 0x000F000;

		}else
		{
			GPIOD->ODR  &= ~(0x000F000);
		}
	  status = !status;
	  /*
	  HAL_GPIO_TogglePin(GPIOD, LD4_Pin);
	  GPIOD = GPIOD*/
	  osDelay(500);
  }


}
