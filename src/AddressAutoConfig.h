#ifndef AddressAutoConfig_h
#define AddressAutoConfig_h

#define AAC_PROTOCOL_NUM_ICMPv6 58

#define AAC_ROUTER_ADVERTISEMENT 134

#define AAC_SUCCESS 0
#define AAC_ERROR_DAD_FAIL -1
#define AAC_ERROR_SLCMD -2
#define AAC_ERROR_TIMEOUT -3

#define AAC_SLAAC_RDNSS 0
#define AAC_SLAAC_DHCP6 1
#define AAC_SFAAC_DHCP6 3

#define AAC_RAO_SLLA 1
#define AAC_RAO_TLLA 2
#define AAC_RAO_PI 3
#define AAC_RAO_RH 4
#define AAC_RAO_MTU 5
#define AAC_RAO_RDNS 25

// Return Type
// AACRTN_
// Address_Auto_Configuration
// Address_Auto_RSRA

#define AACRTN_FAIL 0
#define AACRTN_RSRA 1
#define AACRTN_DHCP 2

#endif
