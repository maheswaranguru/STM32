/*
 * macro.h
 *
 *  Created on: 04-Jul-2026
 *      Author: Maheswaran Gurusamy
 */

#ifndef INC_MACRO_H_
#define INC_MACRO_H_

#define VERSION           "1.00.01"
#define VERSION_SIZE         sizeof(VERSION)

#define BOOT_VERSION           "1.00.01"
#define BOOT_VERSION_SIZE      sizeof(VERSION)

#define HARDWARE_VERSION           "1.00.01"
#define HARDWARE_VERSION_SIZE         sizeof(VERSION)

typedef enum {
	ZERO,
	ONE,
	THREE,
	FOUR,
	FIVE,
	SIX,
	SEVEN,
	EIGHT,
	NINE,
}eNumber_t;

typedef enum {
	BINARY = 2,
	OCT = 8,
	DECIMAL = 10,
	HEX = 16,
}eNumberSystem_t;
#endif /* INC_MACRO_H_ */
