/*
 * buttonWrapper.h
 *
 *  Created on: 23-Mar-2019
 *      Author: gmahez
 */

#ifndef BUTTON_BUTTONWRAPPER_H_
#define BUTTON_BUTTONWRAPPER_H_

#include "stm32f4xx_hal.h"
#include "button.h"

#define READ_PIN(a,b)       HAL_GPIO_ReadPin((GPIO_TypeDef*)(a), (uint16_t) b )
#define WRITE_PIN(a,b,c )   HAL_GPIO_WritePin(GPIO_TypeDef* a, uint16_t b, GPIO_PinState c)
#define READ_PORT( a )      esab_stmF4HAL_ReadPort( GPIO_TypeDef* GPIOx )


#define PIN_INIT( a )        { HAL_GPIO_Init(key [ a ].port, (GPIO_InitTypeDef *) &key [ a ].pinConfig); }

typedef struct
{
    GPIO_TypeDef*   port;
    GPIO_InitTypeDef pinConfig;
}digInputPinConfig_t;




const digInputPinConfig_t key [ MAXIMUM_BUTTON ] =

{  { GPIOA, { GPIO_PIN_0, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0 } }

};     //!< This could be modifiable for match platform driver support.

#endif /* BUTTON_BUTTONWRAPPER_H_ */
