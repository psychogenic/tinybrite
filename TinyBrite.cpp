/*

 TinyBrite.cpp 
 Copyright (C) 2013 Pat Deegan.  All rights reserved.
 TinyBrite (a library to speak to *Brite and other A6281-based devices) implementation.
 

 http://www.flyingcarsandstuff.com/projects/tinybrite/


 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 PARTICULAR PURPOSE.

 See file LICENSE.txt for further informations on licensing terms.


 */

#include "TinyBrite.h"

#define TMB_SETCOLORPACKET(packet, red, green, blue) \
	packet.green = green; \
	packet.red = red; \
	packet.blue = blue; \
	packet.mode_pwm = TA6281_MODE_PWM;

#define TMB_SETCOMMANDPACKET(packet, correctRed, correctGreen, correctBlue, clockMode) \
	packet.greenDotCorrect = correctGreen; \
	packet.redDotCorrect = correctRed; \
	packet.blueDotCorrect = correctBlue; \
	packet.clockMode = clockMode; \
	packet.mode_correct = TA6281_MODE_CORRECT;

TinyBrite::TinyBrite(uint8_t num_brites, bool auto_updates) :
		TinyA6281(num_brites, auto_updates) {
}

BritePacket TinyBrite::colorPacket(TinyBriteColorValue red, TinyBriteColorValue green,
		TinyBriteColorValue blue) {
	BritePacket mpacket = {value:0};
	TMB_SETCOLORPACKET(mpacket, red, green, blue);
	return mpacket;
}

BritePacket TinyBrite::commandPacket(unsigned int redCorrect,
		unsigned int greenCorrect, unsigned int blueCorrect,
		unsigned char clockMode) {

	BritePacket mpacket = {value:0};
	TMB_SETCOMMANDPACKET(mpacket, redCorrect, greenCorrect, blueCorrect,
			clockMode);

	return mpacket;
}

#define CREATE_TA6281PACKET_FROM_MEGABRITEPACKET(ta_packet_name, mb_packet) \
	A6281Packet ta_packet_name = {value:mb_packet.value};

void TinyBrite::sendPacket(BritePacket packet, uint8_t num_times) {
	CREATE_TA6281PACKET_FROM_MEGABRITEPACKET(ta_packet, packet);

	TinyA6281::sendPacket (ta_packet, num_times);

}

void TinyBrite::sendPackets(BritePacket * packets, uint8_t numPackets) {

	TinyA6281::sendPackets((A6281Packet *) packets, numPackets);

}

void TinyBrite::sendPacketToAll(BritePacket packet) {

	CREATE_TA6281PACKET_FROM_MEGABRITEPACKET(ta_packet, packet);

	TinyA6281::sendPacketToAll (ta_packet);

}

void TinyBrite::sendColor(TinyBriteColorValue red, TinyBriteColorValue green,
		TinyBriteColorValue blue) {

	BritePacket mpacket = {value:0};
	TMB_SETCOLORPACKET(mpacket, red, green, blue);
	sendPacket (mpacket);

}

void TinyBrite::sendCommand(unsigned int redDotCorrect,
		unsigned int greenDotCorrect, unsigned int blueDotCorrect,
		unsigned char clockMode) {

	BritePacket mpacket = {value:0};
	TMB_SETCOMMANDPACKET(mpacket, redDotCorrect, greenDotCorrect,
			blueDotCorrect, clockMode);

	sendPacket (mpacket);

}

