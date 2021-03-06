/*
 * voltManager.c
 *
 *  Created on: Jun 24, 2020
 *      Author: gmahez
 */


#include "encoder.h"

#include "gpioWrapper.h"
#include "sysConfigEsab.h"
#include "debugConsole.h"

#include "led.h"

#define ENCODER_UPDATE_TIME 50

uint16_t updateTime = 0;

enoder_t encoder[MAXIMUM_ENCODER] = { 0 };

void voltageMgrTask(void const * argument)
{
	(void)argument;


	for(;;)
	{

		if( ENCODER_UPDATE_TIME <= updateTime++)
		{
			updateTime = CLEAR;

			if(encoder[volt].RotNewPosi != encoder[volt].RotOldPosi )
			{
				updateVoltageDisplay( (float)(encoder[volt].RotNewPosi ) );
				debugTextValue("\n VOLTAGE : ", encoder[volt].RotNewPosi, DECIMAL );

				encoder[volt].RotOldPosi = encoder[volt].RotNewPosi;
			}

			if(encoder[current].RotNewPosi != encoder[current].RotOldPosi )
			{
				updateCurrentSpeedDisplay( (float)(encoder[current].RotNewPosi ) );
				debugTextValue("\n\t\t\tCURRENT : ", encoder[current].RotNewPosi, DECIMAL );

				encoder[current].RotOldPosi = encoder[current].RotNewPosi;
			}

		}

		vTaskDelay(1);
	}


}

/*************************************************************************************
 *Name  :intLed
 *Para1 :N/A
 *Return:N/A
 *Note: Initialize all LED pins configured on 'led' structure.
 ************************************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if( GPIO_Pin == GPIO_PIN_11 )		//!< this one need to choose carefully.
	{
		//HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8 );
		if(encoder[volt].RotA_f)
		{
			if(encoder[volt].RotNewPosi > 0)
			{
				encoder[volt].RotNewPosi--;		//!< clockwise OR anti-clockwise.
			}else
			{
				encoder[volt].RotNewPosi = 0;
			}
			encoder[volt].RotA_f = false;
			encoder[volt].RotB_f = false;
		}else
		{
			encoder[volt].RotB_f = true;
		}
	}

	if( GPIO_Pin == GPIO_PIN_9 )
	{
		//HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9 );
		if(encoder[volt].RotB_f)
		{
			if( encoder[volt].RotNewPosi < 999 )
			{
				encoder[volt].RotNewPosi++;
			}else
			{
				encoder[volt].RotNewPosi = 999;
			}

			encoder[volt].RotA_f = false;
			encoder[volt].RotB_f = false;
		}else
		{
			encoder[volt].RotA_f = true;
		}
	}


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@      VOLTAGE END     @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

	if( GPIO_Pin == GPIO_PIN_13 )		//!< this one need to choose carefully.
	{
		//HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8 );
		if(encoder[current].RotA_f)
		{
			if(encoder[current].RotNewPosi > 0)
			{
				encoder[current].RotNewPosi--;		//!< clockwise OR anti-clockwise.
			}else
			{
				encoder[current].RotNewPosi = 0;
			}
			encoder[current].RotA_f = false;
			encoder[current].RotB_f = false;
		}else
		{
			encoder[current].RotB_f = true;
		}
	}

	if( GPIO_Pin == GPIO_PIN_14 )
	{
		//HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9 );
		if(encoder[current].RotB_f)
		{
			if( encoder[current].RotNewPosi < 999 )
			{
				encoder[current].RotNewPosi++;
			}else
			{
				encoder[current].RotNewPosi = 999;
			}

			encoder[current].RotA_f = false;
			encoder[current].RotB_f = false;
		}else
		{
			encoder[current].RotA_f = true;
		}
	}
	/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@      CURRENT END     @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

}
