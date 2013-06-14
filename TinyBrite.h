/*

 TinyBrite -- a library to speak to *Brite and other A6281-based devices.
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
 
 The TinyBrite library may be used to control one or more daisy-chained 
 *Brites (can be ShiftBrite, MegaBrite or anything that uses the A6281 
 PWM LED driver).
 
 It "bit-bangs" data to the drivers, meaning you don't need SPI support
 (only 3 pins to use as digital outputs), and is specifically designed
 with programmable RGB LEDs so it understands
 
 
 *****************************  HW SETUP  *****************************  
 
 The library expects an Arduino-compatible device tied to a
 chain (one or more) of *Brites (can be ShiftBrite, MegaBrite) or anything
 that uses the A6281 PWM LED driver.
 
 Basic setup (power connections not shown)


    Arduino                   *Brite device
 +===========+pin A           +============+ DO (data out)
 |           |----------------|DI (data)   |-------------->
 | Digispark |                |            | EO (enable out)
 |           |          ------|EI (enable) |-------------->
 |           |          |     |            |
 |           |         GND    |            |
 |           |pin B           |            | CO (clock out)
 |    Any    |----------------|CI (clock)  |-------------->  Another *Brite...
 |    Type   |pin C           |            | LO (latch out)
 |   *duino  |----------------|LI (latch)  |-------------->
 +===========+                +============+
 
 
 *** NOTE ***: you may want to use pull up resistors on the 
 clock and data pins (pins A and B, above).
 This just means putting a (few k) resistor between +5V and 
 each pin. 
 
 *****************************  INFO  *****************************  
 
 This library was developed and run on a Digistump digispark, a pretty
 sweet and tiny Arduino-compatible board based on an Atmel ATtiny85, 
 but should be usable on any *duino which you want to use to control
 a *Brite  (MegaBrite, ShiftBrite, etc) but don't have access to SPI
 to do so (such as on the '85).
 
 To use it, get a MegaBrite or ShiftBrite driver+LED and dedicate 3
 (or 4, if you want to use the ~ENABLE) pins:
 * a data pin (sends data to the underlying A6281 driver)
 * a clock pin (clocks in the data)
 * a latch pin (tells the driver the data has arrived and should be
 taken into account).
 * optionally, an enable pin (to activate or suspend the chain)
 
 The BriteChain example (in Examples -> TinyBrite -> BriteChain)
 contains commentary to clarify how the library is configured 
 and used.
 
 
 
 *****************************  BASIC USAGE  *****************************  
 Basic usage involves:
 
 // creating an instance
 TinyBrite brite_chain(3); // three daisy-chained *brites
 
 // send colors/packets to all brites in an "update cycle"
 // an update cycle consists of 
 //  * a call to beginUpdate()
 //  * sending packets/information
 //  * a call to endUpdate() so the changes will take effect, e.g.
 
 brite_chain.beginUpdate(); // start an update cycle
 brite_chain.sendColor(0, 0, TINYBRITE_COLOR_MAXVALUE); // max blue
 brite_chain.sendColor(0, TINYBRITE_COLOR_MAXVALUE, 0); // max green
 brite_chain.sendColor(TINYBRITE_COLOR_MAXVALUE, 0, 0); // max red
 brite_chain.endUpdate(); // all changes take effect
 
 // and that's pretty much it!
 // See the BriteChain example (in Examples -> TinyBrite -> BriteChain)
 // for all the details.

 *** Auto update cycle ***
 Also, if you don't want to do anything fancy, and you just want to send color commands 
 with the most ease, you can pass a second optional "auto-update" boolean flag to the 
 constructor:

 TinyBrite brite_chain(3, TINYBRITE_AUTOUPDATE_ENABLE); // three daisy-chained *brites, auto-updated.

 // now, every time you call sendColor or sendPacket*, the data will immediately be latched
 // i.e. no need for begin/endUpdate:

 brite_chain.sendColor(0, 0, TINYBRITE_COLOR_MAXVALUE); // blue, color takes effect immediately


 // later
 brite_chain.sendColor(TINYBRITE_COLOR_MAXVALUE, 0, 0);
 // red takes effect immediately, blue shifted down the chain.
 

 // Finally, you can manage the auto-update cycle handling any time using the
 // autoUpdate() (read) and setAutoUpdate(bool) methods.

 Enjoy,
 Pat Deegan, psychogenic.com
*/

#ifndef TinyBrite_h
#define TinyBrite_h

#include "includes/TinyBriteConfig.h"
#include "includes/TinyA6281.h"

#define TINYBRITE_VERSION		1.0

#define TINYBRITE_COLOR_MAXVALUE		TA6281_PWM_MAXVALUE

#define TINYBRITE_CORRECTION_MAXVALUE	TA6281_CORRECTION_MAXVALUE
#define TINYBRITE_COMMAND_CLOCK_800kHz	TA6281_COMMAND_CLOCK_800kHz
#define TINYBRITE_COMMAND_CLOCK_400kHz	TA6281_COMMAND_CLOCK_400kHz
#define TINYBRITE_COMMAND_CLOCK_200kHz	TA6281_COMMAND_CLOCK_200kHz
#define TINYBRITE_COMMAND_CLOCK_EXT		TA6281_COMMAND_CLOCK_EXT

#define TINYBRITE_AUTOUPDATE_ENABLE		TA6281_AUTOUPDATE_ENABLE
#define TINYBRITE_AUTOUPDATE_DISABLE	TA6281_AUTOUPDATE_DISABLE


#define TINYBRITE_PACKETMODE_COLOR			TA6281_MODE_PWM
#define TINYBRITE_PACKETMODE_COMMAND		TA6281_MODE_CORRECT
/* 
 ** Please ignore the man behind the curtain...
 **
 ** The BritePackets are used internally and their structure can be ignored.
 ** They *can* be created and handled manually, but the idea is to
 ** treat them as opaque and use the TinyBrite instances colorPacket/commandPacket methods
 ** to create them.
 **
 ** In fact, you can completely forget them altogether if you use sendColor()/sendCommand() instead.
 */
typedef union BritePacket {
	unsigned long value;

	struct {
		unsigned green :10;
		unsigned red :10;
		unsigned blue :10;
		unsigned mode_pwm :1;
	};

	struct {
		unsigned greenDotCorrect :7;
		unsigned clockMode :2;
		unsigned :1;
		unsigned redDotCorrect :7;
		unsigned :3;
		unsigned blueDotCorrect :7;
		unsigned :1;
		unsigned atb0 :1;
		unsigned atb1 :1;
		unsigned mode_correct :1;
	};
} BritePacket;


/*  BritePacket is basically a redefinition of the A6281Packet, used to make things more
 ** natural in the context of the MegaBrite (e.g. using "green" rather than "pwm_0").
 ** This creates some overhead but, for clarity's sake... them's the breaks.
 */

/*
 ** TinyBrite class.
 **
 ** Derived from the TinyA6281 class, the TinyBrite class is used to provide *Brite-specific
 ** functions and handle color-related notions.
 **
 ** It actually does very little other than rename functions and datatypes to make use of the
 ** TinyA6281's PWMs more natural in the context of ShiftBrite and MegaBrite chips.
 **
 ** See TinyA6281.h and Examples -> TinyBrite -> BriteChain for details.
 */
typedef unsigned int	TinyBriteColorValue;

class TinyBrite: public TinyA6281

{

public:

	/* 
	 ** TinyBrite constructor.
	 ** Call with the number of *Brites chained together.
	 */
	TinyBrite(uint8_t num_brites = 1, bool auto_update_cycle =
			TINYBRITE_AUTOUPDATE_DISABLE);

	/*  SETUP (method from base class)
	 ** setup -- NO ~enable pin used
	 ** Register and configure the pins used for data, clock and latch using
	 ** setup(uint8_t datapin, uint8_t clockpin, uint8_t latchpin)
	 **
	 ** setup -- ~enable pin used
	 ** Register and configure the pins used for data, clock, latch and ~enable using
	 ** setup(uint8_t datapin, uint8_t clockpin, uint8_t latchpin,
	 **		uint8_t nEnablepin);
	 **
	 */


	// a few class methods to create packets
	/*
	 ** colorPacket
	 ** Create a valid color data packet.
	 */
	static BritePacket colorPacket(TinyBriteColorValue red, TinyBriteColorValue green,
			TinyBriteColorValue blue);

	/*
	 ** commandPacket
	 ** Create a valid command data packet.
	 */
	static BritePacket commandPacket(unsigned int redCorrect0,
			unsigned int greenCorrect1, unsigned int blueCorrect2,
			unsigned char clockMode);

	/*
	 ** sendPacket
	 ** Send a packet of data to our chain of 'brites.
	 */
	void sendPacket(BritePacket packet, uint8_t num_times = 1);

	/*
	 ** sendPackets
	 ** Send all the packets in an array to our chain of 'brites.
	 */
	void sendPackets(BritePacket * packets, uint8_t numPackets);

	/*
	 ** sendPacketToAll
	 ** Send a packet of data to each device in our chain of 'brites.
	 */
	void sendPacketToAll(BritePacket packet);

	/*
	 ** sendColor
	 ** Create and send a color packet to the chain of 'brites.
	 */
	void sendColor(TinyBriteColorValue red, TinyBriteColorValue green, TinyBriteColorValue blue);

	/*
	 ** sendCommand
	 ** Create and send a command packet to the chain of 'brites.
	 */
	void sendCommand(unsigned int redDotCorrect, unsigned int greenDotCorrect,
			unsigned int blueDotCorrect, unsigned char clockMode);

};

#endif
