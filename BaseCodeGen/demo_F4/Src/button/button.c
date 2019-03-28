/*
 * button.c
 *
 *  Created on: 22-Mar-2019
 *      Author: gmahez
 */
#include <stdbool.h>

#include "button.h"
#include "buttonWrapper.h"
#include "queue.h"

//#include "led.h"

//!< Local function declaration
void intiButton(void);

const digInputPinConfig_t key [ MAXIMUM_BUTTON ] =

{  { GPIOA, { GPIO_PIN_0, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0 } }

};     //!< This could be modifiable for match platform driver support.

//*********** Static variable declaration *************************************
keyReadStatus_t mKeyStatus;

QueueHandle_t gKeyDetectQ;

/*********************************************************************************
 *Name :- buttonTask
 *Para1:- argument
 *Return:-N/A
 *Details:-  Main task body for button..
 **********************************************************************************/
void buttonTask(void const * argument)
{
    volatile uint8_t i = 0;
    volatile bool tempPinStatus;

    (void) argument;            //!< Just ignore the parameter.
    intiButton();               //!< Initialize button

    for ( ;; )
    {
        mKeyStatus.newStatus = 0;      //!< Reset before read all key

        for ( i = 0; i < MAXIMUM_BUTTON; i++ )      //!< Read all the keys one-by-one.
        {
            tempPinStatus = READ_PIN(key [ i ].port, key [ i ].pinConfig.Pin);

            if ( tempPinStatus )           //!< Update pin status / voltage level on corresponding bit.
            {
                mKeyStatus.newStatus = (1 << i);
            }
            else
            {
                mKeyStatus.newStatus &= ~(1 << i);
            }

        }

        if ( mKeyStatus.newStatus != mKeyStatus.currentStatus )             //!< Some key pressed now !
        {
            if ( mKeyStatus.latchedStatus == mKeyStatus.newStatus )          //!< if it was detected before
            {
                if ( KEY_DEBOUNCE_CNT <= mKeyStatus.debounceCnt++ )
                {
                    mKeyStatus.whichKey = mKeyStatus.currentStatus ^ mKeyStatus.newStatus;
                    mKeyStatus.currentStatus = mKeyStatus.latchedStatus;        //!< Update detected/accepted key

                    mKeyStatus.keyStat = (tempPinStatus == 1) ? PRESSED : RELEASED; // MAHESH : THIS NEED TO IMPLIMENT ONCE INITILIZATION STRUCTURE CREATED..
                                                                                   // IF NEED TO DETECT ALL KEY RELEASE ONLY... THEN MOVE THIS TO TOP WITH "if"
                    if ( mKeyStatus.keyStat )
                    {
                        // SEND RELEASE MESSAGE
                        xQueueSend( gKeyDetectQ, &mKeyStatus, 0);
                    }
                    else
                    {
                        // SEND PRESSED MESSAGE
                    }

                    mKeyStatus.debounceCnt = 0;
                    mKeyStatus.latchedStatus = 0;
                }
            }
            else
            {
                mKeyStatus.latchedStatus = mKeyStatus.newStatus;        //!< first change detected.
            }
        }

        vTaskDelay(KEY_POLLING_INTERVEL);
    }

}


/*********************************************************************************
 *Name :- intiButton
 *Para1:- argument
 *Return:-N/A
 *Details:-  Initialize all the button gpio pins as per the configuration structure data.
 *NOTE : THIS INITIALIZATION BLOCK MAY BE NEED CHANGE FOR DIFFERENT PLATFORM/DRIVER
 **********************************************************************************/
void intiButton(void)
{
    volatile uint8_t i = 0;

    for ( i = 0; i < MAXIMUM_BUTTON; i++ )
    {
        HAL_GPIO_Init(key [ i ].port, (GPIO_InitTypeDef *) &key [ i ].pinConfig);
    }

}


