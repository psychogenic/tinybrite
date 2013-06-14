/*

 TinyA6281.h -- part of the TinyBrite library.
 Copyright (C) 2013 Pat Deegan.  All rights reserved.

 Interface to A6281 drivers.
 
 
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
 
 This is the low-level interface to the A6281, on which the TinyBrite
 interface is based.  See TinyBrite.h and/or the explanations and 
 examples in 
 Examples -> TinyBrite -> BriteChain
 for further information.

*/

#ifndef TinyA6281_h
#define TinyA6281_h

#include "TinyBriteConfig.h"
#include "TinyBritePlatform.h"



#define TA6281_MODE_PWM			0
#define TA6281_MODE_CORRECT		1

#define TA6281_PWM_MAXVALUE		1023

#define TA6281_CORRECTION_MAXVALUE	127

#define TA6281_COMMAND_CLOCK_800kHz		B00
#define TA6281_COMMAND_CLOCK_400kHz		B10
#define TA6281_COMMAND_CLOCK_200kHz		B11
#define TA6281_COMMAND_CLOCK_EXT		B01

#define TA6281_AUTOUPDATE_ENABLE	true
#define TA6281_AUTOUPDATE_DISABLE	false


#ifdef TA6281_STATE_TRACKING_BIGNUM
typedef uint16_t	DriverNum;
#else
typedef uint8_t		DriverNum;
#endif

/*
 **  A6281Packet
 **
 ** We send data in packets of 32 bits.  Depending on the contents of the
 ** mode_* field, the packet will be interpreted as either a set of
 ** three PWM settings, or a "command" packet (for manual correction/adjustment,
 ** clock settings and device testing).
 */
typedef union A6281Packet {
	unsigned long value;

	struct {
		unsigned pwm_0 :10;
		unsigned pwm_1 :10;
		unsigned pwm_2 :10;
		unsigned mode_pwm :1;
	};

	struct {
		unsigned dotCorrect0 :7;
		unsigned clockMode :2;
		unsigned :1;
		unsigned dotCorrect1 :7;
		unsigned :3;
		unsigned dotCorrect2 :7;
		unsigned :1;
		unsigned atb0 :1;
		unsigned atb1 :1;
		unsigned mode_correct :1;

	};
} A6281Packet;


#ifdef TA6281_STATE_TRACKING_ENABLE
typedef A6281Packet		StatePacket;
#endif



/*
 ** TinyA6281 class.
 **
 ** Handles packet creation and communication with chains of A6281-based devices.
 */
class TinyA6281

{

public:

	/* 
	 ** TinyA6281 constructor.
	 ** Call with the number of devices chained together.
	 */
	TinyA6281(DriverNum num_drivers = 1, bool auto_update_cycle =
			TA6281_AUTOUPDATE_DISABLE);

	/*
	 ** Auto-updates.
	 ** Using auto-update, all packets sent take effect immediately so you don't
	 ** need to use
	 ** beginUpdate/endUpdate around your send* calls.
	 **
	 ** With auto-update on, you get:
	 ** * Upside: clear and simple, save begin/endUpdate lines of code.
	 ** * Downside: may be slower and cause flicker when updating a chain of
	 **	devices, as the commands take effect as soon as they are sent.
	 **
	 */

	/* autoUpdate
	 ** Returns current state of auto-update setting.
	 */
	bool autoUpdate();
	/* setAutoUpdate
	 ** Turn auto-update on or off, using a boolean parameter.
	 */
	void setAutoUpdate(bool setTo);

	/* setup:
	 ** Two versions available: with and without an ~enable pin.
	 **
	 ** Call setup(data_pin, clock_pin, latch_pin) if you are only dedicating
	 ** three pins to comm with the A6281.
	 ** Call
	 **	setup(data_pin, clock_pin, latch_pin, enable_pin)
	 **
	 ** if you have connected a pin to EI on the chip and intend to activate/deactivate
	 ** the driver(s) using setEnabled() (below).
	 */

	/*
	 ** setup -- NO ~enable pin used
	 ** Register and configure the pins used for data, clock and latch.
	 */
	void setup(uint8_t datapin, uint8_t clockpin, uint8_t latchpin);

	/*
	 ** setup -- ~enable pin used
	 ** Register and configure the pins used for data, clock, latch and ~enable.
	 */
	void setup(uint8_t datapin, uint8_t clockpin, uint8_t latchpin,
			uint8_t nEnablepin);

	// a few class methods to create packets

	/*
	 ** pwmPacket
	 ** Create a valid PWM data packet.
	 */
	static A6281Packet pwmPacket(unsigned int pwm0, unsigned int pwm1,
			unsigned int pwm2);

	/*
	 ** commandPacket
	 ** Create a valid command data packet.
	 */
	static A6281Packet commandPacket(unsigned int correct0,
			unsigned int correct1, unsigned int correct2,
			unsigned char clockMode);

	/* only need to call setEnabled if you have set the nEnable pin during setup and are 
	 ** using it to control the drivers.
	 ** Call setEnabled(FALSE) to disable/0 the pwm outputs, and  setEnabled(TRUE) to bring it online.
	 */
	void setEnabled(bool activateDriver);

	/* 
	 ** beginUpdate
	 ** Begin an update cycle.
	 ** perform an "update cycle" for each write.
	 ** e.g.
	 ** beginUpdate();
	 **  // commands...
	 **  sendPacket(...);
	 **  sendPacket(...);
	 **
	 **  endUpdate();
	 */
	void beginUpdate();

	/*
	 ** endUpdate
	 ** End an update cycle, latch the current data.
	 ** See beginUpdate, above.
	 */
	DriverNum endUpdate();

	/*
	 ** sendPacket
	 ** Send a packet of data to our chain of A6281 devices.
	 */
	void sendPacket(A6281Packet packet, uint8_t num_times=1);

	/*
	 ** sendPackets
	 ** Send all packets in an array to our chain of A6281 devices.
	 */
	void sendPackets(A6281Packet * packets, DriverNum numPackets);

	/*
	 ** sendPacketToAll
	 ** Send a single packet to every driver in our chain of A6281 devices.
	 */
	void sendPacketToAll(A6281Packet packet);

	/*
	 ** sendPWMValues
	 ** Create a PWM data packet and send it to the first device in the chain.
	 */
	void sendPWMValues(unsigned int pwm0, unsigned int pwm1, unsigned int pwm2);

	/*
	 ** sendCommand
	 ** Create a command data packet and send it to the first device in the chain.
	 */
	void sendCommand(unsigned int dotCorrect0, unsigned int dotCorrect1,
			unsigned int dotCorrect2, unsigned char clockMode);




#ifdef TA6281_STATE_TRACKING_ENABLE
	bool stateTracking() {return tracking_state; }
	bool setStateTracking(bool setTo);

	StatePacket * getState(DriverNum driver_index);
	DriverNum saveState(StatePacket * a_state_vector);
	void restoreState(StatePacket * a_state_vector);
#endif


private:

	void latch();

	void setPins(uint8_t datapin, uint8_t clockpin, uint8_t latchpin,
			uint8_t nEnablepin);

	bool using_nEnable;

	uint8_t pin_data;
	uint8_t pin_clock;
	uint8_t pin_latch;
	uint8_t pin_nEnable;

	DriverNum num_sent;
	DriverNum num_drivers;
	bool auto_update_cycle;
#ifdef TA6281_STATE_TRACKING_ENABLE
	bool tracking_state;
	StatePacket * state_vector;
	DriverNum state_vector_head_idx;
#endif

};

#endif
