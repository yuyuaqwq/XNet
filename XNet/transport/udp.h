#ifndef TRANSPORT_UDP_H_
#define TRANSPORT_UDP_H_

#include <stdint.h>

#include "XNet.h"
#include "network/ip.h"
#include "CUtils/container/AVLTree.h"

#pragma pack(1)
struct XUdpHeader {
	uint16_t srcPort;
	uint16_t destPort;
	uint16_t totalLen;
	uint16_t checksum;
};
#pragma pack()


typedef void (*XUdpCallback)(XIpv4Addr* srcIp, XNetPacket* packet);
struct XUdp {
	AVLEntry* entry;
	uint16_t port;
};

XUdp* XUdpOpen();


void XUdpIn(XIpv4Addr* srcIp, XNetPacket* packet);
XNetStatus XUdpOut(XIpv4Addr* destIp, uint16_t srcPort, uint16_t destPort, XNetPacket* packet);

#endif // TRANSPORT_UDP_H_