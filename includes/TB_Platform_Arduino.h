/*

 TinyBrite Arduino Platorm -- platform implementation for Arduino devices.
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

 This file is used to include Arduino-specific headers and define the
 MCU (microcontroller abstraction) class for use with Arduinos.

 This is to keep all platform specifics in a single place, to ease
 expansion of the library to support other uC environments.


*/

#ifndef TB_Platform_Arduino_h
#define TB_Platform_Arduino_h

#include "TinyBriteConfig.h"

#ifdef TINYBRITE_PLATFORM_ARDUINO

#include <inttypes.h>


// include the Arduino header, to use functions like pinMode and digitalWrite
#include "Arduino.h"
/* class MCU -- abstract away platform
 * This class simply acts as a centralised place to keep all our uC-specific functions.
 */
class MCU : public BaseMCU {

public:

	static void delayMs(unsigned int ms) { delay(ms); }
	static void delayUs(unsigned int us) { delayMicroseconds(us); }
	static void setPinMode(uint8_t pinId, uint8_t mode) { pinMode(pinId, mode); }
	static void digitalOut(uint8_t pinId, bool value) { digitalWrite(pinId, value); }

};

#endif /* TINYBRITE_PLATFORM_ARDUINO */

#endif /* TB_Platform_Arduino_h */


