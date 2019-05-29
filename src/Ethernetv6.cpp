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
#include "utility/w5100.h"
#include "Dhcpv6.h"
#include "AddressAutoConfig.h"

IP6Address EthernetClassv6::_dnsServerAddressv6;
DhcpClassv6* EthernetClassv6::_dhcpv6 = NULL;

AddressAutoConfig* EthernetClassv6::_addressautoconfig = NULL;

int EthernetClassv6::begin(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout)
{
	//IP6Address ip, IP6Address dns, IP6Address gateway, IP6Address subnet

	// IPv6 Address Auto Configuration
	
	static DhcpClassv6 s_dhcp;
	static AddressAutoConfig s_addressautoconfig;

	_dhcpv6 = &s_dhcp;
	_addressautoconfig = &s_addressautoconfig;

	int8_t result;

	// Initialise the basic info
	if (W5100.init() == 0) return 0;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.setMACAddress(mac);
	W5100.setIPAddress(IP6Address(0,0,0,0).raw_address());

	// Set IPv6
	if (Ethernetv6.linkStatus() == LinkON) {

		// Duplicate_Address_Detection
		_addressautoconfig->Duplicate_Address_Detection(mac);

		// Address Auto Configuration
		// RA -> DHCP

		// Use Socket Number 7
		Serial.println("Address_Auto_Configuration Start");
		result = _addressautoconfig->Address_Auto_Configuration(7);
		SPI.endTransaction();

		if(result == AAC_SLAAC_RDNSS) {
			// Completed

			Serial.println("Address_Auto_Configuration Succeed");
			_dhcpv6->use_sateful = 0;
			return 1;

		} else if(result == AAC_SLAAC_DHCP6) {
			// Need Stateless DHCP
			// Get Other Information

			_dhcpv6->use_sateful = 0;

			Serial.println("Address_Auto_Configuration Failed");
			Serial.println("beginWithDHCP Stateless DHCP Start");

			int ret = _dhcpv6->beginWithDHCPV6(mac, timeout, responseTimeout);

			if (ret == 1) {
				// We've successfully found a DHCP server and got our configuration
				// info, so set things accordingly

				Serial.println("beginWithDHCP Stateless DHCP Succeed");
				return ret;
			} else {

				Serial.println("beginWithDHCP Stateless DHCP Failed");
				return 0;
			}

		} else if(result == AAC_SFAAC_DHCP6) {
			// Need Stateful DHCP
			// Get Managed Information

			_dhcpv6->use_sateful = 1;

			Serial.println("Address_Auto_Configuration Failed");
			Serial.println("beginWithDHCP Stateful DHCP Start");

			int ret = _dhcpv6->beginWithDHCPV6(mac, timeout, responseTimeout);

			if (ret == 1) {
				// We've successfully found a DHCP server and got our configuration
				// info, so set things accordingly

				Serial.println("beginWithDHCP Stateful DHCP Succeed");

				SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
				W5100.setGlobalunicastAddress(_dhcpv6->getGua().raw_address());
				SPI.endTransaction();
				socketPortRand(micros());
				return ret;
			} else {

				Serial.println("beginWithDHCP Stateful DHCP Failed");
				return 0;
			}

		}
	}

	return 0;

}

int EthernetClassv6::begin(uint8_t *mac, IP6Address ip, IP6Address dns, IP6Address gateway, IP6Address subnet, unsigned long timeout, unsigned long responseTimeout)
{
	begin(mac);

	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	
#if ARDUINO > 106 || TEENSYDUINO > 121

	W5100.setIPAddress(ip._address.bytes);
	W5100.setGatewayIp(gateway._address.bytes);
	W5100.setSubnetMask(subnet._address.bytes);
#else

	W5100.setIPAddress(ip._address);
	W5100.setGatewayIp(gateway._address);
	W5100.setSubnetMask(subnet._address);
#endif

	SPI.endTransaction();
}

void EthernetClassv6::begin(uint8_t *mac, IP6Address ip, IP6Address dns, IP6Address gateway, IP6Address subnet, 
IP6Address lla, IP6Address gua, IP6Address sn6, IP6Address gw6)
{
//#error Ethernet.cpp 102

	uint8_t temp1[4];
	IP6Address temp;
	int i;

	if (W5100.init() == 0) return;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.setMACAddress(mac);

#if ARDUINO > 106 || TEENSYDUINO > 121

	W5100.setIPAddress(ip._address.bytes);
	W5100.setGatewayIp(gateway._address.bytes);
	W5100.setSubnetMask(subnet._address.bytes);

	W5100.setLinklocalAddress(lla._address.bytes);
	W5100.setGlobalunicastAddress(gua._address.bytes);
	W5100.setSubnetMask6(sn6._address.bytes);
	W5100.setGateway6(gw6._address.bytes);

#else

	W5100.setIPAddress(ip._address);
	W5100.setGatewayIp(gateway._address);
	W5100.setSubnetMask(subnet._address);

	W5100.setLinklocalAddress(lla._address);
	W5100.setGlobalunicastAddress(gua._address);
	W5100.setSubnetMask6(sn6._address);
	W5100.setGateway6(gw6._address);
	
#endif
	SPI.endTransaction();
	_dnsServerAddressv6 = dns;
}

void EthernetClassv6::init(uint8_t sspin)
{
	W5100.setSS(sspin);
}

EthernetLinkStatus EthernetClassv6::linkStatus()
{
	switch (W5100.getLinkStatus()) {
		case UNKNOWN:  return Unknown;
		case LINK_ON:  return LinkON;
		case LINK_OFF: return LinkOFF;
		default:       return Unknown;
	}
}

EthernetHardwareStatus EthernetClassv6::hardwareStatus()
{
	switch (W5100.getChip()) {
		case 51: return EthernetW5100;
		case 52: return EthernetW5200;
		case 55: return EthernetW5500;
		case 61: return EthernetW6100;
		default: return EthernetNoHardware;
	}
}

int EthernetClassv6::maintain()
{
	int rc = DHCP_CHECK_NONE;
	if (_dhcpv6 != NULL && _dhcpv6->use_sateful == 1) {
		// we have a pointer to dhcp, use it
		rc = _dhcpv6->checkLease();
		switch (rc) {
		case DHCP_CHECK_NONE:
			//nothing done
			break;
		case DHCP_CHECK_RENEW_OK:
			break;
		case DHCP_CHECK_REBIND_OK:
			//we might have got a new IP.
			//PRINTVAR(rc);
			
			Serial.print("My IPv6 GUA: ");
  			Serial.println(Ethernetv6.globalunicastAddress());

			#if 0
			SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
			W5100.setIPAddress(_dhcpv6->getLocalIp().raw_address());
			W5100.setGatewayIp(_dhcpv6->getGatewayIp().raw_address());
			W5100.setSubnetMask(_dhcpv6->getSubnetMask().raw_address());
			SPI.endTransaction();
			_dnsServerAddress = _dhcpv6->getDnsServerIp();
			#endif
			break;
		default:
			//PRINTVAR(rc);
			//this is actually an error, it will retry though
			break;
		}
	}
	return rc;
}


void EthernetClassv6::MACAddress(uint8_t *mac_address)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.getMACAddress(mac_address);
	SPI.endTransaction();
}

IP6Address EthernetClassv6::localIP()
{
	IP6Address ret;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.getIPAddress(ret.raw_address());
	SPI.endTransaction();
	return ret;
}

IP6Address EthernetClassv6::subnetMask()
{
	IP6Address ret;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.getSubnetMask(ret.raw_address());
	SPI.endTransaction();
	return ret;
}

IP6Address EthernetClassv6::gatewayIP()
{
	IP6Address ret;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.getGatewayIp(ret.raw_address());
	SPI.endTransaction();
	return ret;
}

IP6Address EthernetClassv6::linklocalAddress()
{
	IP6Address ret;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.getLinklocalAddress(ret.raw_address());
	SPI.endTransaction();
	return ret;
}

IP6Address EthernetClassv6::globalunicastAddress()
{
	IP6Address ret;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.getGlobalunicastAddress(ret.raw_address());
	SPI.endTransaction();
	return ret;
}

IP6Address EthernetClassv6::subnetmask6()
{
	IP6Address ret;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.getSubnetMask6(ret.raw_address());
	SPI.endTransaction();
	return ret;
}

IP6Address EthernetClassv6::gateway6()
{
	IP6Address ret;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.getGateway6(ret.raw_address());
	SPI.endTransaction();
	return ret;
}

void EthernetClassv6::setMACAddress(const uint8_t *mac_address)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.setMACAddress(mac_address);
	SPI.endTransaction();
}

void EthernetClassv6::setLocalIP(const IP6Address local_ip)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IP6Address ip = local_ip;
	W5100.setIPAddress(ip.raw_address());
	SPI.endTransaction();
}

void EthernetClassv6::setSubnetMask(const IP6Address subnet)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IP6Address ip = subnet;
	W5100.setSubnetMask(ip.raw_address());
	SPI.endTransaction();
}

void EthernetClassv6::setGatewayIP(const IP6Address gateway)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IP6Address ip = gateway;
	W5100.setGatewayIp(ip.raw_address());
	SPI.endTransaction();
}

void EthernetClassv6::setLinklocalAddress(const IP6Address lla)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IP6Address ip = lla;
	W5100.setLinklocalAddress(ip.raw_address());
	SPI.endTransaction();
}

void EthernetClassv6::setGlobalunicastAddress(const IP6Address gua)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IP6Address ip = gua;
	W5100.setGlobalunicastAddress(ip.raw_address());
	SPI.endTransaction();
}

void EthernetClassv6::setSubnetMask6(const IP6Address sn6)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IP6Address ip = sn6;
	W5100.setSubnetMask6(ip.raw_address());
	SPI.endTransaction();
}

void EthernetClassv6::setGateway6(const IP6Address gw6)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IP6Address ip = gw6;
	W5100.setGateway6(ip.raw_address());
	SPI.endTransaction();
}

void EthernetClassv6::setRetransmissionTimeout(uint16_t milliseconds)
{
	if (milliseconds > 6553) milliseconds = 6553;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.setRetransmissionTime(milliseconds * 10);
	SPI.endTransaction();
}

void EthernetClassv6::setRetransmissionCount(uint8_t num)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.setRetransmissionCount(num);
	SPI.endTransaction();
}










EthernetClassv6 Ethernetv6;
