/*
 *  Icmp.cpp: Library to send/receive ICMP packets with the Arduino ethernet shield.
 *  This version only offers minimal wrapping of socket.cpp
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

#ifndef EthernetICMP_h
#define EthernetICMP_h

#include <Ethernet.h>
#include "utility/w5100.h"



/*
 * Copyright (c) 2010 by Blake Foster <blfoster@vassar.edu>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */


#define REQ_DATASIZE 64
#define ICMP_ECHOREPLY 0
#define ICMP_ECHOREQ 8
#define ICMP_ECHOREP 0
#define TIME_EXCEEDED 11
#define PING_TIMEOUT 1000


// ICMPPING_ASYNCH_ENABLE -- define this to enable asynch operations
// #define ICMPPING_ASYNCH_ENABLE

// ICMPPING_INSERT_YIELDS -- some platforms, such as ESP8266, like
// (read: need) to do background work so control must be yielded
// back to the main system periodically when you are doing something
// that takes a good while.
// Define (uncomment the following line) on these platforms, which
// will call a short delay() at critical junctures.
// #define ICMPPING_INSERT_YIELDS

typedef unsigned long icmp_time_t;

class EthernetICMPHeader;
class EthernetICMPPing;

typedef enum Status
{
    /*
    Indicates whether a ping succeeded or failed due to one of various error
    conditions. These correspond to error conditions that occur in this
    library, not anything defined in the ICMP protocol.
    */
    SUCCESS = 0,
    SEND_TIMEOUT = 1, // Timed out sending the request
    NO_RESPONSE = 2, // Died waiting for a response
    BAD_RESPONSE = 3, // we got back the wrong type
    ASYNC_SENT = 4
} Status;


struct EthernetICMPHeader
{
    /*
    Header for an ICMP packet. Does not include the IP header.
    */
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
};


struct EthernetICMPEcho
{
    /*
    Contents of an ICMP echo packet, including the ICMP header. Does not
    include the IP header.
    */

    /*
    This constructor sets all fields and calculates the checksum. It is used
    to create ICMP packet data when we send a request.
    @param type: ICMP_ECHOREQ or ICMP_ECHOREP.
    @param _id: Some arbitrary id. Usually set once per process.
    @param _seq: The sequence number. Usually started at zero and incremented
    once per request.
    @param payload: An arbitrary chunk of data that we expect to get back in
    the response.
    */
    EthernetICMPEcho(uint8_t type, uint16_t _id, uint16_t _seq, uint8_t * _payload);

    /*
    This constructor leaves everything zero. This is used when we receive a
    response, since we nuke whatever is here already when we copy the packet
    data out of the W5100.
    */
    EthernetICMPEcho();

    EthernetICMPHeader icmpHeader;
    uint16_t id;
    uint16_t seq;
    icmp_time_t time;
    uint8_t payload [REQ_DATASIZE];

    /*
    Serialize the header as a byte array, in big endian format.
    */
    void serialize(byte * binData) const;
    /*
    Serialize the header as a byte array, in big endian format.
    */
    void deserialize(byte const * binData);
};


struct EthernetICMPEchoReply
{
    /*
    Struct returned by ICMPPing().
    @param data: The packet data, including the ICMP header.
    @param ttl: Time to live
    @param status: SUCCESS if the ping succeeded. One of various error codes
    if it failed.
    @param addr: The ip address that we received the response from. Something
    is borked if this doesn't match the IP address we pinged.
    */
    EthernetICMPEcho data;
    uint8_t ttl;
    Status status;
    IPAddress addr;
};


class EthernetICMPPing
{
    /*
    Function-object for sending ICMP ping requests.
    */

public:
    /*
    Construct an ICMP ping object.
    @param socket: The socket number in the W5100.
    @param id: The id to put in the ping packets. Can be pretty much any
    arbitrary number.
    */
    EthernetICMPPing(SOCKET s, uint8_t id);


    /*
     Control the ping timeout (ms).  Defaults to PING_TIMEOUT (1000ms) but can
     be set using setTimeout(MS).
     @param timeout_ms: Timeout for ping replies, in milliseconds.
     @note: this value is static -- i.e. system-wide for all ICMPPing objects.
     */
    static void setTimeout(uint16_t setTo) { ping_timeout = setTo;}

    /*
     Fetch the current setting for ping timeouts (in ms).
     @return: timeout for all ICMPPing requests, in milliseconds.
     */
    static uint16_t timeout() { return ping_timeout;}


    /*
    Pings the given IP address.
    @param addr: IP address to ping, as an array of four octets.
    @param nRetries: Number of times to rety before giving up.
    @return: An ICMPEchoReply containing the response. The status field in
    the return value indicates whether the echo request succeeded or
    failed. If the request failed, the status indicates the reason for
    failure on the last retry.
    */
    EthernetICMPEchoReply operator()(const IPAddress&, int nRetries);

    /*
    This overloaded version of the () operator takes a (hopefully blank)
    ICMPEchoReply as parameter instead of constructing one internally and
    then copying it on return. This creates a very small improvement in
    efficiency at the cost of making your code uglier.
    @param addr: IP address to ping, as an array of four octets.
    @param nRetries: Number of times to rety before giving up.
    @param result: ICMPEchoReply that will hold the result.
    */
    void operator()(const IPAddress& addr, int nRetries, EthernetICMPEchoReply& result);



    /*
     Use setPayload to set custom data for all ICMP packets
     by passing it an array of [REQ_DATASIZE].  E.g.
       uint8_t myPayload[REQ_DATASIZE] = { ... whatever ...};
       ICMPPing ping(pingSocket, (uint16_t)random(0, 255));
       ping.setPayload(myPayload);
       // ... as usual ...

     @param payload: pointer to start of REQ_DATASIZE array of bytes to use as payload

    */
    void setPayload(uint8_t * payload);

#ifdef ICMPPING_ASYNCH_ENABLE
    /*
     Asynchronous ping methods -- only enabled if ICMPPING_ASYNCH_ENABLE is defined, above.

     These methods are used to start a ping request, go do something else, and 
     come back later to check if the results are in. A complete example is in the 
     examples directory but the gist of it is E.g.


       // say we're in some function, to simplify things...
       IPAddress pingAddr(74,125,26,147); // ip address to ping

       EthernetICMPPing ping(0, (uint16_t)random(0, 255));
       EthernetICMPEchoReply theResult;

       if (! asyncStart(pingAddr, 3, theResult))
       {
       	   // well, this didn't start off on the right foot
       	   Serial.print("Echo request send failed; ");
       	   Serial.println((int)theResult.status);

       	   //
       	   return; // forget about this
       }

       // ok, ping has started...
       while (! ping.asyncComplete(theResult)) {

       	   // whatever needs handling while we wait on results
       	   doSomeStuff();
       	   doSomeOtherStuff();
       	   delay(30);

       }

       // we get here means we either got a response, or timed out...
       if (theResult.status == SUCCESS)
       {
       	   // yay... do something.
       } else {
       	   // boooo... do something else.
       }

       return;


     */ 


    /*
     asyncStart -- begins a new ping request, asynchronously.  Parameters are the 
     same as for regular ping, but the method returns false on error.

     @param addr: IP address to ping, as an array of four octets.
     @param nRetries: Number of times to rety before giving up.
     @param result: ICMPEchoReply that will hold a status == ASYNC_SENT on success.
     @return: true on async request sent, false otherwise.
     @author: Pat Deegan, http://psychogenic.com
    */
    bool asyncStart(const IPAddress& addr, int nRetries, EthernetICMPEchoReply& result);


    /*
     asyncComplete --  check if the asynchronous ping is done.
     This can be either because of a successful outcome (reply received)
     or because of an error/timeout.

     @param result: ICMPEchoReply that will hold the result.
     @return: true if the result ICMPEchoReply contains the status/other data,
              false if we're still waiting for it to complete.
     @author: Pat Deegan, http://psychogenic.com
    */
    bool asyncComplete(EthernetICMPEchoReply& result);
#endif

private:

    // holds the timeout, in ms, for all objects of this class.
    static uint16_t ping_timeout;

    void openSocket();
    void closeSocket();

    Status sendEchoRequest(const IPAddress& addr, const EthernetICMPEcho& echoReq);
    void receiveEchoReply(const EthernetICMPEcho& echoReq, const IPAddress& addr, EthernetICMPEchoReply& echoReply);



#ifdef ICMPPING_ASYNCH_ENABLE
    // extra internal state/methods used when asynchronous pings
    // are enabled.
    bool asyncSend(EthernetICMPEchoReply& result);
    uint8_t _curSeq;
    uint8_t _numRetries;
    icmp_time_t _asyncstart;
    Status _asyncstatus;
    IPAddress	_addr;
#endif
    uint8_t _id;
    uint8_t _nextSeq;
    SOCKET _socket;
    uint8_t _attempt;

    uint8_t _payload[REQ_DATASIZE];
};

#pragma pack(1)

#endif
