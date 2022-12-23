#ifndef DRIVER_NET_H_
#define DRIVER_NET_H_

#include <stdint.h>

#include "XNet.h"

XNetStatus XNetDriverOpen(const char* networkCardIpStr, const uint8_t* macAddr);
XNetStatus XNetDriverSend(XNetPacket* packet);
XNetStatus XNetDriverRead(XNetPacket** packet);

#endif // DRIVER_NET_H_