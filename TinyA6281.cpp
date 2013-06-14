/*

 TinyA6281.cpp -- part of the TinyBrite library.
 Copyright (C) 2013 Pat Deegan.  All rights reserved.

 Implementation of interface to A6281 drivers.

 http://www.flyingcarsandstuff.com/projects/tinybrite/
 
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 PARTICULAR PURPOSE.

 See file LICENSE.txt for further informations on licensing terms.

 See TinyBrite.h and/or the explanations and examples in 
 Examples -> TinyBrite -> BriteChain
 for further information.
 */

#include "includes/TinyA6281.h"
#include "includes/TinyBritePlatform.h"

/* A few packet "setup" defines, to keep from repeating this code while 
 ** leaving things snappy by avoiding function calls
 */
#define TA6281_SETPWMPACKET(packet, pwm0, pwm1, pwm2) \
	packet.pwm_0 = pwm0; \
	packet.pwm_1 = pwm1; \
	packet.pwm_2 = pwm2; \
	packet.mode_pwm = TA6281_MODE_PWM;

#define TA6281_SETCOMMANDPACKET(packet, correct0, correct1, correct2, clockMode) \
	packet.dotCorrect0 = correct0; \
	packet.dotCorrect1 = correct1; \
	packet.dotCorrect2 = correct2; \
	packet.clockMode = clockMode; \
	packet.mode_correct = TA6281_MODE_CORRECT;

/*
 ** TinyA6281 constructor.
 ** Only needs to setup defaults and record the number of A6281s in the chain.
 */
TinyA6281::TinyA6281(DriverNum numA6281s, bool autoUpdates) :
		using_nEnable(false), pin_data(TA6281_DEFAULT_DATAPIN), pin_clock(
				TA6281_DEFAULT_CLOCKPIN), pin_latch(TA6281_DEFAULT_LATCHPIN), pin_nEnable(
				TA6281_DEFAULT_NENABLEPIN), num_sent(0), num_drivers(numA6281s), auto_update_cycle(
				autoUpdates)
#ifdef TA6281_STATE_TRACKING_ENABLE
			, tracking_state(false), state_vector(NULL), state_vector_head_idx(0)
#endif
{

}

/* autoUpdate
 ** Returns current state of auto-update setting.
 */
bool TinyA6281::autoUpdate() {
	return auto_update_cycle;
}

/* setAutoUpdate
 ** Turn auto-update on or off, using a boolean parameter.
 */
void TinyA6281::setAutoUpdate(bool setTo) {
	auto_update_cycle = setTo;
}

/*
 ** setEnabled
 ** Sets the ~ENABLE pin appropriately, if it's being used.
 */
void TinyA6281::setEnabled(bool activate) {
	// it's inverted (enabled when low)
	if (using_nEnable) {
		if (activate) {
			MCU::digitalOut(pin_nEnable, LOW);
		} else {
			MCU::digitalOut(pin_nEnable, HIGH);
		}
	}
}

/*
 ** setPins
 ** Configures the pins to use for data, clock, latch and enable.
 ** NOTE: this is where the pin mode is set, so you want to ensure
 **       the using_nEnable flag is set correctly prior to the call.
 */
void TinyA6281::setPins(uint8_t datapin, uint8_t clockpin, uint8_t latchpin,
		uint8_t nEnablepin) {
	pin_data = datapin;
	pin_clock = clockpin;
	pin_latch = latchpin;
	pin_nEnable = nEnablepin;

	MCU::setPinMode(pin_clock, OUTPUT);
	MCU::setPinMode(pin_latch, OUTPUT);
	MCU::setPinMode(pin_data, OUTPUT);

	if (using_nEnable) {
		MCU::setPinMode(pin_nEnable, OUTPUT);
	}

}

/*
 ** setup -- NO ~enable pin used
 ** Register and configure the pins used for data, clock and latch.
 */
void TinyA6281::setup(uint8_t datapin, uint8_t clockpin, uint8_t latchpin) {

	using_nEnable = false; // make sure we set this *prior* to setPins call
	setPins(datapin, clockpin, latchpin, TA6281_DEFAULT_NENABLEPIN);

	MCU::digitalOut(pin_clock, LOW);
	MCU::digitalOut(pin_latch, LOW);

}

/*
 ** setup -- ~enable pin used
 ** Register and configure the pins used for data, clock, latch and ~enable.
 */
void TinyA6281::setup(uint8_t datapin, uint8_t clockpin, uint8_t latchpin,
		uint8_t nEnablepin) {

	using_nEnable = true; // make sure we set this *prior* to setPins call
	setPins(datapin, clockpin, latchpin, nEnablepin);

	MCU::digitalOut(pin_clock, LOW);
	MCU::digitalOut(pin_latch, LOW);
	MCU::digitalOut(pin_nEnable, LOW);

}

/* 
 ** beginUpdate
 ** Begin an update cycle.
 **
 ** For the moment, beginUpdate isn't all that useful...
 ** we may need additional setup/init code in here at some point,
 ** so to keep us future proof, and also to provide symmetry during
 ** coding, we request users perform an "update cycle" for each write.
 ** e.g.
 ** beginUpdate();
 **  // commands...
 **  sendPacket(...);
 **  sendPacket(...);
 **
 **  endUpdate();
 */
void TinyA6281::beginUpdate() {
	// reset our number sent counter	
	num_sent = 0;
}

/*
 ** endUpdate
 ** End an update cycle, latch the current data.
 ** See beginUpdate, above.
 */
DriverNum TinyA6281::endUpdate() {
	if (num_sent) {
		latch();
	}

	return num_sent;
}

/*
 ** pwmPacket
 ** Create a valid PWM data packet.
 */
A6281Packet TinyA6281::pwmPacket(unsigned int pwm0, unsigned int pwm1,
		unsigned int pwm2) {
	A6281Packet packet = {value:0};

	TA6281_SETPWMPACKET(packet, pwm0, pwm1, pwm2);

	return packet;
}

/*
 ** commandPacket
 ** Create a valid command data packet.
 */
A6281Packet TinyA6281::commandPacket(unsigned int correct0,
		unsigned int correct1, unsigned int correct2, unsigned char clockMode) {

	A6281Packet packet = {value:0};

	TA6281_SETCOMMANDPACKET(packet, correct0, correct1, correct2, clockMode);

	return packet;
}

/*
 ** sendPWMValues
 ** Create a PWM data packet and send it to the first device in the chain.
 */
void TinyA6281::sendPWMValues(unsigned int pwm0, unsigned int pwm1,
		unsigned int pwm2) {
	A6281Packet packet = {value:0};

	TA6281_SETPWMPACKET(packet, pwm0, pwm1, pwm2);

	sendPacket (packet);

}

/*
 ** sendCommand
 ** Create a command data packet and send it to the first device in the chain.
 */
void TinyA6281::sendCommand(unsigned int correct0, unsigned int correct1,
		unsigned int correct2, unsigned char clockMode) {
	A6281Packet packet = {value:0};

	TA6281_SETCOMMANDPACKET(packet, correct0, correct1, correct2, clockMode);

	sendPacket (packet);

}

/*
 ** sendPacket
 ** Send a packet of data to our chain of A6281 devices.
 */
void TinyA6281::sendPacket(A6281Packet packet,  uint8_t num_times) {
	if (auto_update_cycle) {
		beginUpdate();
	}


	for (uint8_t n=0; n < num_times; n++)
	{
		for (uint8_t i = 1; i < 33; i++) {
			//Set the appropriate Data In value according to the packet.
			if ((packet.value >> (32 - i)) & 1)
				MCU::digitalOut(pin_data, HIGH);
			else
				MCU::digitalOut(pin_data, LOW);

			// toggle the clock
			MCU::digitalOut(pin_clock, HIGH);
			MCU::delayUs(TA6281_CLOCK_DELAY_US);
			MCU::digitalOut(pin_clock, LOW);
			MCU::delayUs(TA6281_CLOCK_DELAY_US);
		}

		num_sent++;
	}


#ifdef TA6281_STATE_TRACKING_ENABLE
	// state tracking is on, keep this packet if we need to (and can do so).

	/*
	 * Ok, big explanation for little code, but this needs to be
	 * clear...
	 *
	 * Say we have a state vector for, e.g., 4 drivers
	 * uC -> [ A, B, C, D]
	 * now we send another packet, E, which pushes everything
	 * down the line:
	 * uC -> [E, A, B, C] ("D" falls into the void)
	 * To restore this state at a later time, we would have to
	 * send
	 * C, B, A and finally E
	 *
	 * To store this efficiently, we'll use a ring buffer (starting
	 * at the end) and move back a step everytime something is
	 * added.
	 *
	 * So our system is:
	 *  * prepare an array for state, and header pointer:
	 *  [ _, _, _, _ ]
	 *                 ^
	 * NOTE: pointer (^) starts off out of bounds.
	 *
	 *  * when we send packet "A", so we move the pointer back
	 *    one slot, and store the state there:
	 *  [_, _, _, A] <- uC
	 *            ^
	 *  (state of drivers is "A, ?, ?, ?" from uC)
	 *
	 *  * do this with each packet, "B", "C", "D":
	 *  [D, C, B, A] <- uC
	 *   ^
	 *  (state of drivers is "D, C, B, A" from uC
	 *
	 *  * loop around the circular buffer as needed, eg "E" packet comes in:
	 *  [D, C, B, E] <- uC
	 *            ^
	 *  (state of drivers is "E, D, C, B" from uC.
	 *
	 * In the end, the current state of driver N is slot (header + N) % num drivers.
	 *
	 * To restore, you'll have to send them "backwards", see restore for that.
	 */

	if (tracking_state && state_vector)
	{
		// we *are* tracking state and do have a state vector available
		if (state_vector_head_idx)
		{
			// ok, we have room to move down one slot
			state_vector_head_idx--;
		} else {
			// oh, we're at the bottom of our ring, circle 'round:
			state_vector_head_idx = num_drivers - 1;
		}

		// store this packet.
		state_vector[state_vector_head_idx] = packet;
	}

#endif

	if (auto_update_cycle) {
		endUpdate();
	}

}

/*
 ** sendPackets
 ** Send all packets in an array to our chain of A6281 devices.
 */
void TinyA6281::sendPackets(A6281Packet * packets, DriverNum numPackets) {
	bool tmpUpdate = false;

	if (auto_update_cycle) {
		// suspend autoupdates for multiple send
		tmpUpdate = true;
		auto_update_cycle = false;
		beginUpdate();
	}

	A6281Packet * curPacket = packets;

	for (DriverNum i = 0; i < numPackets; i++) {
		sendPacket(*curPacket);
		curPacket++;
	}

	if (tmpUpdate) {
		// auto updates were on
		endUpdate();
		// re-enable
		auto_update_cycle = true;
	}

}

/*
 ** sendPacketToAll
 ** Send a single packet to every driver in our chain of A6281 devices.
 */
void TinyA6281::sendPacketToAll(A6281Packet packet) {

	sendPacket(packet, num_drivers);

}

/*
 ** latch
 ** Toggle the latch to make data currently in A6281 shift registers take effect.
 */
void TinyA6281::latch() {
	// Set Latch high
	MCU::digitalOut(pin_latch, HIGH);
	MCU::delayUs(TA6281_LATCH_DELAY_US);
	// Set Latch low
	MCU::digitalOut(pin_latch, LOW);
}


#ifdef TA6281_STATE_TRACKING_ENABLE
bool TinyA6281::setStateTracking(bool setTo)
{
	tracking_state = setTo;
	if (tracking_state && num_drivers && ! state_vector)
	{
		state_vector = (StatePacket*)malloc(sizeof(StatePacket) * num_drivers );
		if (state_vector)
		{
			memset(state_vector, 0, sizeof(StatePacket) * num_drivers);
			state_vector_head_idx = num_drivers; // we initialize 1 unit out of bounds (decremented on send)

		} else {
			tracking_state = false;
		}
	}

	return tracking_state;

}

StatePacket * TinyA6281::getState(DriverNum driver_index)
{
	if (driver_index >= num_drivers || ! state_vector)
	{
		return NULL;
	}

	return &(state_vector[driver_index]);

}

DriverNum TinyA6281::saveState(StatePacket * a_state_vector)
{
	// we copy the current state over, in order, to the state vector
	// passed in.  See the commentary in sendPacket() for mucho info.

	if (!state_vector)
	{
		return 0;
	}

	if (state_vector_head_idx >= num_drivers)
	{
		// we've yet to save any state at all...
		return 0;
	}


	// lets start at the beginning:
	DriverNum cur_idx = state_vector_head_idx;

	for (DriverNum i=0; i<num_drivers; i++)
	{
		a_state_vector[i] = state_vector[cur_idx++];

		if (cur_idx >= num_drivers)
		{
			// if we've gone out of bounds, go back to
			// the start of the ring buffer
			cur_idx = 0;
		}
	}

	return num_drivers;

}
void TinyA6281::restoreState(StatePacket * a_state_vector)
{
	// we assume the state vector passed in was generated by
	// saveState() or at least conforms to our expected ordering,
	// i.e. state of each driver in order from uC's point of view,
	// going down the line.

	// See the commentary in sendPacket() for details, but basically
	// we need to read this, and send the packets, backwards--from
	// last to first--so our drivers will wind up in the correct state.

	bool tmpUpdate = auto_update_cycle;

	auto_update_cycle = false; // disable auto-updates
	beginUpdate();


	for (DriverNum i=num_drivers-1; i>=0; i--)
	{
		sendPacket( a_state_vector[i] );
	}

	endUpdate();
	auto_update_cycle = tmpUpdate;


}

#endif









