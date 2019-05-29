/*
  IP6Address.cpp - Base class that provides IP6Address
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

#include <Arduino.h>
#include <IP6Address.h>

IP6Address::IP6Address()
{
    _address.dword[0] = 0;
    _address.dword[1] = 0;
    _address.dword[2] = 0;
    _address.dword[3] = 0;
}

IP6Address::IP6Address(uint8_t* first_octet, uint8_t length)
{
    int i;

    for(i=0; i<length; i++)
    {
        _address.bytes[i] = *(first_octet+i);
        
    }
}

IP6Address::IP6Address(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet)
{
    _address.bytes[0] = first_octet;
    _address.bytes[1] = second_octet;
    _address.bytes[2] = third_octet;
    _address.bytes[3] = fourth_octet;
}

IP6Address::IP6Address(uint32_t address)
{
    _address.dword[0] = address;
    _address.dword[1] = 0;
    _address.dword[2] = 0;
    _address.dword[3] = 0;
}

IP6Address::IP6Address(const uint8_t *address)
{
    memcpy(_address.bytes, address, sizeof(_address.bytes));
}

bool IP6Address::fromString(const char *address)
{
    uint16_t acc = 0; // Accumulator
    uint8_t dots = 0;

    while (*address)
    {
        char c = *address++;
        if (c >= '0' && c <= '9')
        {
            acc = acc * 10 + (c - '0');
            if (acc > 255) {
                // Value out of [0..255] range
                return false;
            }
        }
        else if (c == '.')
        {
            if (dots == 3) {
                // Too much dots (there must be 3 dots)
                return false;
            }
            _address.bytes[dots++] = acc;
            acc = 0;
        }
        else
        {
            // Invalid char
            return false;
        }
    }

    if (dots != 3) {
        // Too few dots (there must be 3 dots)
        return false;
    }
    _address.bytes[3] = acc;
    return true;
}

IP6Address& IP6Address::operator=(const uint8_t *address)
{
    memcpy(_address.bytes, address, sizeof(_address.bytes));
    return *this;
}

IP6Address& IP6Address::operator=(uint32_t address)
{
    _address.dword[0] = address;
    _address.dword[1] = 0;
    _address.dword[2] = 0;
    _address.dword[3] = 0;
    return *this;
}

bool IP6Address::operator==(const uint8_t* addr) const
{
    return memcmp(addr, _address.bytes, sizeof(_address.bytes)) == 0;
}

size_t IP6Address::printTo(Print& p) const
{
    size_t n = 0;

    // IPv4
    // 0.1.2.3

    // IPv6
    // 01:23:45:67:89:1011:1213:1415
#if 1

    if( _address.bytes[4] == 0 && _address.bytes[5] == 0 && 
        _address.bytes[6] == 0 && _address.bytes[7] == 0 && 
        _address.bytes[8] == 0 && _address.bytes[9] == 0 && 
        _address.bytes[10] == 0 && _address.bytes[11] == 0 && 
        _address.bytes[12] == 0 && _address.bytes[13] == 0 && 
        _address.bytes[14] == 0 && _address.bytes[15] == 0) {

        for (int i =0; i < 3; i++)
        {
            n += p.print(_address.bytes[i], DEC);
            n += p.print('.');
        }
        n += p.print(_address.bytes[3], DEC);

    } else {
        n += p.print("[");
        for (int i =0; i < 16; i+=2) {
            if(_address.bytes[i] > 15) {
                n += p.print(_address.bytes[i], HEX);
            } else {
                n += p.print('0');
                n += p.print(_address.bytes[i], HEX);
            }

            if(_address.bytes[i+1] > 15) {
                n += p.print(_address.bytes[i+1], HEX);
            } else {
                n += p.print('0');
                n += p.print(_address.bytes[i+1], HEX);
            }
            
            if(i != 14)
            n += p.print(':');
        }
        n += p.print("]");
    }
    
#else
    
    n += p.print("[");
    for (int i =0; i < 16; i+=2) {
        if(_address.bytes[i] > 15) {
            n += p.print(_address.bytes[i], HEX);
        } else {
            n += p.print('0');
            n += p.print(_address.bytes[i], HEX);
        }

        if(_address.bytes[i+1] > 15) {
            n += p.print(_address.bytes[i+1], HEX);
        } else {
            n += p.print('0');
            n += p.print(_address.bytes[i+1], HEX);
        }
        
        if(i != 14)
        n += p.print(':');
    }
    n += p.print("]");
#endif
    return n;
}

