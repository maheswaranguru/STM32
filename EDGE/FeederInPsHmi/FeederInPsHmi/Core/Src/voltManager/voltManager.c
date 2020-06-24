/*
 * voltManager.c
 *
 *  Created on: Jun 24, 2020
 *      Author: gmahez
 */


#include "gpioWrapper.h"
#include "sysConfigEsab.h"
#include "voltManager.h"
#include "debugConsole.h"

volatile bool vRotA_f = false;
volatile bool vRotB_f = false;
volatile uint16_t vRotNewPosi = 0;
volatile uint16_t vRotOldPosi = 0;


void voltageMgrTask(void const * argument)
{
	(void)argument;


	for(;;)
	{
		if(vRotNewPosi != vRotOldPosi )
		{
			debugTextValue("\n VOLTAGE : ", vRotNewPosi, DECIMAL );

			vRotOldPosi = vRotNewPosi;
		}

		vTaskDelay(100);
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
	if( GPIO_Pin == GPIO_PIN_9 )
	{
		//HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8 );
		if(vRotA_f)
		{
			if(vRotNewPosi > 0)
			{
					vRotNewPosi--;
			}else
			{
				vRotNewPosi = 0;
			}
			vRotA_f = false;
			vRotB_f = false;
		}else
		{
			vRotB_f = true;
		}
	}

	if( GPIO_Pin == GPIO_PIN_11 )
	{
		//HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9 );
		if(vRotB_f)
		{
			if( vRotNewPosi < 999 )
			{
				vRotNewPosi++;
			}else
			{
				vRotNewPosi = 999;
			}

			vRotA_f = false;
			vRotB_f = false;
		}else
		{
			vRotA_f = true;
		}

	}

}
