/*

 TinyBrite Config -- compilation-time configuration
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
 This is a centralised place for configuration of compile-time options.


 */
#ifndef TinyBriteConfig_h
#define TinyBriteConfig_h

/*
 * TINYBRITE_PLATFORM_XXX
 * Defining ONE of the available
 * 	TINYBRITE_PLATFORM_XXX
 * options compiles the library for a given hardware
 * platform.
 *
 * At this time, only TINYBRITE_PLATFORM_ARDUINO (Arduino support)
 * is available.
 */
#define TINYBRITE_PLATFORM_ARDUINO
// #define TINYBRITE_PLATFORM_AVR

#ifdef TINYBRITE_PLATFORM_AVR
#include <avr/io.h>
#define F_CPU	1600000UL
#define TB_DATADIR_PORT		DDRB
#define TB_PORT				PORTB
#endif

/*
 * TA6281_XXX_DELAY_US sets the time to allow for the
 * XXX (CLOCK or LATCH) signal to get through, in microseconds.
 */
#define TA6281_CLOCK_DELAY_US  		20
#define TA6281_LATCH_DELAY_US  		30



/*
 * TA6281_STATE_TRACKING_ENABLE
 *
 * The library can track current state, both to report back on the
 * state of a given A6281 chip and to allow for state save/restore
 * functions.
 *
 * This involves extra code and the use of dynamic memory allocation
 * (malloc/free), so takes up extra space.  If you're really tight
 * on flash space, and don't need the functionality, you can
 * disable this stuff by commenting out the
 * 	TA6281_STATE_TRACKING_ENABLE
 * define, below.
 */
#define TA6281_STATE_TRACKING_ENABLE


/*
 * TA6281_STATE_TRACKING_BIGNUM
 *
 * If you have > 255 'brites controlled by this library,
 * you must define
 *
 * 	TA6281_STATE_TRACKING_BIGNUM
 *
 * in order to index the number of drivers correctly.  If you
 * are controlling less than 256 'Brites, leave this undefined.
 *
 */
//#define TA6281_STATE_TRACKING_BIGNUM

/*
 * TA6281_DEFAULT_XXX
 * Sets the default pin for data, nEnable, clock and latch.
 *
 * You'll normally configure these with setup(), but the values
 * here set sane defaults.
 */
#define TA6281_DEFAULT_DATAPIN  	0
#define TA6281_DEFAULT_NENABLEPIN	1
#define TA6281_DEFAULT_CLOCKPIN  	2
#define TA6281_DEFAULT_LATCHPIN 	3




#endif /* TinyBriteConfig_h */
