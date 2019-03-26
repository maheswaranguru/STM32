/*
 * button.c
 *
 *  Created on: 22-Mar-2019
 *      Author: gmahez
 */
#include <stdbool.h>

#include "button.h"
#include "buttonWrapper.h"

//#include "led.h"

//!< Local function declaration
void intiButton(void);

enum
{
    SWITCH = 0,
    MAXIMUM_BUTTON
};

const digInputPinConfig_t key [ MAXIMUM_BUTTON ] =
{
{ GPIOA, { GPIO_PIN_0, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0 }}

};     // MAHESH : NEED TO RE-STRUCTURE TO COVER ALL GPIO/KEY PARAMETER.

volatile keyReadStatus_t keyStatus;

/****************************************************
 *Name :- buttonTask
 *Para1:- argument
 *Return:-N/A
 *Details:-  reset all parameters for new reading.
 *****************************************************/
void buttonTask(void const * argument)
{
    volatile uint8_t i = 0;
    volatile bool teamPinStatus;

    (void) argument;            //!< Just ignore the parameter.
    intiButton();               //!< Initialize button

    for ( ;; )
    {
        keyStatus.newStatus = 0;      //!< Reset before read all key

        for ( i = 0; i < MAXIMUM_BUTTON; i++ )      //!< Read all the keys one-by-one.
        {
            teamPinStatus = READ_PIN(key [ i ].port, key [ i ].pinConfig.Pin );

            if ( teamPinStatus )           //!< Update pin status / voltage level on corresponding bit.
            {
                keyStatus.newStatus = (1 << i);
            }
            else
            {
                keyStatus.newStatus &= ~(1 << i);
            }

        }

        if ( keyStatus.newStatus != keyStatus.currentStatus )             //!< Some key pressed now !
        {
            if ( keyStatus.latchedStatus == keyStatus.newStatus )          //!< if it was detected before
            {
                if ( KEY_DEBOUNCE_CNT <= keyStatus.debounceCnt++ )
                {
                    keyStatus.whichKey = keyStatus.currentStatus ^ keyStatus.newStatus;
                    keyStatus.currentStatus = keyStatus.latchedStatus;        //!< Update detected/accepted key

                    keyStatus.keyStat = (teamPinStatus == 1) ? PRESSED : RELEASED; // MAHESH : THIS NEED TO IMPLIMENT ONCE INITILIZATION STRUCTURE CREATED..
                                                                                   // IF NEED TO DETECT ALL KEY RELEASE ONLY... THEN MOVE THIS TO TOP WITH "if"

                    keyStatus.debounceCnt = 0;
                    keyStatus.latchedStatus = 0;
                }
            }
            else
            {
                keyStatus.latchedStatus = keyStatus.newStatus;        //!< first change detected.
            }
        }

        vTaskDelay(KEY_POLLING_INTERVEL);
    }

}
/****************************************************
 *Name :- buttonTask
 *Para1:- argument
 *Return:-N/A
 *Details:-  reset all parameters for new reading.
 *****************************************************/
void intiButton(void)
{
    // Currently nothing. NEED TO UPDATE "digInputPinConfig_t" with all gpio details.
}
