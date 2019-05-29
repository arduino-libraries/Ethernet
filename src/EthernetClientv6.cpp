/* Copyright 2018 Paul Stoffregen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <Arduino.h>
#include "Ethernet.h"
#include "Dnsv6.h"
#include "utility/w5100.h"

int EthernetClientv6::connect(const char * host, uint16_t port)
{
	DNSClientv6 dns; // Look up the host first
	IP6Address remote_addr;

	if (sockindex < MAX_SOCK_NUM) {
		if (Ethernetv6.socketStatus(sockindex) != SnSR::CLOSED) {
			Ethernetv6.socketDisconnect(sockindex); // TODO: should we call stop()?
		}
		sockindex = MAX_SOCK_NUM;
	}
	dns.begin(Ethernetv6.dnsServerIP());
	if (!dns.getHostByName(host, remote_addr)) return 0; // TODO: use _timeout
	return connect(remote_addr, port);
}

int EthernetClientv6::connect(IP6Address ip, uint16_t port)
{
	if (sockindex < MAX_SOCK_NUM) {
		if (Ethernetv6.socketStatus(sockindex) != SnSR::CLOSED) {
			Ethernetv6.socketDisconnect(sockindex); // TODO: should we call stop()?
		}
		sockindex = MAX_SOCK_NUM;
	}
#if defined(ESP8266) || defined(ESP32)
	if (ip == IP6Address((uint32_t)0) || ip == IP6Address(0xFFFFFFFFul)) return 0;
#else
	if (ip == IP6Address(0ul) || ip == IP6Address(0xFFFFFFFFul)) return 0;
#endif
	sockindex = Ethernetv6.socketBegin(SnMR::TCP6, 0);
	if (sockindex >= MAX_SOCK_NUM) return 0;
	Ethernetv6.socketConnect(sockindex, rawIPAddress(ip), port);
	uint32_t start = millis();
	while (1) {
		uint8_t stat = Ethernetv6.socketStatus(sockindex);
		if (stat == SnSR::ESTABLISHED) return 1;
		if (stat == SnSR::CLOSE_WAIT) return 1;
		if (stat == SnSR::CLOSED) return 0;
		if (millis() - start > _timeout) break;
		delay(1);
	}
	Ethernetv6.socketClose(sockindex);
	sockindex = MAX_SOCK_NUM;
	return 0;
}

int EthernetClientv6::availableForWrite(void)
{
	if (sockindex >= MAX_SOCK_NUM) return 0;
	return Ethernetv6.socketSendAvailable(sockindex);
}

size_t EthernetClientv6::write(uint8_t b)
{
	return write(&b, 1);
}

size_t EthernetClientv6::write(const uint8_t *buf, size_t size)
{
	if (sockindex >= MAX_SOCK_NUM) return 0;
	if (Ethernetv6.socketSend(sockindex, buf, size)) return size;
	setWriteError();
	return 0;
}

int EthernetClientv6::available()
{
	if (sockindex >= MAX_SOCK_NUM) return 0;
	return Ethernetv6.socketRecvAvailable(sockindex);
	// TODO: do the Wiznet chips automatically retransmit TCP ACK
	// packets if they are lost by the network?  Someday this should
	// be checked by a man-in-the-middle test which discards certain
	// packets.  If ACKs aren't resent, we would need to check for
	// returning 0 here and after a timeout do another Sock_RECV
	// command to cause the Wiznet chip to resend the ACK packet.
}

int EthernetClientv6::read(uint8_t *buf, size_t size)
{
	if (sockindex >= MAX_SOCK_NUM) return 0;
	return Ethernetv6.socketRecv(sockindex, buf, size);
}

int EthernetClientv6::peek()
{
	if (sockindex >= MAX_SOCK_NUM) return -1;
	if (!available()) return -1;
	return Ethernetv6.socketPeek(sockindex);
}

int EthernetClientv6::read()
{
	uint8_t b;
	if (Ethernetv6.socketRecv(sockindex, &b, 1) > 0) return b;
	return -1;
}

void EthernetClientv6::flush()
{
	while (sockindex < MAX_SOCK_NUM) {
		uint8_t stat = Ethernetv6.socketStatus(sockindex);
		if (stat != SnSR::ESTABLISHED && stat != SnSR::CLOSE_WAIT) return;
		if (Ethernetv6.socketSendAvailable(sockindex) >= W5100.SSIZE) return;
	}
}

void EthernetClientv6::stop()
{
	if (sockindex >= MAX_SOCK_NUM) return;

	// attempt to close the connection gracefully (send a FIN to other side)
	Ethernetv6.socketDisconnect(sockindex);
	unsigned long start = millis();

	// wait up to a second for the connection to close
	do {
		if (Ethernetv6.socketStatus(sockindex) == SnSR::CLOSED) {
			sockindex = MAX_SOCK_NUM;
			return; // exit the loop
		}
		delay(1);
	} while (millis() - start < _timeout);

	// if it hasn't closed, close it forcefully
	Ethernetv6.socketClose(sockindex);
	sockindex = MAX_SOCK_NUM;
}

uint8_t EthernetClientv6::connected()
{
	if (sockindex >= MAX_SOCK_NUM) return 0;

	uint8_t s = Ethernetv6.socketStatus(sockindex);
	return !(s == SnSR::LISTEN || s == SnSR::CLOSED || s == SnSR::FIN_WAIT ||
		(s == SnSR::CLOSE_WAIT && !available()));
}

uint8_t EthernetClientv6::status()
{
	if (sockindex >= MAX_SOCK_NUM) return SnSR::CLOSED;
	return Ethernetv6.socketStatus(sockindex);
}

// the next function allows us to use the client returned by
// EthernetServer::available() as the condition in an if-statement.
bool EthernetClientv6::operator==(const EthernetClientv6& rhs)
{
	if (sockindex != rhs.sockindex) return false;
	if (sockindex >= MAX_SOCK_NUM) return false;
	if (rhs.sockindex >= MAX_SOCK_NUM) return false;
	return true;
}

// https://github.com/per1234/EthernetMod
// from: https://github.com/ntruchsess/Arduino-1/commit/937bce1a0bb2567f6d03b15df79525569377dabd
uint16_t EthernetClientv6::localPort()
{
	if (sockindex >= MAX_SOCK_NUM) return 0;
	uint16_t port;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	port = W5100.readSnPORT(sockindex);
	SPI.endTransaction();
	return port;
}

// https://github.com/per1234/EthernetMod
// returns the remote IP address: http://forum.arduino.cc/index.php?topic=82416.0
IP6Address EthernetClientv6::remoteIP()
{
	if (sockindex >= MAX_SOCK_NUM) return IP6Address((uint32_t)0);
	uint8_t remoteIParray[16];
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);

	if(W5100.readSnMR(sockindex) == W6100_SnMR_TCPD) {

		// TCP DUAL
		if((W5100.readSnESR(sockindex) & W6100_SnESR_TCP6) == W6100_SnESR_TCP6)	{
			W5100.readSnDIP6R(sockindex, remoteIParray);
		} else {
			W5100.readSnDIPR(sockindex, remoteIParray);
			memset(&remoteIParray[4], 0, 16-4);
		}

	} else if(W5100.readSnMR(sockindex) == W6100_SnMR_TCP4) {

		// TCP IPv4
		W5100.readSnDIPR(sockindex, remoteIParray);
		memset(&remoteIParray[4], 0, 16-4);

	} else {
		
		// TCP IPv6
		W5100.readSnDIP6R(sockindex, remoteIParray);
	}

	SPI.endTransaction();
	return IP6Address(remoteIParray);
}

// https://github.com/per1234/EthernetMod
// from: https://github.com/ntruchsess/Arduino-1/commit/ca37de4ba4ecbdb941f14ac1fe7dd40f3008af75
uint16_t EthernetClientv6::remotePort()
{
	if (sockindex >= MAX_SOCK_NUM) return 0;
	uint16_t port;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	port = W5100.readSnDPORT(sockindex);
	SPI.endTransaction();
	return port;
}

uint8_t EthernetClientv6::IPVis()
{
	uint8_t ipv;

	if (sockindex >= MAX_SOCK_NUM) return IP6Address((uint32_t)0);
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);

	if(W5100.readSnMR(sockindex) == W6100_SnMR_TCPD) {

		// TCP DUAL
		if((W5100.readSnESR(sockindex) & W6100_SnESR_TCP6) == W6100_SnESR_TCP6)	{
			// IPv6
			ipv = 6;
		} else {
			// IPv4
			ipv = 4;
		}

	} else if(W5100.readSnMR(sockindex) == W6100_SnMR_TCP4) {

		// TCP IPv4
		ipv = 4;

	} else {
		
		// TCP IPv6
		ipv = 6;
	}

	SPI.endTransaction();
	return ipv;
}

