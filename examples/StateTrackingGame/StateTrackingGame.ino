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
 
 This example focuses on the state tracking functionality of the TinyBrite
 library.

 It is a GAME: hook it up as specified below, and press the button when 
 the megabrite is 100% red (red full brightness, no other color).  

 It will rotate RGB a few times and speed up everytime you get it, 
 or flash an error (an eventually slow back down) when you fail.
 
 
 *****************************  HW SETUP  *****************************  
 
 The following code expects an Arduino-compatible device tied to a
 *Brite (can be ShiftBrite, MegaBrite) or anything
 that uses the A6281 PWM LED driver.
 
 Basic hardware setup (power connections not shown)
 
 
 Arduino                        *Brite device
 +===========+                     +============+ 
 |           |                     |            |
 | Digispark |---------------------|DI (data)   |
 |           |pin A          ------|EI (enable) |
 |           |               |     |            |
 |           |              GND    |            |
 |           |pin B                |            |
 |  or any   |---------------------|CI (clock)  |
 |   type    |pin C                |            |
 |  *duino   |---------------------|LI (latch)  |
 +===========+                     +============+
 |pin D (input)
 |                            Vcc
 +-----------------+           ^               
                   |   1k      |
           ____/ __|_/\/\/\____|
           |   SW1             
          GND                
 
 
 So the megabrite is tied to the arduino as in the other examples.
 In addition, there's a pin that's setup as an input.  It is tied, 
 through a resistor, to Vcc (the positive supply) and so reads as 
 HIGH until the switch (SW1) is pressed and makes a path to ground.
 
 So when a person presses the momentary-on switch, we'll know because
 pin D will read as low.
 
 *****************************  INFO  *****************************  
 
 This code demonstrates the use of TinyBrite's state tracking.  For 
 usage examples that are more "regular", see the BriteChain or 
 BriteChainAuto examples.
 
 Enjoy,
 Pat Deegan, psychogenic.com
 
 */

#include <TinyBrite.h>

/* Specify the number of drivers chained together--one in this example */
#define num_brite_devices  1

/* A few defines specifying which pins we'll be using to talk to our 'Brites */
#define clockpin  0
#define latchpin  2
#define datapin   3

/* The pin tied to the user button, which will be reading */
#define buttonpin 4


#define first_level_delay_ms 		1500
#define speed_increase_per_round_ms  	150
#define num_rotated_colors  		15
#define max_losses_in_round		2


#ifndef TA6281_STATE_TRACKING_ENABLE
#error "State tracking is not enabled for library, see docs and TinyBriteConfig.h"
#endif

/* 
 **   *******************  TinyBrite instances **********************
 **
 ** A global TinyBrite instance is created, and initialized by specifying 
 ** the number of drivers in the chain.
 ** E.g.
 **    TinyBrite leftChain(5);  // five daisy-chained MegaBrites
 **    TinyBrite rightChain(3); // three daisy-chained ShiftBrites.
 **
 ** See the BriteChain example for details.
 */



/* **** Globals **** */


// in our test, we'll only use a single chain with a single 'brite, so 
// to make things easy we set *auto-updates* :
TinyBrite MyMegaBrites(num_brite_devices, TINYBRITE_AUTOUPDATE_ENABLE);



/* Color packets:
  We'll be rotating a bunch of colors, to see if the user can hit the button when 
  the light is RED.  We could generate these colors randomly, but here we'll keep
  things simple and just prepare a number of color packets in advance and
  store them in a global array:
*/
BritePacket rotatedColors[num_rotated_colors] = {
  TinyBrite::colorPacket(0, TINYBRITE_COLOR_MAXVALUE, 0),
  TinyBrite::colorPacket(TINYBRITE_COLOR_MAXVALUE, 0, 0), // RED!
  TinyBrite::colorPacket(0, 0, TINYBRITE_COLOR_MAXVALUE),
  TinyBrite::colorPacket(0, 0, 0),
  TinyBrite::colorPacket(TINYBRITE_COLOR_MAXVALUE, TINYBRITE_COLOR_MAXVALUE, 0),
  TinyBrite::colorPacket(0, TINYBRITE_COLOR_MAXVALUE, TINYBRITE_COLOR_MAXVALUE),
  TinyBrite::colorPacket(TINYBRITE_COLOR_MAXVALUE, 0, 0), // RED !
  TinyBrite::colorPacket(TINYBRITE_COLOR_MAXVALUE, TINYBRITE_COLOR_MAXVALUE, TINYBRITE_COLOR_MAXVALUE),
  TinyBrite::colorPacket(0, 0, TINYBRITE_COLOR_MAXVALUE),
  TinyBrite::colorPacket(512, 512, 512),
  TinyBrite::colorPacket(920, 350, 450),
  TinyBrite::colorPacket(200, 600, TINYBRITE_COLOR_MAXVALUE),
  TinyBrite::colorPacket(700, 0, 680),
  TinyBrite::colorPacket(40, 120, 310),
  TinyBrite::colorPacket(740, 190, 420),
};


// global settings we'll be changing in various functions
unsigned int delay_between_colors;
unsigned int cur_color_idx;
bool previous_button_state;
unsigned int consecutive_losses;




/* **** Game helper functions **** */

void resetGame() 
{
  // reset all our state variables
  delay_between_colors = first_level_delay_ms;
  cur_color_idx = 0;
  previous_button_state = HIGH;
  consecutive_losses = 0;
}

void danceOfJoy(int numtimes)
{
  // a little display of happy
  // dance of joy
  for (unsigned int i=0; i<numtimes; i++)
  {
    MyMegaBrites.sendColor(TINYBRITE_COLOR_MAXVALUE, 0, 0);
    delay(250);
    MyMegaBrites.sendColor(0, TINYBRITE_COLOR_MAXVALUE, 0);
    delay(250);
    MyMegaBrites.sendColor(0, 0, TINYBRITE_COLOR_MAXVALUE);
    delay(250);
  }
  return;
}

void cryOfPain()
{
  // display for errors
  for (unsigned int i=0; i<8; i++)
  {
    MyMegaBrites.sendColor(TINYBRITE_COLOR_MAXVALUE, 0, 0);
    delay(80);
    MyMegaBrites.sendColor(0, 0, 0);
    delay(120);
  }
}

void roundWon()
{
  // called when the button press was on RED! Yayz!
  consecutive_losses = 0;

  if (delay_between_colors > speed_increase_per_round_ms)
  {
    // try it faster, now!
    delay_between_colors -= speed_increase_per_round_ms;
    cur_color_idx = 0;

    danceOfJoy(4);
    // no double-pressing... we move to the next color:
    incrementColorIndex();
  } 
  else {
    // wow, you finished the game...
    danceOfJoy(8);
    resetGame();
  }

}

void roundLost()
{
  // called when button pressed on other color
  consecutive_losses++;

  if (consecutive_losses > max_losses_in_round)
  {
    // hum... bump you back a bit, you need the practice
    delay_between_colors += speed_increase_per_round_ms;

    // reset loss counter
    consecutive_losses = 0;
  }

  cryOfPain();

}

void incrementColorIndex()
{
  // move to next color
  cur_color_idx = (cur_color_idx >= num_rotated_colors) ? 0: cur_color_idx+1;

}




/* **** Standard Arduino functions **** */


void setup()
{

  // setup our button monitoring pin for input
  pinMode(buttonpin, INPUT);

  /* TinyBrite::setup()
   **
   ** setup is used to specify the pins used for data, clock, latch and, optionally,
   ** ~enable.
   **
   ** It may be called with 3 or 4 parameters.  If you simply tie the 'Brite EI to 
   ** ground, they will always be enabled and you can save one pin per chain.
   */
  MyMegaBrites.setup(datapin, clockpin, latchpin);


  /* Also, we want to use state tracking, so enable that:
   */
  MyMegaBrites.setStateTracking(true);



  // make sure everything's in a consistent state
  // by doing a reset
  resetGame();
}



// we specify a short time to delay between checks of 
// the button:
#define button_read_delay_ms   20

// main loop
void loop()
{
  // in our loop, we'll just check the button a lot, and occasionally rotate
  // the color.  If the button is pressed, we'll check if the player got it right
  // and act accordingly.

  // we'll  keep track how long we've been putzing around using:
  unsigned int delay_counter = 0;


  // we stay in this little loop for as long as we need to in the current round
  while (delay_counter < delay_between_colors)
  {
    MyMegaBrites.sendPacket(rotatedColors[cur_color_idx]); // send the current color to display

    if (digitalRead(buttonpin) == LOW)
    {
      if (previous_button_state == HIGH)
      {
        // button was just pressed
        previous_button_state = LOW; // this flag avoids winning 'cause you're holding the button down.

        // check color -- this is a convoluted way of doing it (we could just
        // look at rotatedColors[cur_color_idx]), but it is for cases when you want
        // to use state tracking, for instance if you are generating the colors 
        // randomly.
        // We call getState with the index (starting at 0) of the 'brite in the chain
        BritePacket * curColor = (BritePacket *) MyMegaBrites.getState(0); // get the state

        if (curColor != NULL)
        {
          // we have a stored state
          if (curColor->red == TINYBRITE_COLOR_MAXVALUE
            && curColor->green == 0
            && curColor->blue == 0)
          {
            // and it is pure RED!  Congrats...
            roundWon();
          } 
          else {
            // it was another color :(
            roundLost();
          }
        }
      } 

    } 
    else {
      // button is not pressed, make sure we know user has let go
      previous_button_state = HIGH;
    }

    // end if button is pressed

    // take a little break
    delay(button_read_delay_ms);
    // make note of how long we've been waiting to change the color
    delay_counter += button_read_delay_ms;

  } 

  // ah, we're out of the delay loop, time to change the color

  incrementColorIndex();

  // reset our delay counter
  delay_counter = 0;


}  



