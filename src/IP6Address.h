/*
  IP6Address.h - Base class that provides IP6Address
  Copyright (c) 2011 Adrian McEwen.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef IP6Address_h
#define IP6Address_h

#include <stdint.h>
#include <IPAddress.h>
//#include "Printable.h"
//#include "WString.h"

// A class to make it easier to handle and pass around IP addresses

class IP6Address : public IPAddress{
private:
    union {
    uint8_t bytes[16];  // IPv6 address
	uint32_t dword[4];
    } _address;

    // Access the raw byte array containing the address.  Because this returns a pointer
    // to the internal structure rather than a copy of the address this function should only
    // be used when you know that the usage of the returned uint8_t* will be transient and not
    // stored.
    uint8_t* raw_address() { return _address.bytes; };

public:
    // Constructors
    IP6Address();
    IP6Address(uint8_t* first_octet, uint8_t length);
    IP6Address(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet);
    IP6Address(uint32_t address);
    IP6Address(const uint8_t *address);

    bool fromString(const char *address);
    bool fromString(const String &address) { return fromString(address.c_str()); }

    // Overloaded cast operator to allow IP6Address objects to be used where a pointer
    // to a four-byte uint8_t array is expected
    #if 0
    operator uint32_t() const { return _address.dword; };
    #endif
    bool operator==(const IP6Address& addr) const { return _address.dword == addr._address.dword; };
    bool operator==(const uint8_t* addr) const;

    // Overloaded index operator to allow getting and setting individual octets of the address
    uint8_t operator[](int index) const { return _address.bytes[index]; };
    uint8_t& operator[](int index) { return _address.bytes[index]; };

    // Overloaded copy operators to allow initialisation of IP6Address objects from other types
    IP6Address& operator=(const uint8_t *address);
    IP6Address& operator=(uint32_t address);

    virtual size_t printTo(Print& p) const;

    friend class EthernetClassv6;
    friend class UDPv6;
    friend class Clientv6;
    friend class Serverv6;
    friend class DhcpClassv6;
    friend class DNSClientv6;
};

//const IP6Address INADDR_NONE(0,0,0,0);

#endif
