/*
 * Copyright (c) 2010 by Arduino LLC. All rights reserved.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef	W5x00_H_INCLUDED
#define	W5x00_H_INCLUDED

#include <SPI.h>

#define ETHERNET_SHIELD_SPI_CS 10

typedef uint8_t SOCKET;

#define IDM_OR  0x8000
#define IDM_AR0 0x8001
#define IDM_AR1 0x8002
#define IDM_DR  0x8003
/*
class MR {
public:
  static const uint8_t RST   = 0x80;
  static const uint8_t PB    = 0x10;
  static const uint8_t PPPOE = 0x08;
  static const uint8_t LB    = 0x04;
  static const uint8_t AI    = 0x02;
  static const uint8_t IND   = 0x01;
};
*/
/*
class IR {
public:
  static const uint8_t CONFLICT = 0x80;
  static const uint8_t UNREACH  = 0x40;
  static const uint8_t PPPoE    = 0x20;
  static const uint8_t SOCK0    = 0x01;
  static const uint8_t SOCK1    = 0x02;
  static const uint8_t SOCK2    = 0x04;
  static const uint8_t SOCK3    = 0x08;
  static inline uint8_t SOCK(SOCKET ch) { return (0x01 << ch); };
};
*/

class SnMR {
public:
  static const uint8_t CLOSE  = 0x00;
  static const uint8_t TCP    = 0x01;
  static const uint8_t UDP    = 0x02;
  static const uint8_t IPRAW  = 0x03;
  static const uint8_t MACRAW = 0x04;
  static const uint8_t PPPOE  = 0x05;
  static const uint8_t ND     = 0x20;
  static const uint8_t MULTI  = 0x80;
};

enum SockCMD {
  Sock_OPEN      = 0x01,
  Sock_LISTEN    = 0x02,
  Sock_CONNECT   = 0x04,
  Sock_DISCON    = 0x08,
  Sock_CLOSE     = 0x10,
  Sock_SEND      = 0x20,
  Sock_SEND_MAC  = 0x21,
  Sock_SEND_KEEP = 0x22,
  Sock_RECV      = 0x40
};

/*class SnCmd {
public:
  static const uint8_t OPEN      = 0x01;
  static const uint8_t LISTEN    = 0x02;
  static const uint8_t CONNECT   = 0x04;
  static const uint8_t DISCON    = 0x08;
  static const uint8_t CLOSE     = 0x10;
  static const uint8_t SEND      = 0x20;
  static const uint8_t SEND_MAC  = 0x21;
  static const uint8_t SEND_KEEP = 0x22;
  static const uint8_t RECV      = 0x40;
};
*/

class SnIR {
public:
  static const uint8_t SEND_OK = 0x10;
  static const uint8_t TIMEOUT = 0x08;
  static const uint8_t RECV    = 0x04;
  static const uint8_t DISCON  = 0x02;
  static const uint8_t CON     = 0x01;
};

class SnSR {
public:
  static const uint8_t CLOSED      = 0x00;
  static const uint8_t INIT        = 0x13;
  static const uint8_t LISTEN      = 0x14;
  static const uint8_t SYNSENT     = 0x15;
  static const uint8_t SYNRECV     = 0x16;
  static const uint8_t ESTABLISHED = 0x17;
  static const uint8_t FIN_WAIT    = 0x18;
  static const uint8_t CLOSING     = 0x1A;
  static const uint8_t TIME_WAIT   = 0x1B;
  static const uint8_t CLOSE_WAIT  = 0x1C;
  static const uint8_t LAST_ACK    = 0x1D;
  static const uint8_t UDP         = 0x22;
  static const uint8_t IPRAW       = 0x32;
  static const uint8_t MACRAW      = 0x42;
  static const uint8_t PPPOE       = 0x5F;
};

class IPPROTO {
public:
  static const uint8_t IP   = 0;
  static const uint8_t ICMP = 1;
  static const uint8_t IGMP = 2;
  static const uint8_t GGP  = 3;
  static const uint8_t TCP  = 6;
  static const uint8_t PUP  = 12;
  static const uint8_t UDP  = 17;
  static const uint8_t IDP  = 22;
  static const uint8_t ND   = 77;
  static const uint8_t RAW  = 255;
};

class W5x00Chipset {
public:
  static const uint8_t W5100 = 0;
  static const uint8_t W5200 = 1;
  static const uint8_t W5500 = 2;
};

enum W5x00Linkstatus {
  UNKNOWN,
  LINK_ON,
  LINK_OFF
};

class W5x00Class {

public:
  void init();

  /**
   * @brief	This function is being used for copy the data form Receive buffer of the chip to application buffer.
   * 
   * It calculate the actual physical address where one has to read
   * the data from Receive buffer. Here also take care of the condition while it exceed
   * the Rx memory uper-bound of socket.
   */
  void read_data(SOCKET s, uint16_t src, uint8_t *dst, uint16_t len);
  
  /**
   * @brief	 This function is being called by send() and sendto() function also. 
   * 
   * This function read the Tx write pointer register and after copy the data in buffer update the Tx write pointer
   * register. User should read upper byte first and lower byte later to get proper value.
   */
  void send_data_processing(SOCKET s, const uint8_t *data, uint16_t len);

  /**
   * @brief A copy of send_data_processing that uses the provided ptr for the
   *        write offset.  Only needed for the "streaming" UDP API, where
   *        a single UDP packet is built up over a number of calls to
   *        send_data_processing_ptr, because TX_WR doesn't seem to get updated
   *        correctly in those scenarios
   * @param ptr value to use in place of TX_WR.  If 0, then the value is read
   *        in from TX_WR
   * @return New value for ptr, to be used in the next call
   */
  void send_data_processing_offset(SOCKET s, uint16_t data_offset, const uint8_t *data, uint16_t len);

  /**
   * @brief	This function is being called by recv() also.
   * 
   * This function read the Rx read pointer register
   * and after copy the data from receive buffer update the Rx write pointer register.
   * User should read upper byte first and lower byte later to get proper value.
   */
  void recv_data_processing(SOCKET s, uint8_t *data, uint16_t len, uint8_t peek = 0);

  inline void setGatewayIp(const uint8_t *addr)  { writeGAR(addr); }
  inline void getGatewayIp(uint8_t *addr)        { readGAR(addr);  }

  inline void setSubnetMask(const uint8_t *addr) { writeSUBR(addr); }
  inline void getSubnetMask(uint8_t *addr)       { readSUBR(addr);  }

  inline void setMACAddress(const uint8_t *addr) { writeSHAR(addr); }
  inline void getMACAddress(uint8_t *addr)       { readSHAR(addr);  }

  inline void setIPAddress(const uint8_t *addr)  { writeSIPR(addr); }
  inline void getIPAddress(uint8_t *addr)        { readSIPR(addr);  }

  inline void setRetransmissionTime(uint16_t timeout);
  inline void setRetransmissionCount(uint8_t _retry);

  void execCmdSn(SOCKET s, SockCMD _cmd);
  
  uint16_t getTXFreeSize(SOCKET s);
  uint16_t getRXReceivedSize(SOCKET s);
  
  inline uint8_t getMaxSockets() { return sockets; }

  inline W5x00Linkstatus getLinkStatus();

private:
  static uint8_t chipset;
  static uint8_t sockets;

  // W5x00 Registers
  // ---------------
private:
  static uint8_t write(uint16_t _addr, uint8_t _cb, uint8_t _data);
  static uint16_t write(uint16_t addr, uint8_t _cb, const uint8_t *buf, uint16_t len);
  static inline uint8_t read(uint16_t addr, uint8_t _cb) { return readByte(addr, _cb); }
  static inline uint16_t read(uint16_t addr, uint8_t _cb, uint8_t *buf, uint16_t len) { return readBlock(addr, _cb, buf, len); }

  static uint8_t (*readByte)(uint16_t addr, uint8_t _cb);
  static uint8_t W5100_readByte(uint16_t _addr, uint8_t _cb);
  static uint8_t W5200_readByte(uint16_t _addr, uint8_t _cb);
  static uint8_t W5500_readByte(uint16_t _addr, uint8_t _cb);

  static uint16_t (*readBlock)(uint16_t addr, uint8_t _cb, uint8_t *buf, uint16_t len);
  static uint16_t W5100_readBlock(uint16_t _addr, uint8_t _cb, uint8_t *_buf, uint16_t _len);
  static uint16_t W5200_readBlock(uint16_t _addr, uint8_t _cb, uint8_t *_buf, uint16_t _len);
  static uint16_t W5500_readBlock(uint16_t _addr, uint8_t _cb, uint8_t *_buf, uint16_t _len);
  
#define __GP_REGISTER8(name, address)             \
  static inline void write##name(uint8_t _data) { \
    write(address, 0x04, _data);                  \
  }                                               \
  static inline uint8_t read##name() {            \
    return read(address, 0x00);                   \
  }
#define __GP_REGISTER16(name, address)            \
  static void write##name(uint16_t _data) {       \
    write(address,   0x04, _data >> 8);           \
    write(address+1, 0x04, _data & 0xFF);         \
  }                                               \
  static uint16_t read##name() {                  \
    uint16_t res = read(address, 0x00);           \
    res = (res << 8) + read(address + 1, 0x00);   \
    return res;                                   \
  }
#define __GP_REGISTER_N(name, address, size)          \
  static uint16_t write##name(const uint8_t *_buff) { \
    return write(address, 0x04, _buff, size);         \
  }                                                   \
  static uint16_t read##name(uint8_t *_buff) {        \
    return read(address, 0x00, _buff, size);          \
  }

public:
  __GP_REGISTER8 (MR,                0x0000);    // Mode
  __GP_REGISTER_N(GAR,               0x0001, 4); // Gateway IP address
  __GP_REGISTER_N(SUBR,              0x0005, 4); // Subnet mask address
  __GP_REGISTER_N(SHAR,              0x0009, 6); // Source MAC address
  __GP_REGISTER_N(SIPR,              0x000F, 4); // Source IP address
  __GP_REGISTER8 (IR,                0x0015);    // Interrupt
  __GP_REGISTER8 (IMR,               0x0016);    // Interrupt Mask
  __GP_REGISTER16(RTR_W5100_W5200,   0x0017);    // Timeout address
  __GP_REGISTER8 (SIR_W5500,         0x0017);    // Socket Interrupt
  __GP_REGISTER8 (SIMR_W5500,        0x0018);    // Socket Interrupt Mask
  __GP_REGISTER16(RTR_W5500,         0x0019);    // Timeout address
  __GP_REGISTER8 (RCR_W5100_W5200,   0x0019);    // Retry count
  __GP_REGISTER8 (RMSR,              0x001A);    // Receive memory size
  __GP_REGISTER8 (RCR_W5500,         0x001B);    // Retry count
  __GP_REGISTER8 (TMSR,              0x001B);    // Transmit memory size
  __GP_REGISTER8 (PATR,              0x001C);    // Authentication type address in PPPoE mode
  __GP_REGISTER8 (PTIMER_W5500,      0x001C);    // PPP LCP Request Timer
  __GP_REGISTER8 (PMAGIC_W5500,      0x001D);    // PPP LCP Magic Number
  __GP_REGISTER8 (PPPALGO_W5200,     0x001E);    // Authentication algorithm in PPPoE
  __GP_REGISTER_N(PHAR_W5500,        0x001E, 6); // PPP Destination MAC Address
  __GP_REGISTER8 (VERSIONR_W5200,    0x001F);    // Chip version
  __GP_REGISTER16(PSID_W5500,        0x0024);    // PPP Session Identification
  __GP_REGISTER16(PMRU_W5500,        0x0026);    // PPP Maximum segment Size
  __GP_REGISTER8 (PTIMER,            0x0028);    // PPP LCP Request Timer
  __GP_REGISTER_N(UIPR_W5500,        0x0028, 4); // Unreachable IP address in UDP mode
  __GP_REGISTER8 (PMAGIC,            0x0029);    // PPP LCP Magic Number
  __GP_REGISTER_N(UIPR_W5100_W5200,  0x002A, 4); // Unreachable IP address in UDP mode
  __GP_REGISTER16(UPORT_W5500,       0x002C);    // Unreachable Port address in UDP mode
  __GP_REGISTER16(UPORT_W5100_W5200, 0x002E);    // Unreachable Port address in UDP mode
  __GP_REGISTER8 (PHYCFGR_W5500,     0x002E);    // PHY Configuration register, default value: 0b 1011 1xxx
  __GP_REGISTER16(INTLEVEL_W5200,    0x0030);    // Interrupt Low Level Timer
  __GP_REGISTER8 (IR2_W5200,         0x0034);    // Socket Interrupt
  __GP_REGISTER8 (PSTATUS_W5200,     0x0035);    // PHY Status
  __GP_REGISTER8 (IMR2_W5200,        0x0036);    // Socket Interrupt Mask
  __GP_REGISTER8 (VERSIONR_W5500,    0x0039);    // Chip version
#undef __GP_REGISTER8
#undef __GP_REGISTER16
#undef __GP_REGISTER_N

  // W5x00 Socket registers
  // ----------------------
private:
  static inline uint8_t readSn(SOCKET _s, uint16_t _addr);
  static inline uint8_t writeSn(SOCKET _s, uint16_t _addr, uint8_t _data);
  static inline uint16_t readSn(SOCKET _s, uint16_t _addr, uint8_t *_buf, uint16_t len);
  static inline uint16_t writeSn(SOCKET _s, uint16_t _addr, uint8_t *_buf, uint16_t len);

  static uint16_t CH_BASE;
  static const uint16_t CH_SIZE = 0x0100;

#define __SOCKET_REGISTER8(name, address)                    \
  static inline void write##name(SOCKET _s, uint8_t _data) { \
    writeSn(_s, address, _data);                             \
  }                                                          \
  static inline uint8_t read##name(SOCKET _s) {              \
    return readSn(_s, address);                              \
  }
#define __SOCKET_REGISTER16(name, address)                   \
  static void write##name(SOCKET _s, uint16_t _data) {       \
    writeSn(_s, address,   _data >> 8);                      \
    writeSn(_s, address+1, _data & 0xFF);                    \
  }                                                          \
  static uint16_t read##name(SOCKET _s) {                    \
    uint16_t res = readSn(_s, address);                      \
    uint16_t res2 = readSn(_s,address + 1);                  \
    res = res << 8;                                          \
    res2 = res2 & 0xFF;                                      \
    res = res | res2;                                        \
    return res;                                              \
  }
#define __SOCKET_REGISTER_N(name, address, size)             \
  static uint16_t write##name(SOCKET _s, uint8_t *_buff) {   \
    return writeSn(_s, address, _buff, size);                \
  }                                                          \
  static uint16_t read##name(SOCKET _s, uint8_t *_buff) {    \
    return readSn(_s, address, _buff, size);                 \
  }
  
public:
  __SOCKET_REGISTER8(SnMR,                0x0000)    // Mode
  __SOCKET_REGISTER8(SnCR,                0x0001)    // Command
  __SOCKET_REGISTER8(SnIR,                0x0002)    // Interrupt
  __SOCKET_REGISTER8(SnSR,                0x0003)    // Status
  __SOCKET_REGISTER16(SnPORT,             0x0004)    // Source Port
  __SOCKET_REGISTER_N(SnDHAR,             0x0006, 6) // Destination Hardw Addr
  __SOCKET_REGISTER_N(SnDIPR,             0x000C, 4) // Destination IP Addr
  __SOCKET_REGISTER16(SnDPORT,            0x0010)    // Destination Port
  __SOCKET_REGISTER16(SnMSSR,             0x0012)    // Max Segment Size
  __SOCKET_REGISTER8(SnPROTO,             0x0014)    // Protocol in IP RAW Mode
  __SOCKET_REGISTER8(SnTOS,               0x0015)    // IP TOS
  __SOCKET_REGISTER8(SnTTL,               0x0016)    // IP TTL
  __SOCKET_REGISTER8(SnRXBUF_SIZE,        0x001E)    // RX Buffer Size
  __SOCKET_REGISTER8(SnTXBUF_SIZE,        0x001F)    // TX Buffer Size
  __SOCKET_REGISTER16(SnTX_FSR,           0x0020)    // TX Free Size
  __SOCKET_REGISTER16(SnTX_RD,            0x0022)    // TX Read Pointer
  __SOCKET_REGISTER16(SnTX_WR,            0x0024)    // TX Write Pointer
  __SOCKET_REGISTER16(SnRX_RSR,           0x0026)    // RX Free Size
  __SOCKET_REGISTER16(SnRX_RD,            0x0028)    // RX Read Pointer
  __SOCKET_REGISTER16(SnRX_WR,            0x002A)    // RX Write Pointer (supported?)
  __SOCKET_REGISTER8 (SnIMR_W5200_W5500,  0x002C)    // Interrupt Mask
  __SOCKET_REGISTER16(SnFRAG_W5200_W5500, 0x002D)    // Fragment Offset in IP header
  __SOCKET_REGISTER8 (SnKPALVTR_W5500,    0x002F)    // Keep alive timer
#undef __SOCKET_REGISTER8
#undef __SOCKET_REGISTER16
#undef __SOCKET_REGISTER_N


private:
  static const uint8_t  RST = 7; // Reset BIT

  static const uint16_t SMASK = 0x07FF; // Tx buffer MASK
  static const uint16_t RMASK = 0x07FF; // Rx buffer MASK
public:
  static const uint16_t SSIZE = 2048; // Max Tx buffer size
private:
  static const uint16_t RSIZE = 2048; // Max Rx buffer size
  uint16_t SBASE[8]; // Tx buffer base address
  uint16_t RBASE[8]; // Rx buffer base address

public:
  static void setSSPin(uint8_t pin) { ss_pin = pin; }
private:
  static uint8_t ss_pin;

  // W5100 supports up to 14Mhz
  #define SPI_ETHERNET_SETTINGS SPISettings(14000000, MSBFIRST, SPI_MODE0)
  #if defined(ARDUINO_ARCH_AVR)
    inline static void initSS()  { pinMode(ss_pin, OUTPUT); }
    inline static void setSS()   { *portOutputRegister(digitalPinToPort(ss_pin)) &= ~digitalPinToBitMask(ss_pin); }
    inline static void resetSS() { *portOutputRegister(digitalPinToPort(ss_pin)) |=  digitalPinToBitMask(ss_pin); }
  #else
    inline static void initSS()  { pinMode(ss_pin, OUTPUT);    }
    inline static void setSS()   { digitalWrite(ss_pin, LOW);  }
    inline static void resetSS() { digitalWrite(ss_pin, HIGH); }
  #endif
};

extern W5x00Class W5100;

uint8_t W5x00Class::readSn(SOCKET _s, uint16_t _addr) {
  if (chipset != W5x00Chipset::W5500)
    return read(CH_BASE + _s * CH_SIZE + _addr, 0x00);
  else
    return read(_addr, (_s<<5) + 0x08);
}

uint8_t W5x00Class::writeSn(SOCKET _s, uint16_t _addr, uint8_t _data) {
  if (chipset != W5x00Chipset::W5500)
    return write(CH_BASE + _s * CH_SIZE + _addr, 0x00, _data);
  else
    return write(_addr, (_s<<5) + 0x0C, _data);
}

uint16_t W5x00Class::readSn(SOCKET _s, uint16_t _addr, uint8_t *_buf, uint16_t _len) {
  if (chipset != W5x00Chipset::W5500)
    return read(CH_BASE + _s * CH_SIZE + _addr, 0x00, _buf, _len);
  else
    return read(_addr, (_s<<5) + 0x08, _buf, _len);
}

uint16_t W5x00Class::writeSn(SOCKET _s, uint16_t _addr, uint8_t *_buf, uint16_t _len) {
  if (chipset != W5x00Chipset::W5500)
    return write(CH_BASE + _s * CH_SIZE + _addr, 0x00, _buf, _len);
  else
    return write(_addr, (_s<<5) + 0x0C, _buf, _len);
}

void W5x00Class::setRetransmissionTime(uint16_t _timeout) {
  if (chipset != W5x00Chipset::W5500)
    writeRTR_W5100_W5200(_timeout);
  else
    writeRTR_W5500(_timeout);
}

void W5x00Class::setRetransmissionCount(uint8_t _retry) {
  if (chipset != W5x00Chipset::W5500)
    writeRCR_W5100_W5200(_retry);
  else
    writeRCR_W5500(_retry);
}

W5x00Linkstatus W5x00Class::getLinkStatus() {
  switch (chipset) {
    case W5x00Chipset::W5500:
      return (readPHYCFGR_W5500() & 0x01) ? LINK_ON : LINK_OFF;
    case W5x00Chipset::W5200:
      return (readPSTATUS_W5200() & 0x20) ? LINK_ON : LINK_OFF;
    default:
      return UNKNOWN;
  }
}
#endif
