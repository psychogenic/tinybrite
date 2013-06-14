/*

 TinyBrite Platorm -- platform conditional includes.
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

 This file is used to include the platform implementation according to
 the TinyBriteConfig.h's TINYBRITE_PLATFORM_XXX setting.


*/
#ifndef TinyBritePlatform_h
#define TinyBritePlatform_h

#include "TinyBriteConfig.h"


#include <inttypes.h>

class BaseMCU {

public:

	static void delayMs(unsigned int ms) {}
	static void delayUs(unsigned int us) {}
	static void setPinMode(uint8_t pinId, uint8_t mode) {}
	static void digitalOut(uint8_t pinId, bool value) {}

};


#include "TB_Platform_Arduino.h"
#include "TB_Platform_AVR.h"




#endif /* TinyBritePlatform_h */
