/*
 * voltManager.h
 *
 *  Created on: Jun 24, 2020
 *      Author: gmahez
 */

#ifndef SRC_ENCODER_ENCODER_H_
#define SRC_ENCODER_ENCODER_H_

#include "sysConfigEsab.h"

typedef struct encoder
{

	volatile bool RotA_f;
	volatile bool RotB_f;
	volatile uint16_t RotNewPosi;
	volatile uint16_t RotOldPosi;
}enoder_t;

typedef enum
{
	volt = 0,
	current,
	MAXIMUM_ENCODER
} eEncoderName_t;

void voltageMgrTask(void const * argument);

#endif /* SRC_ENCODER_ENCODER_H_ */
