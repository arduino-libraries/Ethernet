// Arduino DNS client for WizNet5100-based Ethernet shield
// (c) Copyright 2009-2010 MCQN Ltd.
// Released under Apache License, version 2.0

#ifndef DNSClientv6_h
#define DNSClientv6_h

#include "Ethernet.h"

class DNSClientv6
{
public:
	void begin(const IP6Address& aDNSServer);

	/** Convert a numeric IP address string into a four-byte IP address.
	    @param aIPAddrString IP address to convert
	    @param aResult IP6Address structure to store the returned IP address
	    @result 1 if aIPAddrString was successfully converted to an IP address,
	            else error code
	*/
	int inet_aton(const char *aIPAddrString, IP6Address& aResult);

	/** Resolve the given hostname to an IP address.
	    @param aHostname Name to be resolved
	    @param aResult IP6Address structure to store the returned IP address
	    @result 1 if aIPAddrString was successfully converted to an IP address,
	            else error code
	*/
	int getHostByName(const char* aHostname, IP6Address& aResult, uint16_t timeout=5000);

protected:
	uint16_t BuildRequest(const char* aName);
	uint16_t ProcessResponse(uint16_t aTimeout, IP6Address& aAddress);

	IP6Address iDNSServer;
	uint16_t iRequestId;
	EthernetUDPv6 iUdp;
};

#endif
