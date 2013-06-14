/*

 TinyBrite library auto-update chain example.
 Copyright (C) 2013 Pat Deegan.  All rights reserved.
 
 http://www.flyingcarsandstuff.com/projects/tinybrite/
 
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.
 
 See file LICENSE.txt for further informations on licensing terms.
 
 
 
 *****************************  OVERVIEW  *****************************
 
 The complete tutorial/demo of the TinyBrite library can be found in the
 BriteChain example.

 Here, we'll just demonstrate use of the auto-update cycle.
 
 ** Upside: clear and simple, save begin/endUpdate lines of code.  Can make
 your compiled program slightly smaller, too.

 ** Downside: may be slower and cause flicker when updating a chain of 
	devices, as the commands take effect as soon as they are sent.
        Doesn't allow "instantaneous" updates of multiple devices.


 Hardware setup is the same as in Examples -> TinyBrite -> BriteChain.
 
 
 Enjoy,
 Pat Deegan, psychogenic.com
 
*/

#include <TinyBrite.h>

/* Specify the number of drivers chained together */
#define num_brite_devices  3


/* A few defines specifying which pins we'll be using to talk to our 'Brites */
#define clockpin  0
#define latchpin  2
#define datapin   3



/* 
 **   *******************  TinyBrite instances **********************
 **
 ** A global TinyBrite instance is created, and initialized by specifying 
 ** the number of drivers in the chain and a flag to indicate we want
 ** auto updates in this case..
 ** E.g.
 **    TinyBrite leftChain(5, TINYBRITE_AUTOUPDATE_ENABLE);  // five daisy-chained MegaBrites
 **    TinyBrite rightChain(3, TINYBRITE_AUTOUPDATE_ENABLE); // three daisy-chained ShiftBrites.
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
 ** You can manage the auto-update cycle handling any time using the
 ** autoUpdate() (read) and setAutoUpdate(bool) methods.
 **
 */
 
// in our test, we'll only use a single chain with auto-update enabled:
TinyBrite MyMegaBrites(num_brite_devices, TINYBRITE_AUTOUPDATE_ENABLE);



void setup()
{
  
  /* TinyBrite::setup()
  ** 
  ** See the BriteChain example for details.
  */
  MyMegaBrites.setup(datapin, clockpin, latchpin);

}


// num_send_trios_to_perform -- number of times we'll circle around the R-G-B colors
#define num_send_trios_to_perform	15

// num_fades_to_perform -- number of loops to perform for "fade" in/out operations
#define num_fades_to_perform  5

void loop()
{

  // Note: Full usage and explanations available in BriteChain example (this only goes over auto updates).



  // Though not required, thanks to our constructor call parameter in setup() above, we'll
  // start be "turning on" auto update cycle handling using setAutoUpdate(), just to demonstrate:
  MyMegaBrites.setAutoUpdate(TINYBRITE_AUTOUPDATE_ENABLE); 

  // With auto-updates, all we really need to do is:
  MyMegaBrites.sendColor(TINYBRITE_COLOR_MAXVALUE, TINYBRITE_COLOR_MAXVALUE, TINYBRITE_COLOR_MAXVALUE);

  // and the 'brites will be updated. The end.  No longer have a need to call
  // beginUpdate()/endUpdate() around the send* ...

  delay(400);
  
  // send a few more
  for (byte i=0; i< num_send_trios_to_perform; i++)
  {

  	// send a color:
  	MyMegaBrites.sendColor(0, 0, TINYBRITE_COLOR_MAXVALUE); // blue
	delay(75);

	// make it so:
  	MyMegaBrites.sendColor(TINYBRITE_COLOR_MAXVALUE, 0, 0); // red
	delay(75);

	// and another:
  	MyMegaBrites.sendColor(0, TINYBRITE_COLOR_MAXVALUE, 0); // green
	delay(75);
  }



  // Let's do something a little more fancy, fading in 
  // all our devices, say...

  doFades();

}



// a few index defines to make things legible
#define red_idx    0
#define green_idx  1
#define blue_idx   2

void doFades() 
{


  // rgb_vals will hold some color values.
  // when using variables to hold colors, use unsigned ints to hold the 10 bit value
  // nicely.
  unsigned int rgb_vals[3] = {0};

  
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

      // we want every 'brite to have the same color... we could send the packet once for each 
      // driver in the chain, but it's easier to use sendPacketToAll()
      MyMegaBrites.sendPacketToAll(packet);


      // take a little break, so we can actually see something happening:
      delay(5);
    }
    
    // go from 0 intensity to full intensity, for whatever color (incrementing by 10 on each iteration)
    for (unsigned int intensity_value=TINYBRITE_COLOR_MAXVALUE; intensity_value >= 15 ; intensity_value-=15)
    {

      // we want bring all our colors up evenly, so we create a packet with some value
      // here we'll just set the intensity of R, G and B to the same value

      BritePacket packet = MyMegaBrites.colorPacket(intensity_value, intensity_value, intensity_value);

      // we want every 'brite to have the same color... we could send the packet once for each 
      // driver in the chain, but it's easier to use sendPacketToAll()
      MyMegaBrites.sendPacketToAll(packet);

      // take a little break, so we can actually see something happening:
      delay(5);
    }

  }

  delay(300);





}  

