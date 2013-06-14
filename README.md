TinyBrite
=========

TinyBrite: the easiest way to use a chain of MegaBrites, ShiftBrites or anything based on the A6281 PWM LED driver.

TinyBrite Overview
===============================

   		Home: http://www.flyingcarsandstuff.com/projects/tinybrite

If you want a simple way to use a chain of MegaBrites, ShiftBrites or anything 
based on the A6281 PWM LED driver, the TinyBrite library will handle all the 
details for you.  

Using TinyBrite can be as simple as:

	chain_of_lights.sendColor(R, G, B);  // yay!
	
TinyBrite doesn't need SPI, so you can drive the RGB LEDs even with ATTiny-based 
Arduino-compatible hardware like the (Digistump) Digispark!

The library was developed for small Arduino-compatible boards, but in addition
to working with all Arduinos it should be usable on pretty much any Atmel AVR 
chip and probably relatively easily with other microcontrollers, too, with a 
little tweaking.

'Brites: MegaBrite, ShiftBrite, and others
===============================


The Macetech LED modules take all the hardware-related work out of dealing 
with powerful RGB lighting.

MegaBrites, are a combination of 3 sets of LEDs.  Each set is equivalent to 15
standard LEDs of red, green or blue.  Also on-board is an A6281 (Allegro 
3-Channel Constant Current LED Driver with Programmable PWM Control).

From the outside, the MegaBrite looks like a single, programmable, RGB LED. You
can specify the intensity for each of red, green and blue with 10-bit resolution
(a value between 0 and 1023).

Best of all, the MegaBrites are designed to be easily daisy-chained.  This 
means that, using three or four pins on your microcontroller, you can control
(almost) any number of MegaBrites with no additional components (other than 
wiring between the modules).

I've yet to play with ShiftBrites, but the API seems much the same with the main
difference being that the megas have more juice.


How TinyBrite works
===============================

The TinyBrite library uses software to handle the serial communication with the
'brites.

The downside is that while data is being transmitted your microcontroller will
be busy (i.e. you have to account for the transmit time if you're dealing with
stringent timing constraints).  

The main advantage of bit banging the comm in this way is that there are no 
special requirements for your microcontroller--there is no need for SPI support, 
or if it's available you can dedicate it to something else, and you can use any
three of the available pins for clock, latch and data transmission.

For an overview, basic schematics, usage, a full description of the API and 
TinyBrite projects see:
http://www.flyingcarsandstuff.com/projects/tinybrite



License
===============================

 TinyBrite, Copyright (C) 2013 Pat Deegan, is released under the 
 terms of the GNU Lesser General Public License (LGPL).

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 See file LICENSE.txt for further informations on licensing terms.
 
 
