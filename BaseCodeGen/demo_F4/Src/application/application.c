/*
 * application.c
 *
 *  Created on: 22-Mar-2019
 *      Author: gmahez
 */
#include "freertos.h"
#include "task.h"
#include "queue.h"


#include "application.h"
#include "button.h"
#include "led.h"
#include "debugConsole.h"


void debugconsoleTask(void);

/* Private Constant -----------------------------------------------*/
const sysTask_t sysTask[] =
{
    { (TaskFunction_t) ledTask,             "Led",              128, 0, osPriorityNormal, NULL },
    { (TaskFunction_t) buttonTask,          "button",           128, 0, osPriorityNormal, NULL },
    { (TaskFunction_t) debugconsoleTask,    "debugconsole",     128, 0, osPriorityNormal, NULL }
};

/* Private function prototypes -----------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId ledHandle;
osThreadId buttonHandle;

TaskHandle_t xHandle = NULL;


void application(void)
{
    volatile uint8_t i = 0;
    volatile uint8_t TaskStatus = 0;

    for( i = 0; i< MAXMUM_TASK; i++)
    {
        if( pdPASS !=  xTaskCreate(sysTask[i].vTaskfunPtr, sysTask[i].vTaskName, sysTask[i].stacksize, sysTask[i].VTaskparaeter, sysTask[i].VtaskPriority, sysTask[i].pxCreatedTask) )
        {
            TaskStatus++;
        }
    }

    if( 0 != TaskStatus )       //!< If any of the task not created successfully, freeze here !
    {
        while(1);
    }

    gKeyDetectQ = xQueueCreate( 3, sizeof(keyReadStatus_t));    //!< 3 number of keys can stored inside.
    if( NULL == gKeyDetectQ )
    {
        while(1);
    }

    xMutexDebugUart = xSemaphoreCreateMutex();
    if( NULL == xMutexDebugUart )
    {
        // TURN ON ERROR LED;
    }

    /* Start scheduler */
    osKernelStart();

}

