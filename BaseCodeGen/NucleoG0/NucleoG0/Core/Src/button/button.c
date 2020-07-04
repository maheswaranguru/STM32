/*
 * button.c
 *
 *  Created on: 22-Mar-2019
 *      Author: gmahez
 */
#include <gpioWrapper.h>
#include <stdbool.h>

#include "button.h"
#include "queue.h"

#ifdef PUSH_BUTTON
/*****************************************************************************************
 * @@@@@@@@@@@@@@@@@@   Button GPIO CONFIGURATION @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 * Note : This structure May be differs depends on platform used.
******************************************************************************************/
const gpioPinConfig_t button [ MAXIMUM_BUTTON ] =
{
	//  	Key PORT	  Pin No
        { 	GPIOC, 		{ GPIO_PIN_13, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0 }, 	HIGH, 	RELEASED }
};     //!< Note : This could be modifiable for match platform driver support.
/*@@@@@@@@@@@@@@@@@@@@@ CONFIGURATION END @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/


//!< Local function declaration
void intiButton(void);
void buttonRead(void);

//!< Static variable declaration
buttonReadStatus_t mButtonStatus;     //!< structure to store key status and change in key status.
keyMsgData_t keyMsgData;

#endif

#ifdef MATRIX_KEYBOARD

void initKeyPad( void );
void keypadRead( void );

#endif

#ifdef EXTERNAL_CHIP_READ

void externalchipkeyRead( void );

#endif


QueueHandle_t gKeyDetectQ;      //!< Message Queue for key pressed

/*********************************************************************************
 *Name :- buttonTask
 *Para1:- argument
 *Return:-N/A
 *Details:-  Main task body for button..
 **********************************************************************************/
void buttonTask(void const * argument)
{

    (void) argument;            //!< Just ignore the parameter.

#ifdef PUSH_BUTTON
    intiButton();               //!< Initialize button
#endif

    for ( ;; )
    {

#ifdef PUSH_BUTTON
    	buttonRead();
#endif


#ifdef MATRIX_KEYBOARD
    	keypadRead();
#endif

#ifdef EXTERNAL_CHIP_READ
    	externalchipkeyRead();
#endif


        vTaskDelay(KEY_POLLING_INTERVEL);
    }

}

#ifdef PUSH_BUTTON
/*********************************************************************************
 *Name :- buttonRead
 *Para1:- N/A
 *Return:-N/A
 *Details:-  read configured buttons.
 **********************************************************************************/

void buttonRead( void )
{

    uint8_t i = 0;
    mButtonStatus.newStatus = 0;      //!< Reset before read all key
    bool tempPinStatus;

    for ( i = CLEAR; i < MAXIMUM_BUTTON; i++ )      //!< Read all the keys one-by-one.
    {
        tempPinStatus = READ_PIN(button [ i ].port, button [ i ].pinConfig.Pin);

        if ( tempPinStatus )           //!< Update pin status / voltage level on corresponding bit.
        {
            mButtonStatus.newStatus = (SET << i);
        }
        else
        {
            mButtonStatus.newStatus &= ~(SET << i);
        }



		if ( mButtonStatus.newStatus != mButtonStatus.currentStatus )             //!< Some key pressed now !
		{
			if ( mButtonStatus.latchedStatus == mButtonStatus.newStatus )          //!< if it was detected before
			{
				if ( KEY_DEBOUNCE_CNT <= mButtonStatus.debounceCnt++ )
				{
					mButtonStatus.whichButton = mButtonStatus.currentStatus ^ mButtonStatus.newStatus;
					mButtonStatus.currentStatus = mButtonStatus.latchedStatus;        //!< Update detected/accepted key


					//mButtonStatus.keyStat = (tempPinStatus == key[i].defaultState ) ? PRESSED : RELEASED;   //!< Hard coded detection of key.

					if ( button[i].buttonDetectStat == tempPinStatus )		//!< check we got required detection.
					{
						keyMsgData.whichButton = 0;

							do
							{
								mButtonStatus.whichButton>>=1;
							}while ( RESET != mButtonStatus.whichButton );

						// SEND RELEASE MESSAGE
						xQueueSend( gKeyDetectQ, &mButtonStatus, CLEAR);       //!< Send Message immediately, ELSE handle it separately as per application need.
					}

					mButtonStatus.debounceCnt = 0;
					mButtonStatus.latchedStatus = 0;
				}
			}
			else
			{
				mButtonStatus.latchedStatus = mButtonStatus.newStatus;        //!< first change detected.
			}
		}

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
    uint8_t i = 0;

    for ( i = 0; i < MAXIMUM_BUTTON; i++ )
    {
        PIN_INIT( (button [ i ].port), &(button[i].pinConfig));      //!< Macro defined in buttonWrapper.h. May be need to change if required.
    }
}


#endif

#ifdef MATRIX_KEYBOARD
void initKeyPad( void )
{
    uint8_t i = 0;

    for ( i = 0; i < MAXIMUM_BUTTON; i++ )
    {
       // PIN_INIT( (key [ i ].port), &(key[i].pinConfig));      //!< Macro defined in buttonWrapper.h. May be need to change if required.
    }

}
void keypadRead( void )
{

}
#endif


#ifdef EXTERNAL_CHIP_READ
/*********************************************************************************
 *Name :- externalchipkeyRead
 *Para1:- argument
 *Return:-N/A
 *Details:-  Read the Initialize the External GPIO expander Chip.
 *NOTE : THIS INITIALIZATION BLOCK MAY BE NEED CHANGE FOR DIFFERENT PLATFORM/DRIVER
 **********************************************************************************/
void externalchipkeyRead( void )
{

	return;

}

void initExternalKeyReadChip( void )
{

}

#endif
