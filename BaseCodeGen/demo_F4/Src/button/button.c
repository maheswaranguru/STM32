/*
 * button.c
 *
 *  Created on: 22-Mar-2019
 *      Author: gmahez
 */
#include <stdbool.h>

#include "button.h"
#include "buttonWrapper.h"

#include "led.h"


enum{
    SWITCH = 0,
    //BLUE_SWITCH,
    MAXIMUM_BUTTON
};

const digInputPinConfig_t key[MAXIMUM_BUTTON]= { {GPIOA, GPIO_PIN_0} };

volatile keyReadStatus_t keyStatus;

void buttonTask(void const * argument)
{
uint8_t i = 0;
volatile bool teamPinStatus;

  for(;;)
  {
      keyStatus.newStatus = 0;      //!< Reset before read all key

      for( i=0; i<MAXIMUM_BUTTON; i++)      //!< Read all the keys one-by-one.
      {
          teamPinStatus = READ_PIN( key[i].port, key[i].pin );

          if( teamPinStatus )
          {
              keyStatus.newStatus = (1<< i);
          }else
          {
              keyStatus.newStatus &= ~(1<< i);
          }

      }

      if( keyStatus.newStatus != keyStatus.currentStatus  )             //!< Some key pressed now !
      {
          if( keyStatus.latchedStatus == keyStatus.newStatus )          //!< if it was detected before
          {
              if( KEY_DEBOUNCE_CNT <= keyStatus.debounceCnt ++ )
              {
                  keyStatus.debounceCnt = 0;
                  keyStatus.currentStatus = keyStatus.latchedStatus;
                  keyStatus.latchedStatus = 0;
              }
          }
      }


    osDelay(1);
  }

}
