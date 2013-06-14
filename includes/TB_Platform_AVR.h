/*

 TinyBrite AVR Platorm -- platform implementation for AVR devices.
 Copyright (C) 2013 Pat Deegan.  All rights reserved.


 http://www.flyingcarsandstuff.com/projects/tinybrite/


 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


 See file LICENSE.txt for further informations on licensing terms.

 *****************************  OVERVIEW  *****************************

 This file is used to include AVR-specific headers and define the
 MCU (microcontroller abstraction) class for use with AVRs.

 This is to keep all platform specifics in a single place, to ease
 expansion of the library to support other uC environments.


*/

#ifndef TB_Platform_AVR_h
#define TB_Platform_AVR_h

#include "TinyBriteConfig.h"

#ifdef TINYBRITE_PLATFORM_AVR

#include <util/delay.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef NULL
#define NULL 0x0
#endif

#ifndef LOW
#define LOW 0x0
#endif

#ifndef HIGH
#define HIGH 0x1
#endif

#ifndef INPUT
#define INPUT 0x0
#endif

#ifndef OUTPUT
#define OUTPUT 0x1
#endif

// include the AVR header, to use functions like pinMode and digitalWrite

/* class MCU -- abstract away platform
 * This class simply acts as a centralised place to keep all our uC-specific functions.
 */
class MCU : public BaseMCU {

public:

	static void delayMs(unsigned int ms) {
		_delay_ms(ms);
	}
	static void delayUs(unsigned int us) {

		_delay_us(us);
	}
	static void setPinMode(uint8_t pinId, uint8_t mode) {
		if (mode)
		{
			TB_DATADIR_PORT |= (1 << pinId);
		} else {
			TB_DATADIR_PORT &= ~(1 << pinId);
		}

	}
	static void digitalOut(uint8_t pinId, bool value)
	{
		if (value)
		{
			TB_PORT |= (1 << pinId);
		} else {
			TB_PORT &= (0xff & ~(1 << pinId));
		}
	}

};

#endif /* TINYBRITE_PLATFORM_AVR */

#endif /* TB_Platform_AVR_h */


