// DHCP Library v0.3 - April 25, 2009
// Author: Jordan Terrell - blog.jordanterrell.com

#ifndef Dhcpv6_h
#define Dhcpv6_h

/* DHCP state machine. */
#define STATE_DHCP_START 0
#define STATE_DHCP_DISCOVER 1
#define STATE_DHCP_REQUEST 2
#define STATE_DHCP_LEASED 3
#define STATE_DHCP_REREQUEST 4
#define STATE_DHCP_RELEASE 5

#define DHCP_FLAGSBROADCAST 0x8000

#if 0
// IPv4

/* UDP port numbers for DHCP */
#define DHCP_SERVER_PORT 67 /* from server to client */
#define DHCP_CLIENT_PORT 68 /* from client to server */

/* DHCP message OP code */
#define DHCP_BOOTREQUEST 1
#define DHCP_BOOTREPLY 2

/* DHCP message type */
#define DHCP_DISCOVER 1
#define DHCP_OFFER 2
#define DHCP_REQUEST 3
#define DHCP_DECLINE 4
#define DHCP_ACK 5
#define DHCP_NAK 6
#define DHCP_RELEASE 7
#define DHCP_INFORM 8

#define DHCP_HTYPE10MB 1
#define DHCP_HTYPE100MB 2

#define DHCP_HLENETHERNET 6
#define DHCP_HOPS 0
#define DHCP_SECS 0

#define MAGIC_COOKIE 0x63825363
#define MAX_DHCP_OPT 16

//#define HOST_NAME "WIZnet"
#define DEFAULT_LEASE (900) //default lease time in seconds
#endif

#define DHCP_CHECK_NONE (0)
#define DHCP_CHECK_RENEW_FAIL (1)
#define DHCP_CHECK_RENEW_OK (2)
#define DHCP_CHECK_REBIND_FAIL (3)
#define DHCP_CHECK_REBIND_OK (4)

/* Retry to processing DHCP */
#define MAX_DHCP_RETRY 2  ///< Maxium retry count
#define DHCP_WAIT_TIME 10 ///< Wait Time 10s

/* UDP port numbers for DHCP */
#define DHCP_SERVER_PORT 547 ///< DHCP server port number
#define DHCP_CLIENT_PORT 546 ///< DHCP client port number

#define DCHP_HOST_NAME "WIZnet\0"

/* DHCP6 state machine. */
#define STATE_DHCP6_INIT 0		///< Initialize
#define STATE_DHCP6_SOLICIT 1   ///< send DISCOVER and wait OFFER
#define STATE_DHCP6_REQUEST 2   ///< send REQEUST and wait ACK or NACK
#define STATE_DHCP6_LEASED 3	///< ReceiveD ACK and IP leased
#define STATE_DHCP6_REREQUEST 4 ///< send REQUEST for maintaining leased IP
#define STATE_DHCP6_RELEASE 5   ///< No use
#define STATE_DHCP6_STOP 6		///< Stop procssing DHCP

/* DHCP6 message type */
#define DHCP6_SOLICIT 1		  ///< DISCOVER message in OPT of @ref RIP_MSG
#define DHCP6_ADVERTISE 2	 ///< OFFER message in OPT of @ref RIP_MSG
#define DHCP6_REQUEST 3		  ///< REQUEST message in OPT of @ref RIP_MSG
#define DHCP6_CONFIRM 4		  ///< DECLINE message in OPT of @ref RIP_MSG
#define DHCP6_RENEW 5		  ///< ACK message in OPT of @ref RIP_MSG
#define DHCP6_REBIND 6		  ///< NACK message in OPT of @ref RIP_MSG
#define DHCP6_REPLY 7		  ///< RELEASE message in OPT of @ref RIP_MSG. No use
#define DHCP6_RELEASE 8		  ///< INFORM message in OPT of @ref RIP_MSG. No use
#define DHCP6_DECLINE 9		  ///< INFORM message in OPT of @ref RIP_MSG. No use
#define DHCP6_RECONFIGURE 10  ///< INFORM message in OPT of @ref RIP_MSG. No use
#define DHCP6_INFO_REQUEST 11 ///< INFORM message in OPT of @ref RIP_MSG. No use

#if 0
// IPv4
enum
{
	padOption = 0,
	subnetMask = 1,
	timerOffset = 2,
	routersOnSubnet = 3,
	/* timeServer		=	4,
	nameServer		=	5,*/
	dns = 6,
	/*logServer		=	7,
	cookieServer		=	8,
	lprServer		=	9,
	impressServer		=	10,
	resourceLocationServer	=	11,*/
	hostName = 12,
	/*bootFileSize		=	13,
	meritDumpFile		=	14,*/
	domainName = 15,
	/*swapServer		=	16,
	rootPath		=	17,
	extentionsPath		=	18,
	IPforwarding		=	19,
	nonLocalSourceRouting	=	20,
	policyFilter		=	21,
	maxDgramReasmSize	=	22,
	defaultIPTTL		=	23,
	pathMTUagingTimeout	=	24,
	pathMTUplateauTable	=	25,
	ifMTU			=	26,
	allSubnetsLocal		=	27,
	broadcastAddr		=	28,
	performMaskDiscovery	=	29,
	maskSupplier		=	30,
	performRouterDiscovery	=	31,
	routerSolicitationAddr	=	32,
	staticRoute		=	33,
	trailerEncapsulation	=	34,
	arpCacheTimeout		=	35,
	ethernetEncapsulation	=	36,
	tcpDefaultTTL		=	37,
	tcpKeepaliveInterval	=	38,
	tcpKeepaliveGarbage	=	39,
	nisDomainName		=	40,
	nisServers		=	41,
	ntpServers		=	42,
	vendorSpecificInfo	=	43,
	netBIOSnameServer	=	44,
	netBIOSdgramDistServer	=	45,
	netBIOSnodeType		=	46,
	netBIOSscope		=	47,
	xFontServer		=	48,
	xDisplayManager		=	49,*/
	dhcpRequestedIPaddr = 50,
	dhcpIPaddrLeaseTime = 51,
	/*dhcpOptionOverload	=	52,*/
	dhcpMessageType = 53,
	dhcpServerIdentifier = 54,
	dhcpParamRequest = 55,
	/*dhcpMsg			=	56,
	dhcpMaxMsgSize		=	57,*/
	dhcpT1value = 58,
	dhcpT2value = 59,
	/*dhcpClassIdentifier	=	60,*/
	dhcpClientIdentifier = 61,
	endOption = 255
};

typedef struct _RIP_MSG_FIXED
{
	uint8_t op;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;
	uint32_t xid;
	uint16_t secs;
	uint16_t flags;
	uint8_t ciaddr[4];
	uint8_t yiaddr[4];
	uint8_t siaddr[4];
	uint8_t giaddr[4];
	uint8_t chaddr[6];
} RIP_MSG_FIXED;
#endif

enum
{
	DHCP_FAILED = 0, ///< Procssing Fail
	DHCP_RUNNING,	///< Procssing DHCP proctocol
	DHCP_IP_ASSIGN,  ///< First Occupy IP from DHPC server      (if cbfunc == null, act as default default_ip_assign)
	DHCP_IP_CHANGED, ///< Change IP address by new ip from DHCP (if cbfunc == null, act as default default_ip_update)
	DHCP_IP_LEASED,  ///< Stand by
	DHCP_STOPPED	 ///< Stop procssing DHCP protocol
};

/* 
 * @brief DHCPv6 option (cf. RFC3315)
 */
enum
{
	OPT_CLIENTID = 1,
	OPT_SERVERID = 2,
	OPT_IANA = 3,
	OPT_IATA = 4,
	OPT_IAADDR = 5,
	OPT_REQUEST = 6,
	OPT_PREFERENCE = 7,
	OPT_ELAPSED_TIME = 8,
	OPT_RELAY_MSG = 9,
	OPT_AUTH = 11,
	OPT_UNICAST = 12,
	OPT_STATUS_CODE = 13,
	OPT_RAPID_COMMIT = 14,
	OPT_USER_CLASS = 15,
	OPT_VENDOR_CLASS = 16,
	OPT_VENDOR_OPTS = 17,
	OPT_INTERFACE_ID = 18,
	OPT_RECONF_MSG = 19,
	OPT_RECONF_ACCEPT = 20,
	SIP_Server_DNL = 21,
	SIP_Server_V6ADDR = 22,
	DNS_RecursiveNameServer = 23,
	Domain_Search_List = 24,
	OPT_IAPD = 25,
	OPT_IAPREFIX = 26,
	OPT_NIS_SERVERS = 27,
	OPT_NISP_SERVERS = 28,
	OPT_NIS_DOMAIN_NAME = 29,
	OPT_NISP_DOMAIN_NAME = 30,
	OPT_LIFETIME = 32,
	FQ_DOMAIN_NAME = 39
};

#endif
