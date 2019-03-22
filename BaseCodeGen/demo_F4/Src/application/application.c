/*
 * application.c
 *
 *  Created on: 22-Mar-2019
 *      Author: gmahez
 */

#include "application.h"
#include "button.h"
#include "led.h"
#include "freertos.h"
#include "task.h"

void debugconsoleTask(void);

/* Private Constant -----------------------------------------------*/
sysTask_t sysTask[] =
{
    { (TaskFunction_t) ledTask, "Led", 128, 0, osPriorityNormal, NULL },
    { (TaskFunction_t) buttonTask, "button", 128, 0, osPriorityNormal, NULL },
    { (TaskFunction_t) debugconsoleTask, "debugconsole", 128, 0, osPriorityNormal, NULL }
};

/* Private function prototypes -----------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId ledHandle;
osThreadId buttonHandle;

TaskHandle_t xHandle = NULL;

void application(void)
{
    volatile static uint8_t TaskStatus = 1;

    /* Create the thread(s) */
    TaskStatus = xTaskCreate(sysTask[LED_TASK].vTaskfunPtr, sysTask[LED_TASK].vTaskName, sysTask[LED_TASK].stacksize, sysTask[LED_TASK].VTaskparaeter, sysTask[LED_TASK].VtaskPriority, sysTask[LED_TASK].pxCreatedTask);
    TaskStatus = xTaskCreate(sysTask[BUTTON_TASK].vTaskfunPtr, sysTask[BUTTON_TASK].vTaskName, sysTask[BUTTON_TASK].stacksize, sysTask[BUTTON_TASK].VTaskparaeter, sysTask[BUTTON_TASK].VtaskPriority, sysTask[BUTTON_TASK].pxCreatedTask);
    TaskStatus = xTaskCreate(sysTask[CONSOLE_TASK].vTaskfunPtr, sysTask[CONSOLE_TASK].vTaskName, sysTask[CONSOLE_TASK].stacksize, sysTask[CONSOLE_TASK].VTaskparaeter, sysTask[CONSOLE_TASK].VtaskPriority, sysTask[CONSOLE_TASK].pxCreatedTask);

    /* Start scheduler */
    osKernelStart();

}

void debugconsoleTask(void)
{
    for (;;)
    {
        vTaskDelay(1);
    }
}
