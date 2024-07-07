#ifndef _ETHERNETSP_H_
#define _ETHERNETSP_H_

// reset
#define SJ1_12 (21) // Jumper setting is SJ1=12, use D21/EMMC_DATA3
#define SJ1_23 (26) // Jumper setting is SJ1=23, use D26/I2S_BCK

// cs
#define SJ2_12 (19) // Jumper setting is SJ2=12, use D19/I2S_DIN
#define SJ2_23 (24) // Jumper setting is SJ2=23, use D24/SPI5_CS_X


#if defined(ARDUINO_ARCH_SPRESENSE)

#include "Ethernet.h"
void W5500ETH_reset(uint8_t sj1_rst)
{
	pinMode(sj1_rst, OUTPUT);
	digitalWrite(sj1_rst, LOW);
	delay(50);
	digitalWrite(sj1_rst, HIGH);
	return;
}

#else
	#error ("This library use only Sony spresense with W5500-Ether add-on board.")
#endif



#endif
