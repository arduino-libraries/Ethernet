#include <Arduino.h>
#include "Ethernet.h"
#include "AddressAutoConfig.h"
#include "utility/w5100.h"

//#define AAC_INFO_DEBUG

uint8_t DNS6_Address[16] = {
	0,
};

uint8_t AddressAutoConfig::Duplicate_Address_Detection(uint8_t *mac_addr)
{
	uint8_t result;

	uint8_t WIZ_LLA[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	uint8_t flags;
	uint8_t tmp_array[16];
	uint8_t i;

	W5100.writeSLRTR(2000);
	W5100.writeSLRCR(5);

	Generate_EUI64(mac_addr, WIZ_LLA);

	W5100.writeSLDIP6R(WIZ_LLA); //target address setting

	//setSLIMR(SLIR_TIOUT|SLIR_NS); //only external interrupt???

	if (W5100.readSLCR() != 0x00) //check clear CMD
	{
		Serial.println("ERROR : RQCMD is not clear : ");
		Serial.println(W5100.readSLCR());
		while (1)
			;
	}

	W5100.writeSLCR(W6100_SLCR_NS);

#ifdef AAC_INFO_DEBUG
	Serial.println("Wait RQIR.....");
#endif

	do
	{
		flags = W5100.readSLIR();

		if (flags & W6100_SLIR_TIOUT)
		{
#ifdef AAC_INFO_DEBUG
			Serial.println("Timeout !!! DAD SUCCESSED");
#endif
			W5100.writeLLAR(WIZ_LLA);
			result = AAC_SUCCESS;
		}
		else if (flags & W6100_SLCR_NS)
		{
			Serial.println("Received NA !!! DAD FAILED");
			Serial.println("Please, Check your MAC Address");
			result = AAC_ERROR_DAD_FAIL;
		}
	} while (!((flags & W6100_SLIR_TIOUT) || (flags & W6100_SLCR_NS)));

#ifdef AAC_INFO_DEBUG
	PRINTVAR_HEX(flags);
#endif

	W5100.writeSLIRCLR(flags);

	return result;
}

void AddressAutoConfig::Generate_EUI64(uint8_t *mac_addr, uint8_t *Link_Local_Addr)
{
	*Link_Local_Addr = 0xfe;
	*(Link_Local_Addr + 1) = 0x80;
	//00:00:00:00:00:00
	*(Link_Local_Addr + 8) = *(mac_addr); //flip the 7th bit of 1st byte
	*(Link_Local_Addr + 8) ^= 1 << 1;
	*(Link_Local_Addr + 9) = *(mac_addr + 1);
	*(Link_Local_Addr + 10) = *(mac_addr + 2);
	*(Link_Local_Addr + 11) = 0xFF;
	*(Link_Local_Addr + 12) = 0xFE;
	*(Link_Local_Addr + 13) = *(mac_addr + 3);
	*(Link_Local_Addr + 14) = *(mac_addr + 4);
	*(Link_Local_Addr + 15) = *(mac_addr + 5);
}

static uint8_t data_buf[2048] = {
	0,
};

int8_t AddressAutoConfig::Address_Auto_Configuration(uint8_t sn)
{
	int8_t result;

	// RS(Router Solicitation) RA(Router Advertisement)

	result = Address_Auto_RSRA(sn, data_buf, sizeof(data_buf));

	return result;
}

int8_t AddressAutoConfig::Address_Auto_RSRA(uint8_t sn, uint8_t *icmpbuf, uint16_t buf_size)
{
	int8_t result;
	uint8_t ra_cnt = 2; // Set Default Router Counter 2
	uint16_t escape_cnt = 1000000;

	volatile uint16_t size;
	volatile uint16_t size_rsr;
	uint8_t destip[16]={0,};
	uint16_t destport;
	uint8_t addr_len, o_len;
	uint8_t flags;

	uint8_t *p;
	uint8_t *e;
	int i;
	uint8_t o_type, type, code, RA_flag, MO_flag; //, O_flag;
	uint16_t Router_lifetime;
	uint32_t Reachable_time, Retrans_time;
	uint32_t end_point;
	uint8_t prefix_len, pi_flag;
	uint32_t validtime, prefertime, dnstime;
	uint8_t prefix[16]={0,};
	uint8_t subnet[16]={0,};

	if (W5100.readSLCR() != 0x00) //check clear CMD
	{
		Serial.println("ERROR : RQCMD is not clear");
		result = AAC_ERROR_SLCMD;
		return result;
	}

	sn = Ethernetv6.socketBegin(SnMR::IPRAW6, 1024);

	// Packet Send
	W5100.writeICMP6BLKR(W6100_ICMP6BLK_RA);
	W5100.writeSnPNR(sn, AAC_PROTOCOL_NUM_ICMPv6); //ICMPv6 : 58
	W5100.writeSLRTR(4000);						   // SOCKET-less Retransmission Time Register
	W5100.writeSLRCR(0);						   // SOCKET-less Retransmission Count Register
	W5100.writeSLCR(W6100_SLCR_RS);				   // (1<<1) Auto configuration RS Transmission Command

	// Packet Recive
	while (ra_cnt)
	{
#ifdef AAC_INFO_DEBUG
		SPRINT_DEC(\nRemain Revice RA Count:, ra_cnt);
#endif
		do
		{
			if (getSn_RX_RSR(sn) > 0)
			{
				size_rsr = getSn_RX_RSR(sn);
				//PRINTSTR_HEX(getSn_RX_RSR = 0x,size_rsr);

				uint8_t head[8];
				uint16_t pack_len = 0;

				// Read Peer's ver, data lenth
				size = Ethernetv6.socketRecv(sn, head, 2);
				head[0] &= 0x07;
				pack_len = (uint16_t)(head[0] << 8) + head[1];
				size_rsr -= size;

				// Read Peer's IP address, port number
				size = Ethernetv6.socketRecv(sn, destip, 16);
				size_rsr -= size;

				size = Ethernetv6.socketRecv(sn, icmpbuf, size_rsr);
				size_rsr -= size;

				W5100.writeGA6R(destip);
			}

			p = icmpbuf;
			if (escape_cnt > 0)
			{
				escape_cnt--;
			}
			else
			{
#ifdef AAC_INFO_DEBUG
				SPRINT_STR(Not Recived AAC_ROUTER_ADVERTISEMENT);
#endif
				if (*p != AAC_ROUTER_ADVERTISEMENT)
				{
					// Not Recived RA
					Ethernetv6.socketClose(sn);
					return result;
				}
			}
		} while (*p != AAC_ROUTER_ADVERTISEMENT);

		e = p + size;

		// ICMP RA message
		if (*p == AAC_ROUTER_ADVERTISEMENT)
		{
			type = *p++;
			code = *p++;

			p++;
			p++; //checksum
			p++; //Cur hop limit

			RA_flag = *p++;

			Router_lifetime = *p++ << 8;
			Router_lifetime = Router_lifetime + (*p++);

			Reachable_time = *p++ << 24;
			Reachable_time = Reachable_time + (*p++ << 16);
			Reachable_time = Reachable_time + (*p++ << 8);
			Reachable_time = Reachable_time + (*p++);

			Retrans_time = *p++ << 24;
			Retrans_time = Retrans_time + (*p++ << 16);
			Retrans_time = Retrans_time + (*p++ << 8);
			Retrans_time = Retrans_time + (*p++);

#ifdef AAC_INFO_DEBUG
			SPRINT_STR(RA);
			SPRINT_HEX(type : 0x, type);
			SPRINT_HEX(code : 0x, code);
			SPRINT_HEX(RA_flag : 0x, RA_flag);
			SPRINT_DEC(Router_lifetime(s), Router_lifetime);
			SPRINT_DEC(Reachable_time(ms), Reachable_time);
			SPRINT_DEC(Retrans_time(ms), Retrans_time);
#endif
			while (p < e)
			{
				switch (*p)
				{
				case AAC_RAO_SLLA:
				{

					o_type = *p++;
					o_len = (*p++) * 8;
					p += (o_len - 2);

#ifdef AAC_INFO_DEBUG
					SPRINT_DEC(Option Type(Source LLA)
							   :, o_type);
					SPRINT_DEC(Option length:, o_len);
#endif
					break;
				}
				case AAC_RAO_TLLA:
				{
					o_type = *p++;
					o_len = (*p++) * 8;

					p += (o_len - 2);

#ifdef AAC_INFO_DEBUG
					SPRINT_DEC(Option Type(Target LLA)
							   :, o_type);
					SPRINT_DEC(Option length:, o_len);
#endif
					break;
				}
				case AAC_RAO_PI:
				{

					o_type = *p++;
					o_len = (*p++) * 8;

					end_point = (uint32_t)p - 2 + o_len;
					prefix_len = *p++;

					pi_flag = *p++;

					validtime = (*p++ << 24);
					validtime += (*p++ << 16);
					validtime += (*p++ << 8);
					validtime += (*p++);

					prefertime = (*p++ << 24);
					prefertime += (*p++ << 16);
					prefertime += (*p++ << 8);
					prefertime += (*p++);

#ifdef AAC_INFO_DEBUG
					SPRINT_DEC(Option Type(Prefix information)
							   :, o_type);
					SPRINT_DEC(Option length:, o_len);
					SPRINT_DEC(Prefix Length:, prefix_len);
					SPRINT_HEX(Prefix Information Flag : 0x, pi_flag);
					SPRINT_DEC(valid lifetime:, validtime);
					SPRINT_DEC(preferred lifetime:, prefertime);
#endif

					p++;
					p++;
					p++;
					p++; //reserved
					W5100.readLLAR(prefix);
					for (int i = 0; i < prefix_len / 8; i++)
					{
						prefix[i] = *p++;
						subnet[i] = 0xff;
					}
					W5100.writeGUAR(prefix);
					W5100.writeSUB6R(subnet);

					while ((uint32_t)p != end_point)
					{
						p++;
					}
#ifdef AAC_INFO_DEBUG
					SPRINT_STR(prefix:);
					for (i = 0; i < prefix_len / 8; i++)
					{
						if (prefix[i] > 16)
						{
							Serial.print(prefix[i], HEX);
						}
						else
						{
							Serial.print("0");
							Serial.print(prefix[i], HEX);
						}

						if (1 == (i % 2))
							Serial.print(":");
					}
					Serial.println(":");
#endif
					break;
				}
				case AAC_RAO_RH:
				{
					o_type = *p++;
					o_len = (*p++) * 8;

#ifdef AAC_INFO_DEBUG
					SPRINT_DEC(Option Type
							   : (Redirected Header), o_type);
					SPRINT_DEC(Option length:, o_len);
#endif
					p += (o_len - 2);
					break;
				}
				case AAC_RAO_MTU:
				{
					o_type = *p++;
					o_len = (*p++) * 8;

#ifdef AAC_INFO_DEBUG
					SPRINT_DEC(Option Type
							   : (MTU), o_type);
					SPRINT_DEC(Option length:, o_len);
#endif
					p += (o_len - 2);
					break;
				}
				case AAC_RAO_RDNS:
				{
					o_type = *p++;
					o_len = (*p++) * 8;

					end_point = (uint32_t)p - 2 + o_len;
					p++;
					p++; //reserved

					dnstime = (*p++ << 24);
					dnstime += (*p++ << 16);
					dnstime += (*p++ << 8);
					dnstime += (*p++);

					DNS6_Address[0] = *p++;
					DNS6_Address[1] = *p++;
					DNS6_Address[2] = *p++;
					DNS6_Address[3] = *p++;
					DNS6_Address[4] = *p++;
					DNS6_Address[5] = *p++;
					DNS6_Address[6] = *p++;
					DNS6_Address[7] = *p++;
					DNS6_Address[8] = *p++;
					DNS6_Address[9] = *p++;
					DNS6_Address[10] = *p++;
					DNS6_Address[11] = *p++;
					DNS6_Address[12] = *p++;
					DNS6_Address[13] = *p++;
					DNS6_Address[14] = *p++;
					DNS6_Address[15] = *p++;

					while ((uint32_t)p != end_point)
					{
						p++;
					}

#ifdef AAC_INFO_DEBUG
					SPRINT_DEC(Option Type
							   : (Recursive DNS Server), o_type);
					SPRINT_DEC(Option length:, o_len);
					SPRINT_DEC(DNS lifetime:, dnstime);

					SPRINT_STR(DNS IP:);
					for (i = 0; i < 15; i++)
					{

						if (prefix[i] > 16)
						{
							Serial.print(DNS6_Address[i], HEX);
						}
						else
						{
							Serial.print("0");
							Serial.print(DNS6_Address[i], HEX);
						}

						if (1 == (i % 2))
							Serial.print(":");
					}
					Serial.println(DNS6_Address[15], HEX);
#endif
					break;
				}
				default:
				{
					o_type = *p++;
					o_len = (*p++) * 8;
					p += (o_len - 2);

#ifdef AAC_INFO_DEBUG
					SPRINT_STR(default);
					SPRINT_DEC(o_type:, o_type);
					SPRINT_DEC(o_len:, o_len);
#endif
					break;
				}
				} // ICMP option
			}	 //while
		}

		//=========================================================
		// RA_flag
		// Managed address configuration	(1<<7)
		// Other configuration				(1<<6)
		// Home Agent						(1<<5)
		// Pfr (Default Router Preference)	(3<<3)
		// Proxy							(1<<2)
		// Reserved							(1<<1)
		//=========================================================

		//M_flag = RA_flag >> 7;	// Get Managed Flag
		MO_flag = RA_flag >> 6; // Get Managed and Other Flag

#ifdef AAC_INFO_DEBUG
		SPRINT_HEX(RA_flag : 0x, RA_flag);
		SPRINT_HEX(MO : 0x, MO_flag);
#endif
		if (MO_flag == AAC_SLAAC_RDNSS)
		{
			// Success RS-RA
			ra_cnt = 0;
			result = MO_flag;
		}
		else
		{
			// Fail RS-RA
			// Retry..
			ra_cnt--;

			result = MO_flag;
			memset(icmpbuf, 0, 2048);
		}
	}

	Ethernetv6.socketClose(sn);

	return result;
}

uint16_t AddressAutoConfig::getSn_RX_RSR(uint8_t s)
{
	uint16_t val, prev;

	prev = W5100.readSnRX_RSR(s);
	while (1)
	{
		val = W5100.readSnRX_RSR(s);
		if (val == prev)
		{
			return val;
		}
		prev = val;
	}
}
