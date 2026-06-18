#ifndef TCP_STATES_H
#define TCP_STATES_H

#include "w5100.h"

// common constants for client.state() return values
enum tcp_state {
  CLOSED      = SnSR::CLOSED,
  LISTEN      = SnSR::LISTEN,
  SYN_SENT    = SnSR::SYNSENT,
  SYN_RCVD    = SnSR::SYNRECV,
  ESTABLISHED = SnSR::ESTABLISHED,
  FIN_WAIT_1  = SnSR::FIN_WAIT,
  FIN_WAIT_2  = SnSR::FIN_WAIT,
  CLOSE_WAIT  = SnSR::CLOSE_WAIT,
  CLOSING     = SnSR::CLOSING,
  LAST_ACK    = SnSR::LAST_ACK,
  TIME_WAIT   = SnSR::TIME_WAIT
};

#endif
