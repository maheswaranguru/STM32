/*
 * application.c
 *
 *  Created on: 22-Mar-2019
 *      Author: gmahez
 */

#include "application.h"
#include "button.h"
#include "led.h"

/* Private function prototypes -----------------------------------------------*/
void StartDefaultTask(void const * argument);

/* Private variables ---------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId ledHandle;
osThreadId buttonHandle;

void application( void )
{

	 /* Create the thread(s) */
	  /* definition and creation of defaultTask */
	  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
	  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

	  /* definition and creation of led */
	  osThreadDef(led, ledTask, osPriorityIdle, 0, 128);
	  ledHandle = osThreadCreate(osThread(led), NULL);

	  /* definition and creation of button */
	  osThreadDef(button, buttonTask, osPriorityIdle, 0, 128);
	  buttonHandle = osThreadCreate(osThread(button), NULL);



	  /* Start scheduler */
	  osKernelStart();

	  while(1);
}

/***********************************************************
 *  USER CODE BEGIN Header_StartDefaultTask */
/***********************************************************
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  ***********************************************************/

void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}


