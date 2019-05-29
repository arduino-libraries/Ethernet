/*
 *  Udp.cpp: Library to send/receive UDP packets with the Arduino ethernet shield.
 *  This version only offers minimal wrapping of socket.cpp
 *  Drop Udp.h/.cpp into the Ethernet library directory at hardware/libraries/Ethernet/
 *
 * MIT License:
 * Copyright (c) 2008 Bjoern Hartmann
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * bjoern@cs.stanford.edu 12/30/2008
 */

#include <Arduino.h>
#include "Ethernet.h"
#include "Dnsv6.h"
#include "utility/w5100.h"

/* Start EthernetUDP socket, listening at local port PORT */
uint8_t EthernetUDPv6::begin(uint16_t port)
{
	#if 0
	PRINTVAR(port);
	#endif

	if (sockindex < MAX_SOCK_NUM) Ethernetv6.socketClose(sockindex);
	sockindex = Ethernetv6.socketBegin(SnMR::UDP6, port);

	// Set Source IPv6 to GUA
	W5100.writeSnPSR(sockindex, W6100_SnPSR_GUA);

	if (sockindex >= MAX_SOCK_NUM) return 0;
	_port = port;
	_remaining = 0;
	return 1;
}

uint8_t EthernetUDPv6::begin(uint16_t port, uint8_t ipv4)
{
	if(ipv4 == 1)
	{
		#if 0
		PRINTSTR(IPv4);
		#endif
	}
	if (sockindex < MAX_SOCK_NUM) Ethernetv6.socketClose(sockindex);
	PRINTLINE();
	sockindex = Ethernetv6.socketBegin(SnMR::UDP, port);
	PRINTLINE();
	if (sockindex >= MAX_SOCK_NUM) return 0;
	PRINTLINE();
	_port = port;
	_remaining = 0;
	return 1;
}

/* return number of bytes available in the current packet,
   will return zero if parsePacket hasn't been called yet */
int EthernetUDPv6::available()
{
	return _remaining;
}

/* Release any resources being used by this EthernetUDP instance */
void EthernetUDPv6::stop()
{
	if (sockindex < MAX_SOCK_NUM) {
		Ethernetv6.socketClose(sockindex);
		sockindex = MAX_SOCK_NUM;
	}
}

int EthernetUDPv6::beginPacket(const char *host, uint16_t port)
{
	// Look up the host first
	int ret = 0;
	DNSClientv6 dns;
	IP6Address remote_addr;

	dns.begin(Ethernetv6.dnsServerIP());
	ret = dns.getHostByName(host, remote_addr);
	if (ret != 1) return ret;
	return beginPacket(remote_addr, port);
}

int EthernetUDPv6::beginPacket(IP6Address ip, uint16_t port)
{
	_offset = 0;
	//Serial.printf("UDP beginPacket\n");
	return Ethernetv6.socketStartUDP(sockindex, rawIPAddress(ip), port);
}

int EthernetUDPv6::endPacket()
{
	return Ethernetv6.socketSendUDP(sockindex);
}

size_t EthernetUDPv6::write(uint8_t byte)
{
	return write(&byte, 1);
}

size_t EthernetUDPv6::write(const uint8_t *buffer, size_t size)
{
	//Serial.printf("UDP write %d\n", size);
	uint16_t bytes_written = Ethernetv6.socketBufferData(sockindex, _offset, buffer, size);
	_offset += bytes_written;
	return bytes_written;
}

int EthernetUDPv6::parsePacket()
{
	// discard any remaining bytes in the last packet
	while (_remaining) {
		// could this fail (loop endlessly) if _remaining > 0 and recv in read fails?
		// should only occur if recv fails after telling us the data is there, lets
		// hope the w5100 always behaves :)
		read((uint8_t *)NULL, _remaining);
	}

	if (Ethernetv6.socketRecvAvailable(sockindex) > 0) {
		//HACK - hand-parse the UDP packet using TCP recv method
		uint8_t tmpBuf[20];
		int ret=0;
		int i;

		if(W5100.getChip() == 61) {
			//read 2 header bytes and get one IPv4 or IPv6
			ret = Ethernetv6.socketRecv(sockindex, tmpBuf, 2);
			if(ret > 0) {
				_remaining = (tmpBuf[0] & (0x7))<<8 | tmpBuf[1];

				if((tmpBuf[0] & W6100_UDP_HEADER_IPV) == W6100_UDP_HEADER_IPV6) {
					// IPv6 UDP Recived
					// 0 1
					// 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17
					// 18 19

					//read 16 header bytes and get IP and port from it
					ret = Ethernetv6.socketRecv(sockindex, &tmpBuf[2], 18);
					_remoteIP = &tmpBuf[2];					
					_remotePort = (tmpBuf[18]<<8) | tmpBuf[19];
				} else {
					// IPv4 UDP Recived
					// 0 1
					// 2 3 4 5
					// 6 7

					//read 6 header bytes and get IP and port from it
					ret = Ethernetv6.socketRecv(sockindex, &tmpBuf[2], 6);
					_remoteIP = &tmpBuf[2];
					_remotePort = (tmpBuf[6]<<8) | tmpBuf[7];
				}

				ret = _remaining;
			}
		} else {
			//read 8 header bytes and get IP and port from it
			ret = Ethernetv6.socketRecv(sockindex, tmpBuf, 8);

			if (ret > 0) {

				_remoteIP = tmpBuf;
				_remotePort = tmpBuf[4];
				_remotePort = (_remotePort << 8) + tmpBuf[5];
				_remaining = tmpBuf[6];
				_remaining = (_remaining << 8) + tmpBuf[7];

				// When we get here, any remaining bytes are the data
				ret = _remaining;
			}
		}
		return ret;
	}
	// There aren't any packets available
	return 0;
}

int EthernetUDPv6::read()
{
	uint8_t byte;

	if ((_remaining > 0) && (Ethernetv6.socketRecv(sockindex, &byte, 1) > 0)) {
		// We read things without any problems
		_remaining--;
		return byte;
	}

	// If we get here, there's no data available
	return -1;
}

int EthernetUDPv6::read(unsigned char *buffer, size_t len)
{
	if (_remaining > 0) {
		int got;
		if (_remaining <= len) {
			// data should fit in the buffer
			got = Ethernetv6.socketRecv(sockindex, buffer, _remaining);
		} else {
			// too much data for the buffer,
			// grab as much as will fit
			got = Ethernetv6.socketRecv(sockindex, buffer, len);
		}
		if (got > 0) {
			_remaining -= got;
			//Serial.printf("UDP read %d\n", got);
			return got;
		}
	}
	// If we get here, there's no data available or recv failed
	return -1;
}

int EthernetUDPv6::peek()
{
	// Unlike recv, peek doesn't check to see if there's any data available, so we must.
	// If the user hasn't called parsePacket yet then return nothing otherwise they
	// may get the UDP header
	if (sockindex >= MAX_SOCK_NUM || _remaining == 0) return -1;
	return Ethernetv6.socketPeek(sockindex);
}

void EthernetUDPv6::flush()
{
	// TODO: we should wait for TX buffer to be emptied
}

/* Start EthernetUDP socket, listening at local port PORT */
uint8_t EthernetUDPv6::beginMulticast(IP6Address ip, uint16_t port)
{
	if (sockindex < MAX_SOCK_NUM) Ethernetv6.socketClose(sockindex);
	sockindex = Ethernetv6.socketBeginMulticast(SnMR::UDP6 | SnMR::MULTI, ip, port);
	if (sockindex >= MAX_SOCK_NUM) return 0;
	_port = port;
	_remaining = 0;
	return 1;
}

