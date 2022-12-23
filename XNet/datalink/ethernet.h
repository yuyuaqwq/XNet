#ifndef DATALINK_ETHERNET_H_
#define DATALINK_ETHERNET_H_

#include "XNet.h"

#define XNET_MAC_ADDR_SIZE 6

enum class XNetworkProtocol : uint16_t {
	kIp = TO_NETWORK_BYTE_ORDER16(0x0800),
	kArp = TO_NETWORK_BYTE_ORDER16(0x0806),
};

#pragma pack(1)
struct XEthernetHedaer {
	uint8_t destMac[XNET_MAC_ADDR_SIZE];
	uint8_t srcMac[XNET_MAC_ADDR_SIZE];
	XNetworkProtocol protocol;
};
#pragma pack()

XNetStatus EthernetInit(const uint8_t* localMac);
void EthernetPoll();
XNetStatus EthernetOut(XNetworkProtocol protocol, const uint8_t* targetMacAddr, XNetPacket* packet);
XNetStatus EthernetIn(XNetPacket* packet);

extern uint8_t gLocalMac[XNET_MAC_ADDR_SIZE];
extern const uint8_t kEthernetBroadcast[];

#endif // DATALINK_ETHERNET_H_