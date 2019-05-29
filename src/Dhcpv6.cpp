// DHCP Library v0.3 - April 25, 2009
// Author: Jordan Terrell - blog.jordanterrell.com

#include <Arduino.h>
#include "Ethernet.h"
#include "Dhcpv6.h"
#include "utility/w5100.h"

//#define DHCP6_DEBUG

extern uint8_t DNS6_Address[16];

typedef struct
{
	uint8_t *OPT; ///< Option
} __attribute__((packed)) RIP_MSG;

uint32_t DHCP_XID; // Any number

RIP_MSG pDHCPMSG; // Buffer pointer for DHCP processing

uint16_t DUID_type_s;
uint16_t Hardware_type_s;
uint8_t Time_s[4];
uint32_t Enterprise_num_s;
uint8_t Server_MAC[6];
uint8_t recv_IP[16];
uint32_t PreLifeTime;
uint32_t ValidLifeTime;
uint16_t code;
uint8_t IAID[4];
uint8_t T1[4];
uint8_t T2[4];
uint16_t iana_len;
uint16_t iaaddr_len;
uint16_t statuscode_len;
uint16_t Lstatuscode_len;
uint16_t serverid_len;
uint16_t clientid_len;
uint8_t status_msg[] = "";

uint32_t size;
uint32_t num;
uint32_t num2 = 0;
uint32_t growby;

static uint8_t data_buf[2048] = {
	0,
};

int DhcpClassv6::beginWithDHCPV6(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout)
{
	_dhcpLeaseTime = 0;
	_dhcpT1 = 0;
	_dhcpT2 = 0;
	_timeout = timeout;
	_responseTimeout = responseTimeout;

	// zero out _dhcpMacAddr
	memset(_dhcpMacAddr, 0, 6);
	reset_DHCPV6_lease();

	memcpy((void *)_dhcpMacAddr, (void *)mac, 6);
	_dhcp_state = STATE_DHCP6_INIT;
	return request_DHCPV6_lease();
}

void DhcpClassv6::reset_DHCPV6_lease()
{
	memset(_dhcpGua, 0, 16 * 5);
}

//return:0 on error, 1 if request is sent and response is received
int DhcpClassv6::request_DHCPV6_lease()
{
	uint8_t messageType = 0;
	uint8_t i;
	uint8_t zeroip[4] = {0, 0, 0, 0};

	//==============================================
	// DHCP_init()
	memset(data_buf, 0, sizeof(data_buf));
	pDHCPMSG.OPT = data_buf;

	// Pick an initial transaction ID
	_dhcpTransactionId = random(1UL, 2000UL);
	DHCP_XID = htonl(_dhcpTransactionId);

	_dhcpUdpSocket.stop();
	if (_dhcpUdpSocket.begin(DHCP_CLIENT_PORT) == 0)
	{
		// Couldn't get a socket
		return 0;
	}

	//==============================================

	presend_DHCPV6(); // Empty Function

	uint8_t reply;
	uint8_t tmp[16];
	uint32_t toggle = 1;
	uint32_t my_dhcp_retry = 0;

	unsigned long startTime = millis();

#define MY_MAX_DHCP_RETRY 3

	while (1)
	{

		if (use_sateful == 1)
		{
			// stateful
			reply = DHCPV6_run_stateful();
		}
		else
		{
			// stateless
			reply = DHCPV6_run_stateless();
		}

		switch (reply)
		{
		case DHCP_IP_ASSIGN:
		case DHCP_IP_CHANGED:

			toggle = 1;

			if (toggle)
			{
#ifdef DHCP6_DEBUG
				W5100.getGatewayIp(tmp);
				PRINTSTR(getGatewayIp);
				PRINTVAR_DECN(tmp, 4, i);

				W5100.getSubnetMask(tmp);
				PRINTSTR(getSubnetMask);
				PRINTVAR_DECN(tmp, 4, i);

				W5100.getIPAddress(tmp);
				PRINTSTR(getIPAddress);
				PRINTVAR_DECN(tmp, 4, i);
#endif
				toggle = 0;

				_dhcpUdpSocket.stop();
			}
			break;

		case DHCP_IP_LEASED:

			if (toggle)
			{
#ifdef DHCP6_DEBUG
				W5100.getMACAddress(tmp);
				PRINTSTR(getMACAddress);
				PRINTVAR_HEXN(tmp, 6, i);

				W5100.getLinklocalAddress(tmp);
				PRINTSTR(getLinklocalAddress);
				PRINTVAR_HEXN(tmp, 16, i);

				W5100.getGlobalunicastAddress(tmp);
				PRINTSTR(getGlobalunicastAddress);
				PRINTVAR_HEXN(tmp, 16, i);

				W5100.getGateway6(tmp);
				PRINTSTR(getGateway6);
				PRINTVAR_HEXN(tmp, 16, i);
				PRINTSTR(DNS6_Address);
				PRINTVAR_HEXN(DNS6_Address, 16, i);
#endif
				toggle = 0;
			}

			_lastCheckLeaseMillis = millis();
			return 1;
#if 0
		case DHCP_FAILED:

			my_dhcp_retry++;
			if (my_dhcp_retry > MY_MAX_DHCP_RETRY)
			{
				Serial.println(">> DHCP Failed");

				my_dhcp_retry = 0;

				_dhcpUdpSocket.stop();
				_dhcp_state = STATE_DHCP6_STOP;
			}
			break;
#endif
		default:
			break;
		}

		if ((millis() - startTime) > 60 * 1000)
		{
			Serial.println("DHCPv6 Time Out!");
			my_dhcp_retry++;
			if (my_dhcp_retry > MY_MAX_DHCP_RETRY)
			{
				Serial.println(">> DHCP Failed");

				my_dhcp_retry = 0;

				_dhcpUdpSocket.stop();
				_dhcp_state = STATE_DHCP6_INIT;

				return 0;
			}

			startTime = millis();
		}
	}
	return 0;
}

void DhcpClassv6::send_DHCPV6_SOLICIT(void)
{
	uint16_t j;
	uint8_t ip[16];
	uint8_t rip_msg_size;

	size = 0;
	num = 0;
	growby = 0;

	// send broadcasting packet
	ip[0] = 0xff;
	ip[1] = 0x02;

	for (j = 2; j < 13; j++)
		ip[j] = 0x00;

	ip[13] = 0x01;
	ip[14] = 0x00;
	ip[15] = 0x02;

	IP6Address dest_addr(ip, 16); // Broadcast address

	if (_dhcpUdpSocket.beginPacket(dest_addr, DHCP_SERVER_PORT) == -1)
	{
		Serial.print("DHCP transmit error\n");
		// FIXME Need to return errors
		//return 1;
	}

	InitDhcpV6Option(34, 1);
	DumpDhcpV6Option("option init");

	AppendDhcpV6Option(DHCP6_SOLICIT);

	AppendDhcpV6Option((uint8_t)((DHCP_XID & 0x00FF0000) >> 16));
	AppendDhcpV6Option((uint8_t)((DHCP_XID & 0x00FF0000) >> 8));
	AppendDhcpV6Option((uint8_t)((DHCP_XID & 0x00FF0000) >> 0));
	DumpDhcpV6Option("Type&XID");

// Elapsed time
#if 1
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(OPT_ELAPSED_TIME);
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x02);
	AppendDhcpV6Option(0x0c);
	AppendDhcpV6Option(0x1c);
	DumpDhcpV6Option("Option Elapsed Time");
#endif

	// Client Identifier
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(OPT_CLIENTID);
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x0a); //length
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x03); //DUID_Type
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x01); //Hard_Type
	AppendDhcpV6Option(_dhcpMacAddr[0]);
	AppendDhcpV6Option(_dhcpMacAddr[1]); // MAC Addr
	AppendDhcpV6Option(_dhcpMacAddr[2]);
	AppendDhcpV6Option(_dhcpMacAddr[3]);
	AppendDhcpV6Option(_dhcpMacAddr[4]);
	AppendDhcpV6Option(_dhcpMacAddr[5]);
	DumpDhcpV6Option("Option Client ID");

	// Identity Association for Non-temporary Address
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(OPT_IANA);
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x0c); // length
	AppendDhcpV6Option(0x03);
	AppendDhcpV6Option(0x00); // IAID
	AppendDhcpV6Option(0x08);
	AppendDhcpV6Option(0xdc);
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x00); // T1
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x00); // T2
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x00);
	DumpDhcpV6Option("Option IANA");

// Fully Qualified Domain Name
#if 0
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(39);
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(0x06); // length
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(0x04);
	AppendDhcpV6Option(0x44);AppendDhcpV6Option(0x45);
	AppendDhcpV6Option(0x44);AppendDhcpV6Option(0x59);DumpDhcpV6Option("Option FQ Domain Name");
#endif

// Vendor Class
#if 0
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(16);
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(0x0e); // length
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x01);AppendDhcpV6Option(0x37);
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(0x08);
	AppendDhcpV6Option(0x4d);AppendDhcpV6Option(0x53);
	AppendDhcpV6Option(0x46);AppendDhcpV6Option(0x54);
	AppendDhcpV6Option(0x20);AppendDhcpV6Option(0x35);
	AppendDhcpV6Option(0x2e);AppendDhcpV6Option(0x30);DumpDhcpV6Option("Option Vendor Class");
#endif

// Option Request
#if 0
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(OPT_REQUEST);
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(0x08); // length
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(OPT_VENDOR_OPTS);
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(DNS_RecursiveNameServer);
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(Domain_Search_List);
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(FQ_DOMAIN_NAME);DumpDhcpV6Option("Option Request");
#endif

	rip_msg_size = size;

	_dhcpUdpSocket.write((uint8_t *)pDHCPMSG.OPT, rip_msg_size);
	_dhcpUdpSocket.endPacket();
}

uint8_t DhcpClassv6::send_DHCPV6_REQUEST(uint8_t type)
{
	//uint16_t i;
	uint16_t j;
	uint8_t ip[16];
	uint8_t rip_msg_size;
	uint8_t ret = 1;

	size = 0;
	num = 0;
	growby = 0;

	if (iana_len == 0)
	{
		return 9;
	}

	// send broadcasting packet
	ip[0] = 0xff;
	ip[1] = 0x02;

	for (j = 2; j < 13; j++)
		ip[j] = 0x00;

	ip[13] = 0x01;
	ip[14] = 0x00;
	ip[15] = 0x02;

	IP6Address dest_addr(ip, 16); // Broadcast address

	if (_dhcpUdpSocket.beginPacket(dest_addr, DHCP_SERVER_PORT) == -1)
	{
		Serial.print("DHCP transmit error\n");
		// FIXME Need to return errors
		return 1;
	}

	InitDhcpV6Option(60, 1);
	DumpDhcpV6Option("option init");

	AppendDhcpV6Option(type);
	AppendDhcpV6Option((uint8_t)((DHCP_XID & 0x00FF0000) >> 16));
	AppendDhcpV6Option((uint8_t)((DHCP_XID & 0x0000FF00) >> 8));
	AppendDhcpV6Option((uint8_t)((DHCP_XID & 0x000000FF) >> 0));
	DumpDhcpV6Option("Type&XID");

	// Elapsed time
	//AppendDhcpV6Option(0x00);AppendDhcpV6Option(OPT_ELAPSED_TIME);
	//AppendDhcpV6Option(0x00);AppendDhcpV6Option(0x02);
	//AppendDhcpV6Option(0x0c);AppendDhcpV6Option(0x1c);DumpDhcpV6Option("Option Elapsed Time");

	// Identity Association for Non-temporary Address

	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(OPT_IANA);
	AppendDhcpV6Option((uint8_t)(iana_len >> 8));
	AppendDhcpV6Option((uint8_t)iana_len); // length
	AppendDhcpV6Option(IAID[0]);
	AppendDhcpV6Option(IAID[1]); // IAID
	AppendDhcpV6Option(IAID[2]);
	AppendDhcpV6Option(IAID[3]);
	AppendDhcpV6Option(T1[0]);
	AppendDhcpV6Option(T1[1]); // T1
	AppendDhcpV6Option(T1[2]);
	AppendDhcpV6Option(T1[3]);
	AppendDhcpV6Option(T2[0]);
	AppendDhcpV6Option(T2[1]); // T2
	AppendDhcpV6Option(T2[2]);
	AppendDhcpV6Option(T2[3]);
	DumpDhcpV6Option("Option IANA");

	// IA Address

	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(OPT_IAADDR);
	AppendDhcpV6Option((uint8_t)(iaaddr_len >> 8));
	AppendDhcpV6Option((uint8_t)iaaddr_len); // length
	AppendDhcpV6Option(recv_IP[0]);
	AppendDhcpV6Option(recv_IP[1]); // IP
	AppendDhcpV6Option(recv_IP[2]);
	AppendDhcpV6Option(recv_IP[3]);
	AppendDhcpV6Option(recv_IP[4]);
	AppendDhcpV6Option(recv_IP[5]);
	AppendDhcpV6Option(recv_IP[6]);
	AppendDhcpV6Option(recv_IP[7]);
	AppendDhcpV6Option(recv_IP[8]);
	AppendDhcpV6Option(recv_IP[9]);
	AppendDhcpV6Option(recv_IP[10]);
	AppendDhcpV6Option(recv_IP[11]);
	AppendDhcpV6Option(recv_IP[12]);
	AppendDhcpV6Option(recv_IP[13]);
	AppendDhcpV6Option(recv_IP[14]);
	AppendDhcpV6Option(recv_IP[15]);
	AppendDhcpV6Option((uint8_t)(PreLifeTime >> 24));
	AppendDhcpV6Option((uint8_t)(PreLifeTime >> 16));
	AppendDhcpV6Option((uint8_t)(PreLifeTime >> 8));
	AppendDhcpV6Option((uint8_t)PreLifeTime);
	AppendDhcpV6Option((uint8_t)(ValidLifeTime >> 24));
	AppendDhcpV6Option((uint8_t)(ValidLifeTime >> 16));
	AppendDhcpV6Option((uint8_t)(ValidLifeTime >> 8));
	AppendDhcpV6Option((uint8_t)ValidLifeTime);
	DumpDhcpV6Option("Option IA_addr");

// Status code
#if 0
	// 20190318
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(OPT_STATUS_CODE);DumpDhcpV6Option("Option status_code type");
	AppendDhcpV6Option((uint8_t)(Lstatuscode_len>>8));AppendDhcpV6Option((uint8_t)Lstatuscode_len); DumpDhcpV6Option("Option status_code length");// length
	AppendDhcpV6Option((uint8_t)(code>>8));AppendDhcpV6Option((uint8_t)code); DumpDhcpV6Option("Option status_code code");// code
#endif

	//    for(i=0; i<(statuscode_len-2); i++)
	//        AppendDhcpV6Option(status_msg[i]);
	//    DumpDhcpV6Option("Option status_code msg");

#if 0
	// 20190318
	AppendDhcpV6Option(0x41);AppendDhcpV6Option(0x73);
	AppendDhcpV6Option(0x73);AppendDhcpV6Option(0x69);
	AppendDhcpV6Option(0x67);AppendDhcpV6Option(0x6e);
	AppendDhcpV6Option(0x65);AppendDhcpV6Option(0x64);
	AppendDhcpV6Option(0x20);AppendDhcpV6Option(0x61);
	AppendDhcpV6Option(0x6e);AppendDhcpV6Option(0x20);
	AppendDhcpV6Option(0x61);AppendDhcpV6Option(0x64);
	AppendDhcpV6Option(0x64);AppendDhcpV6Option(0x72);
	AppendDhcpV6Option(0x65);AppendDhcpV6Option(0x73);
	AppendDhcpV6Option(0x73);AppendDhcpV6Option(0x2e);
#endif

	// Client Identifier
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(OPT_CLIENTID);
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x0a); //length
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x03); //DUID_Type
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x01); //Hard_Type
	AppendDhcpV6Option(_dhcpMacAddr[0]);
	AppendDhcpV6Option(_dhcpMacAddr[1]); // MAC Addr
	AppendDhcpV6Option(_dhcpMacAddr[2]);
	AppendDhcpV6Option(_dhcpMacAddr[3]);
	AppendDhcpV6Option(_dhcpMacAddr[4]);
	AppendDhcpV6Option(_dhcpMacAddr[5]);
	DumpDhcpV6Option("Option Client ID");
	//    AppendDhcpV6Option(0x00);AppendDhcpV6Option(OPT_CLIENTID);
	//    AppendDhcpV6Option((uint8_t)(clientid_len>>8));AppendDhcpV6Option((uint8_t)clientid_len); //length
	//    AppendDhcpV6Option((uint8_t)(DUID_type>>8));AppendDhcpV6Option((uint8_t)DUID_type); //DUID_Type
	//    AppendDhcpV6Option((uint8_t)(Hardware_type>>8));AppendDhcpV6Option((uint8_t)Hardware_type); //Hard_Type
	//    AppendDhcpV6Option(_dhcpMacAddr[0]);AppendDhcpV6Option(_dhcpMacAddr[1]); // MAC Addr
	//    AppendDhcpV6Option(_dhcpMacAddr[2]);AppendDhcpV6Option(_dhcpMacAddr[3]);
	//    AppendDhcpV6Option(_dhcpMacAddr[4]);AppendDhcpV6Option(_dhcpMacAddr[5]);DumpDhcpV6Option("Option Client ID");

	//Server Identifier
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(OPT_SERVERID);
	AppendDhcpV6Option((uint8_t)(serverid_len >> 8));
	AppendDhcpV6Option((uint8_t)serverid_len); //length
	AppendDhcpV6Option((uint8_t)(DUID_type_s >> 8));
	AppendDhcpV6Option((uint8_t)DUID_type_s); //DUID_Type
	AppendDhcpV6Option((uint8_t)(Hardware_type_s >> 8));
	AppendDhcpV6Option((uint8_t)Hardware_type_s); //Hard_Type
#if 0
	// 20190318
	AppendDhcpV6Option(Time_s[0]);AppendDhcpV6Option(Time_s[1]); // Time
	AppendDhcpV6Option(Time_s[2]);AppendDhcpV6Option(Time_s[3]);
#endif
	AppendDhcpV6Option(Server_MAC[0]);
	AppendDhcpV6Option(Server_MAC[1]); // MAC Addr
	AppendDhcpV6Option(Server_MAC[2]);
	AppendDhcpV6Option(Server_MAC[3]);
	AppendDhcpV6Option(Server_MAC[4]);
	AppendDhcpV6Option(Server_MAC[5]);
	DumpDhcpV6Option("Option Server ID");

	// Fully Qualified Domain Name
#if 0
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(39);
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(0x06); // length
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(0x04);
	AppendDhcpV6Option(0x44);AppendDhcpV6Option(0x45);
	AppendDhcpV6Option(0x44);AppendDhcpV6Option(0x59);DumpDhcpV6Option("Option FQ Domain Name");
#endif

	// Vendor Class
#if 0
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(16);
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(0x0e); // length
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x01);AppendDhcpV6Option(0x37);
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(0x08);
	AppendDhcpV6Option(0x4d);AppendDhcpV6Option(0x53);
	AppendDhcpV6Option(0x46);AppendDhcpV6Option(0x54);
	AppendDhcpV6Option(0x20);AppendDhcpV6Option(0x35);
	AppendDhcpV6Option(0x2e);AppendDhcpV6Option(0x30);DumpDhcpV6Option("Option Vendor Class");
#endif

	// Option Request
#if 0
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(OPT_REQUEST);
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(0x08); // length
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(OPT_VENDOR_OPTS);
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(DNS_RecursiveNameServer);
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(Domain_Search_List);
	AppendDhcpV6Option(0x00);AppendDhcpV6Option(FQ_DOMAIN_NAME);DumpDhcpV6Option("Option Request");
#endif

	rip_msg_size = size;

	_dhcpUdpSocket.write((uint8_t *)pDHCPMSG.OPT, rip_msg_size);
	_dhcpUdpSocket.endPacket();

	return ret;
}

uint8_t DhcpClassv6::DHCPV6_run_stateful()
{
	uint8_t type;
	uint8_t ret;
	uint8_t i;

	if (_dhcp_state == STATE_DHCP6_STOP)
	{
		return DHCP_STOPPED; // Check DHCP6 STOP State
	}

	ret = DHCP_RUNNING;
	type = parseDHCPV6MSG();

	switch (_dhcp_state)
	{
	case STATE_DHCP6_INIT:

		memset(_dhcpGua, 0, 16);
		send_DHCPV6_SOLICIT();

		_dhcp_state = STATE_DHCP6_SOLICIT;
		break;

	case STATE_DHCP6_SOLICIT:
		if (type == DHCP6_ADVERTISE)
		{
			ret = send_DHCPV6_REQUEST(DHCP6_REQUEST);
			if (ret == 9)
			{
				return 0;
			}
			_dhcp_state = STATE_DHCP6_REQUEST;
		}
		break;

	case STATE_DHCP6_REQUEST:

		memcpy(_dhcpGua, recv_IP, 16);
		_dhcp_state = STATE_DHCP6_LEASED;

		return DHCP_IP_LEASED;
		break;

	case STATE_DHCP6_REREQUEST:

		ret = send_DHCPV6_REQUEST(_dhcp_msg);
		if (ret == 9)
		{
			PRINTLINE();
			return 0;
		}
		_dhcp_state = 7; // set default for wait received reply
		break;

	default:

		if (type == DHCP6_REPLY)
		{
			_dhcp_state = STATE_DHCP6_LEASED;

			return DHCP_IP_LEASED;
		}
		break;
	}
	return ret;
}

uint8_t DhcpClassv6::DHCPV6_run_stateless()
{
	uint8_t type;
	uint8_t ret;

	if (_dhcp_state == STATE_DHCP6_STOP)
	{
		return DHCP_STOPPED; // Check DHCP6 STOP State
	}

	ret = DHCP_RUNNING;
	type = parseDHCPV6MSG();

	switch (_dhcp_state)
	{
	case STATE_DHCP6_INIT:
		send_DHCPV6_INFOREQ();
		_dhcp_state = STATE_DHCP6_RELEASE;
		break;

	case STATE_DHCP6_RELEASE:
		_dhcp_state = STATE_DHCP6_LEASED;
		return DHCP_IP_LEASED;

	default:
		break;
	}

	return ret;
}

void DhcpClassv6::InitDhcpV6Option(unsigned asize, unsigned agrowby)
{
	size = asize;
	growby = agrowby;
	num = 0;
}

void DhcpClassv6::DumpDhcpV6Option(char *sMark)
{
	unsigned i;

#ifdef DHCP6_DEBUG
	Serial.print(sMark);
	Serial.print(" => size =");
	Serial.print(size);
	Serial.print(", num = ");
	Serial.print(num);
	Serial.println("\r\n");

	for (i = num2; i < num; i++)
	{
		Serial.print("0x");
		Serial.print(pDHCPMSG.OPT[i], HEX);
		Serial.print(" ");
	}
	Serial.println("\r\n");
#endif
	num2 = num;
}

void DhcpClassv6::AppendDhcpV6Option(uint8_t value)
{
	uint32_t need;

	need = num + 1;
	if (need > size)
	{
		size++;
	}
	pDHCPMSG.OPT[num] = value;
	num++;
}

uint8_t DhcpClassv6::send_DHCPV6_INFOREQ(void)
{
	uint16_t j;
	uint8_t ip[16];
	uint8_t rip_msg_size;
	uint8_t ret = 0;

	size = 0;
	num = 0;
	growby = 0;

	// send broadcasting packet
	ip[0] = 0xff;
	ip[1] = 0x02;

	for (j = 2; j < 13; j++)
		ip[j] = 0x00;

	ip[13] = 0x01;
	ip[14] = 0x00;
	ip[15] = 0x02;

	IP6Address dest_addr(ip, 16); // Broadcast address

	if (_dhcpUdpSocket.beginPacket(dest_addr, DHCP_SERVER_PORT) == -1)
	{
		Serial.print("DHCP transmit error\n");
		// FIXME Need to return errors
		return 1;
	}

	InitDhcpV6Option(30, 1);
	DumpDhcpV6Option("option init");

	AppendDhcpV6Option(DHCP6_INFO_REQUEST);
	AppendDhcpV6Option((uint8_t)((DHCP_XID & 0x00FF0000) >> 16));
	AppendDhcpV6Option((uint8_t)((DHCP_XID & 0x0000FF00) >> 8));
	AppendDhcpV6Option((uint8_t)((DHCP_XID & 0x000000FF) >> 0));
	DumpDhcpV6Option("Type&XID");

	// Client Identifier
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(OPT_CLIENTID);

	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x0a); //length

	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x03); //DUID_Type

	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x01); //Hard_Type

	AppendDhcpV6Option(_dhcpMacAddr[0]);
	AppendDhcpV6Option(_dhcpMacAddr[1]); // MAC Addr

	AppendDhcpV6Option(_dhcpMacAddr[2]);
	AppendDhcpV6Option(_dhcpMacAddr[3]);

	AppendDhcpV6Option(_dhcpMacAddr[4]);
	AppendDhcpV6Option(_dhcpMacAddr[5]);
	DumpDhcpV6Option("Option Client ID");

	// Option Request
	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(OPT_REQUEST);

	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(0x06); // length

	// AppendDhcpV6Option(0x00);
	// AppendDhcpV6Option(OPT_VENDOR_OPTS);

	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(DNS_RecursiveNameServer);

	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(Domain_Search_List);

	AppendDhcpV6Option(0x00);
	AppendDhcpV6Option(OPT_LIFETIME);
	DumpDhcpV6Option("Option Request");

	rip_msg_size = size;

	_dhcpUdpSocket.write((uint8_t *)pDHCPMSG.OPT, rip_msg_size);
	_dhcpUdpSocket.endPacket();

	return ret;
}

int8_t DhcpClassv6::parseDHCPV6MSG()
{
	uint8_t svr_addr[16];
	uint16_t svr_port;
	uint8_t addlen;
	uint16_t len;
	uint8_t *p;
	uint8_t *e;
	uint8_t type, i;
	uint16_t opt_len;
	uint32_t end_point;

	// temp
	uint32_t ploopcnt;

	len = _dhcpUdpSocket.parsePacket();
	if (len != 0)
	{
		_dhcpUdpSocket.read((uint8_t *)pDHCPMSG.OPT, len);
	}

	type = 0;
	p = (uint8_t *)(pDHCPMSG.OPT);
	e = p + len;

	i = 0;

	if (*p == DHCP6_ADVERTISE || *p == DHCP6_REPLY)
	{
		type = *p++; // type
		p++;		 // xid[0]
		p++;		 // xid[1]
		p++;		 // xid[2]

		while (p < e)
		{
			p++;
			switch (*p)
			{
			case OPT_CLIENTID:
				p++;

				opt_len = (*p++ << 8);
				clientid_len = opt_len + (*p++);
				end_point = (uint32_t)p + clientid_len;

				while ((uint32_t)p != end_point)
				{
					p++;
				}
				break;

			case OPT_IANA:
				p++;
				opt_len = (*p++ << 8);
				iana_len = opt_len + (*p++);
				end_point = (uint32_t)p + iana_len;

				//IAID
				memcpy(IAID, p, 4);
				p += 4;

				//T1
				memcpy(T1, p, 4);
				p += 4;

				_dhcpT1 = T1[0] | T1[1] << 8 | T1[2] << 16 | T1[3] << 24;
				_dhcpT1 = ntohl(_dhcpT1); // sec
				_renewInSec = _dhcpT1;

				//T2
				memcpy(T2, p, 4);
				p += 4;

				_dhcpT2 = T2[0] | T2[1] << 8 | T2[2] << 16 | T2[3] << 24;
				_dhcpT2 = ntohl(_dhcpT2); // sec
				_rebindInSec = _dhcpT2;

				//IA_NA-options
				while ((uint32_t)p < end_point)
				{
					p++;
					switch (*p)
					{
					case OPT_IAADDR:
						p++;
						opt_len = (*p++ << 8);
						iaaddr_len = opt_len + (*p++);

						memcpy(recv_IP, p, 16);
						p += 16;

						PreLifeTime = (*p++ << 24);
						PreLifeTime += (*p++ << 16);
						PreLifeTime += (*p++ << 8);
						PreLifeTime += (*p++);

						ValidLifeTime = (*p++ << 24);
						ValidLifeTime += (*p++ << 16);
						ValidLifeTime += (*p++ << 8);
						ValidLifeTime += (*p++);

						_dhcpLeaseTime = ValidLifeTime; // sec
						break;

					case OPT_STATUS_CODE:
						p++;
						opt_len = (*p++ << 8);
						statuscode_len = opt_len + (*p++);
						Lstatuscode_len = statuscode_len;

						code = (*p++ << 8);
						code = code + (*p++);

						p += statuscode_len - 2;
						break;

					default:
						p++;
						opt_len = (*p++ << 8);
						opt_len = opt_len + (*p++);
						p += opt_len;
						break;
					}
				}
				break;

			case OPT_IATA:
				p++;
				opt_len = (*p++ << 8);
				opt_len = opt_len + (*p++);
				//IA_TA-options
				p += opt_len;
				break;

			case OPT_SERVERID:
				p++;
				opt_len = (*p++ << 8);
				serverid_len = opt_len + (*p++);

				end_point = (uint32_t)p + serverid_len;

				DUID_type_s = (*p++ << 8);
				DUID_type_s = DUID_type_s + (*p++);

				if (DUID_type_s == 0x02)
				{
					Enterprise_num_s = (*p++ << 24);
					Enterprise_num_s = Enterprise_num_s + (*p++ << 16);
					Enterprise_num_s = Enterprise_num_s + (*p++ << 8);
					Enterprise_num_s = Enterprise_num_s + (*p++);
				}
				else
				{
					Hardware_type_s = (*p++ << 8);
					Hardware_type_s = Hardware_type_s + (*p++);
				}

				if (DUID_type_s == 0x01)
				{
					memcpy(Time_s, p, 4);
					p += 4;
				}

				memcpy(Server_MAC, p, 6);
				p += 6;

				while ((uint32_t)p != end_point)
				{
					p++;
				}
				break;

			case DNS_RecursiveNameServer:
				p++;
				opt_len = (*p++ << 8);
				opt_len = opt_len + (*p++);
				end_point = (uint32_t)p + opt_len;

				memcpy(DNS6_Address, p, 16);
				p += 16;

				while ((uint32_t)p < end_point)
				{
					p++;
				}
				break;

			case Domain_Search_List:
				p++;
				opt_len = (*p++ << 8);
				opt_len = opt_len + (*p++);
				end_point = (uint32_t)p + opt_len;

				while ((uint32_t)p < end_point)
				{
					p++;
				}
				break;

			default:
				p++;
				opt_len = (*p++ << 8);
				opt_len = opt_len + (*p++);
				p += opt_len;
				break;
			}
		}
	}
	return type;
}

void DhcpClassv6::presend_DHCPV6()
{
}

int DhcpClassv6::checkLease()
{
	int rc = DHCP_CHECK_NONE;

	unsigned long now = millis();
	unsigned long elapsed = now - _lastCheckLeaseMillis;

	// if more then one sec passed, reduce the counters accordingly
	if (elapsed >= 1000)
	{
		// set the new timestamps
		_lastCheckLeaseMillis = now - (elapsed % 1000);
		elapsed = elapsed / 1000;

		// decrease the counters by elapsed seconds
		// we assume that the cycle time (elapsed) is fairly constant
		// if the remainder is less than cycle time * 2
		// do it early instead of late
		if (_renewInSec < elapsed * 2)
		{
			_renewInSec = 0;
		}
		else
		{
			_renewInSec -= elapsed;
		}

		if (_rebindInSec < elapsed * 2)
		{
			_rebindInSec = 0;
		}
		else
		{
			_rebindInSec -= elapsed;
		}

#ifdef DHCP6_DEBUG
		PRINTVAR(_dhcp_state);
		PRINTVAR(_renewInSec);
		PRINTVAR(_rebindInSec);
#endif
	}
	// if we have a lease but should renew, do it
	if (_renewInSec == 0 && _dhcp_state == STATE_DHCP6_LEASED)
	{
		_dhcp_state = STATE_DHCP6_REREQUEST;
		_dhcp_msg = DHCP6_RENEW;

		// request_DHCPV6_lease return
		// 0 : failed
		// 1 : dhcp complete

		rc = 1 + request_DHCPV6_lease();
	}

	// if we have a lease or is renewing but should bind, do it
	if (_rebindInSec == 0 && (_dhcp_state == STATE_DHCP6_LEASED ||
							  _dhcp_state == STATE_DHCP6_INIT))
	{
		_dhcp_state = STATE_DHCP6_INIT;
		//_dhcp_msg = DHCP6_REBIND;

		// this should basically restart completely
		reset_DHCPV6_lease();
		rc = 3 + request_DHCPV6_lease();
	}
	return rc;
}

IP6Address DhcpClassv6::getGua()
{
	return IP6Address(_dhcpGua);
}
#if 0
IP6Address DhcpClassv6::getSubnetMask()
{
	return IP6Address(_dhcpSubnetMask);
}

IP6Address DhcpClassv6::getGatewayIp()
{
	return IP6Address(_dhcpGatewayIp);
}

IP6Address DhcpClassv6::getDhcpServerIp()
{
	return IP6Address(_dhcpDhcpServerIp);
}

IP6Address DhcpClassv6::getDnsServerIp()
{
	return IP6Address(_dhcpDnsServerIp);
}
#endif

void DhcpClassv6::printByte(char *buf, uint8_t n)
{
	char *str = &buf[1];
	buf[0] = '0';
	do
	{
		unsigned long m = n;
		n /= 16;
		char c = m - 16 * n;
		*str-- = c < 10 ? c + '0' : c + 'A' - 10;
	} while (n);
}
