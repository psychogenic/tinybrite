/*

 TinyBrite library example usage.
 Copyright (C) 2013 Pat Deegan.  All rights reserved.
 
 http://www.flyingcarsandstuff.com/projects/tinybrite/
 
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.
 
 See file LICENSE.txt for further informations on licensing terms.
 
 
 
 *****************************  OVERVIEW  *****************************
 
 This is a tutorial and demonstration of the TinyBrite library, used to
 control one or more daisy-chained *Brites (can be ShiftBrite, MegaBrite 
 or anything that uses the A6281 PWM LED driver).
 
 It "bit-bangs" data to the drivers, meaning you don't need SPI support
 (only 3 pins to use as digital outputs), and is specifically designed 
 with programmable RGB LEDs so it understands
 
 
 *****************************  HW SETUP  *****************************  
 
 The following code expects an Arduino-compatible device tied to a
 chain (one or more) of *Brites (can be ShiftBrite, MegaBrite) or anything
 that uses the A6281 PWM LED driver.
 
 Basic hardware setup (power connections not shown)


    Arduino                   *Brite device
 +===========+pin A           +============+ DO (data out)
 |           |----------------|DI (data)   |-------------->
 | Digispark |                |            | EO (enable out)
 |           |          ------|EI (enable) |-------------->
 |           |          |     |            |
 |           |         GND    |            |
 |           |pin B           |            | CO (clock out)
 |  or any   |----------------|CI (clock)  |-------------->  Another *Brite...
 |   type    |pin C           |            | LO (latch out)
 |  *duino   |----------------|LI (latch)  |-------------->
 +===========+                +============+
 
 
 *** NOTE ***: you may want to use pull up resistors on the 
 clock and data pins (pins A and B, above).
 This just means putting a (few k) resistor between +5V and 
 each pin. 
 
 *****************************  INFO  *****************************  
 
 This code was developed and run on a Digistump digispark, a pretty 
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
 
 The code contains commentary to clarify how the library is configured 
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
 brite_chain.endUpdate(); // changes take effect
 
 // and that's pretty much it!
 // more info on creating packets and using
 // sendPacket()/sendPackets()/sendPacketToAll()
 // below.
 
 *** Auto update cycle ***
 If you don't want to do anything fancy, and you just want to send color commands 
 with the most ease, you can pass a second optional "auto-update" boolean flag to the 
 constructor:

 TinyBrite brite_chain(3, TINYBRITE_AUTOUPDATE_ENABLE); // three daisy-chained *brites, auto-updated.

 // now, every time you call sendColor or sendPacket*, the data will immediately be latched
 // i.e. no need for begin/endUpdate:

 brite_chain.sendColor(0, 0, TINYBRITE_COLOR_MAXVALUE); // blue, color takes effect immediately


 // ... later
 brite_chain.sendColor(TINYBRITE_COLOR_MAXVALUE, 0, 0); // red takes effect immediately, blue shifted down the chain.
 
 See the accompanying BriteChainAuto example for details on auto-updates.


 Enjoy,
 Pat Deegan, psychogenic.com
 
*/

#include <TinyBrite.h>

/* Specify the number of drivers chained together */
#define num_brite_devices  4

/* A few defines specifying which pins we'll be using to talk to our 'Brites */
#define clockpin  0
#define latchpin  2
#define datapin   3


/*
**  Use of the ~enable pin is optional
 ** you can just tie the *Brite enable pin
 ** to ground, to leave enabled permanently,
 ** or tie it to an available pin.  In that case:
 ** 1) define the enable pin value
 **   #define enablepin 1
 ** 2) include the value in the call to TinyBrite::setup() (see the setup() function below)
 **
 ** 3) call TinyBrite::setEnabled(true) to activate (or pass "false" to deactivate) when appropriate
 **
 */
//#define enablepin 4


// BLINKENLIGHT
// I like blinkies to tell me something is happening... if you do too, 
// define the pin that's tied to a LED as ledpin like so:
//  #define ledpin  1



/* 
 **   *******************  TinyBrite instances **********************
 **
 ** A global TinyBrite instance is created, and initialized by specifying 
 ** the number of drivers in the chain.
 ** E.g.
 **    TinyBrite leftChain(5);  // five daisy-chained MegaBrites
 **    TinyBrite rightChain(3); // three daisy-chained ShiftBrites.
 **
 ** If you had more than one chain of 'brites tied to the uC, you could create
 ** multiple instances.  For example, an ATtiny85 could handle 2 distinct chains.
 ** The important thing would be to call setup with the correct params for each 
 ** instance, in setup() (more info below):
 **
 **  void setup() {
 **    leftChain.setup(0,1,2);
 **    rightChain.setup(3,4,5);
 **  }
 ** 
 **
 ** Note: the number of drivers passed in the constructor 
 **       only matters when using TinyBrite::sendPacketToAll() or 
 **       state tracking.
 */
 
// in our test, we'll only use a single chain:
TinyBrite MyMegaBrites(num_brite_devices);



void setup()
{
  
  /* TinyBrite::setup()
  **
  ** setup is used to specify the pins used for data, clock, latch and, optionally,
  ** ~enable.
  **
  ** It may be called with 3 or 4 parameters.  If you simply tie the 'Brite EI to 
  ** ground, they will always be enabled and you can save one pin per chain.
  */

#ifdef enablepin
  // we've got an enable pin set
  MyMegaBrites.setup(datapin, clockpin, latchpin, enablepin);
  MyMegaBrites.setEnabled(true);
#else
  // not using enable pin, assume tied to ground (i.e. always on)
  MyMegaBrites.setup(datapin, clockpin, latchpin);
#endif

  // Note: if using the ~enable pin to control activation, call setup with 4 params:
  //  MyMegaBrites.setup(datapin, clockpin, latchpin, enablepin);
  // and, at some point, do:
  //  MyMegaBrites.setEnabled(true);


#ifdef ledpin
  pinMode(ledpin, OUTPUT);
#endif
}



/* a little feedback can be provided by toggling the 'duino's on-board LED, if 
** you have defined 
**  ledpin
** above.  Otherwise, toggleBlinker will have no effect.
*/
boolean ledState = false;
void toggleBlinker()
{
#ifdef ledpdin
  ledState = ! ledState;
  digitalWrite(ledpin, ledState);
#endif
}


// a few useful defines so our code is nice and meaningful

// num_fades_to_perform -- number of loops to perform for "fade" in/out operations
#define num_fades_to_perform  4

// num_pulses_to_send -- number of pulses to send down the chain
#define num_pulses_to_send    50

// a few index defines to make things legible
#define red_idx    0
#define green_idx  1
#define blue_idx   2

void loop()
{
  
  // rgb_vals will hold some color values.
  // when using variables to hold colors, use unsigned ints to hold the 10 bit value
  // nicely.
  unsigned int rgb_vals[3] = {0};


  /*** simple updates: sendColor() ***/

  // we start by sending colors such that they'll fade in on each bright in the chain, following a pattern
  // like "blue <- green <- red <- blue ..." for as many 'brites as there are in the chain
  // the reason the arrows above are "left-pointing" is that first will spew out the code for red, then
  // *if* there's another 'brite attached, we'll push out the code for fading in green (which will move the
  // red setting down the chain), then *if* there's another bright we'll send the fade in for blue, which will
  // move green and red one step down the chain, etc.

  // loop a few times so we get a chance to appreciate our wonderous fade ins...
  for (byte numFades=0; numFades < num_fades_to_perform; numFades++)
  {
    // go from 0 intensity to full intensity, for whatever color (incrementing by 20 on each iteration)
    for (unsigned int color_value=0; color_value <= TINYBRITE_COLOR_MAXVALUE; color_value+=20)
    {

      // we are going to be sending one or more packets to the chain--always beginUpdate() prior to sending:
      MyMegaBrites.beginUpdate();

      // now we send a color for each of the 'brites in our chain.
      // will increase the value for a single color--which, exactly, will depend on where you are in the chain

      for (byte drv_index=0; drv_index < num_brite_devices; drv_index++)
      {
        // zero all our color values
        rgb_vals[red_idx]   = 0;
        rgb_vals[green_idx] = 0;
        rgb_vals[blue_idx]  = 0;

        // set the color intesity for whichever LED we're turning on at this stage
        rgb_vals[drv_index % 3] = color_value;

        // send that color
        MyMegaBrites.sendColor(rgb_vals[red_idx], rgb_vals[green_idx], rgb_vals[blue_idx]);

      }
      // call endUpdate() to have our sent colors activated on all the 'brites.
      MyMegaBrites.endUpdate();

      // take a little break, so we can actually see something happening:
      delay(10);

      toggleBlinker();
    }

  }



  /***** hand-crufted packets: colorPacket()/sendPacket() *****/

  // now we'll send a sequence of colors (red, green, blue...) one at
  // at time.  These should march down the chain.
  for (byte numPulses=0; numPulses < num_pulses_to_send; numPulses++)
  {

    // zero all our color values
    rgb_vals[red_idx]   = 0;
    rgb_vals[green_idx] = 0;
    rgb_vals[blue_idx]  = 0;

    // set the color intesity for whichever LED we're turning on at this stage
    rgb_vals[numPulses % 3] = TINYBRITE_COLOR_MAXVALUE;

    BritePacket packet = MyMegaBrites.colorPacket(rgb_vals[red_idx], rgb_vals[green_idx], rgb_vals[blue_idx]);

    // we are sending a packet to the chain--always beginUpdate() prior to sending:
    MyMegaBrites.beginUpdate();
    // send that packet
    MyMegaBrites.sendPacket(packet);

    // call endUpdate() to have our sent colors activated on all the 'brites.
    MyMegaBrites.endUpdate();


    // take a little break, so we can actually see something happening:
    delay(110);
    toggleBlinker();
  }


  /***** broadcasting: sendPacketToAll() *******/

  // here we'll use the easier way to get all the 'brites set to the same value using sendPacketToAll()

  // loop a few times so we get a chance to appreciate our wonderous fade ins...
  for (byte numFades=0; numFades < num_fades_to_perform; numFades++)
  {
    // go from 0 intensity to full intensity, for whatever color (incrementing by 10 on each iteration)
    for (unsigned int intensity_value=0; intensity_value <= TINYBRITE_COLOR_MAXVALUE; intensity_value+=15)
    {

      // we want bring all our colors up evenly, so we create a packet with some value
      // here we'll just set the intensity of R, G and B to the same value

      BritePacket packet = MyMegaBrites.colorPacket(intensity_value, intensity_value, intensity_value);

      // we are going to be sending one or more packets to the chain--always beginUpdate() prior to sending:
      MyMegaBrites.beginUpdate();

      // we want every 'brite to have the same color... we could send the packet once for each 
      // driver in the chain, but it's easier to use sendPacketToAll()
      MyMegaBrites.sendPacketToAll(packet);
      // call endUpdate() to have our sent colors activated on all the 'brites.
      MyMegaBrites.endUpdate();

      // take a little break, so we can actually see something happening:
      delay(5);
    }
    
    // go from 0 intensity to full intensity, for whatever color (incrementing by 10 on each iteration)
    for (unsigned int intensity_value=TINYBRITE_COLOR_MAXVALUE; intensity_value >= 15 ; intensity_value-=15)
    {

      // we want bring all our colors up evenly, so we create a packet with some value
      // here we'll just set the intensity of R, G and B to the same value

      BritePacket packet = MyMegaBrites.colorPacket(intensity_value, intensity_value, intensity_value);

      // we are going to be sending one or more packets to the chain--always beginUpdate() prior to sending:
      MyMegaBrites.beginUpdate();

      // we want every 'brite to have the same color... we could send the packet once for each 
      // driver in the chain, but it's easier to use sendPacketToAll()
      MyMegaBrites.sendPacketToAll(packet);
      // call endUpdate() to have our sent colors activated on all the 'brites.
      MyMegaBrites.endUpdate();

      // take a little break, so we can actually see something happening:
      delay(5);
    }

    toggleBlinker();
  }


  /*** Lists of packets: sendPackets() ***/
  
  // If you have a long chain of drivers, you can set them all in one go by calling sendPackets (not the "s") 
  // along with a pointer to an array of BritePackets.
  // Remember that the _first_ 'brite in the chain (closest to the uC) will end up with the _last_ packet in the list.
  
  // We'll define an array to hold 10 BritePackets, and initialize the array using the colorPacket() classmethod.
  #define num_packets_in_bunch  10
  BritePacket aBunch[num_packets_in_bunch] = {
          TinyBrite::colorPacket(TINYBRITE_COLOR_MAXVALUE, 0, 0),
          TinyBrite::colorPacket(0, TINYBRITE_COLOR_MAXVALUE, 0),
          TinyBrite::colorPacket(0, 0, TINYBRITE_COLOR_MAXVALUE),
          TinyBrite::colorPacket(0, 0, 0),
          TinyBrite::colorPacket(TINYBRITE_COLOR_MAXVALUE, TINYBRITE_COLOR_MAXVALUE, 0),
          TinyBrite::colorPacket(0, TINYBRITE_COLOR_MAXVALUE, TINYBRITE_COLOR_MAXVALUE),
          TinyBrite::colorPacket(TINYBRITE_COLOR_MAXVALUE, 0, TINYBRITE_COLOR_MAXVALUE),
          TinyBrite::colorPacket(TINYBRITE_COLOR_MAXVALUE, TINYBRITE_COLOR_MAXVALUE, TINYBRITE_COLOR_MAXVALUE),
          TinyBrite::colorPacket(0, 0, TINYBRITE_COLOR_MAXVALUE),
          TinyBrite::colorPacket(512, 512, 512)
  };


  // now we need only call sendPackets (within a single "update cycle"), passing a pointer to the first element
  // in the list and the number of elements total.  Careful here: if you go out of bounds, scary undefined things may happen.
  MyMegaBrites.beginUpdate();
  MyMegaBrites.sendPackets(&aBunch[0], num_packets_in_bunch);
  MyMegaBrites.endUpdate();
  
  // show off our chain lightshow a bit...
  delay(800);


  // now we'll send each packet in the array to everyone, one at a time... 
  // looks nice and retro-computery and you can see some action even if you 
  // don't have a chain of 10 megabrites.
  for (byte numFades=0; numFades < (num_fades_to_perform*2); numFades++)
  {
    for (byte i=0; i<10; i++)
    {

      MyMegaBrites.beginUpdate();
      MyMegaBrites.sendPacketToAll(aBunch[i]);
      MyMegaBrites.endUpdate();
      
      delay(200);
      toggleBlinker();
    }
  }


}  

