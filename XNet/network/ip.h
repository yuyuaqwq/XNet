#ifndef NETWORK_IP_H_
#define NETWORK_IP_H_

#include "XNet.h"

#define XNET_IPV4_ADDR_SIZE 4

union XIpv4Addr {
	uint8_t array[XNET_IPV4_ADDR_SIZE];
	uint32_t addr;
};

#define XNET_IP_VERSION_IPV4 4

enum class XTransportProtocol : uint8_t {
	kIcmp = 1,
	kUdp = 17,
	kTcp = 6,
};

#pragma pack(1)
struct XIpv4Header {
	uint8_t headerSize : 4;
	uint8_t version : 4;
	uint8_t tos;
	uint16_t totalSize;
	uint16_t id;
	uint16_t flagsFragment;
	uint8_t ttl;
	XTransportProtocol protocol;
	uint16_t headerChecksum;
	XIpv4Addr srcIp;
	XIpv4Addr destIp;
};
#pragma pack()

bool XIpAddrEqual(const XIpv4Addr* ip1, const XIpv4Addr* ip2);

void XIpInit(const XIpv4Addr* localIp);
void XIpIn(uint8_t* destMac, XNetPacket* packet);
XNetStatus XIpOut(XTransportProtocol protocol, XIpv4Addr* destIp, XNetPacket* packet);

extern XIpv4Addr gLocalIp;

#endif // NETWORK_IP_H_