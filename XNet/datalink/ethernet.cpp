#include "ethernet.h"

#include <string.h>

#include "driver/NetDriver.h"
#include "network/arp.h"


uint8_t gLocalMac[XNET_MAC_ADDR_SIZE];
const uint8_t kEthernetBroadcast[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

XNetStatus EthernetInit(const uint8_t* localMac) {
	memcpy(gLocalMac, localMac, XNET_MAC_ADDR_SIZE);
	return XNetStatus::kOk;
}

XNetStatus EthernetOut(XNetworkProtocol protocol, const uint8_t* targetMacAddr, XNetPacket* packet) {
	AddHeader(packet, sizeof(XEthernetHedaer));
	auto etherHeader = (XEthernetHedaer*)packet->data;
	memcpy(etherHeader->destMac, targetMacAddr, XNET_MAC_ADDR_SIZE);
	memcpy(etherHeader->srcMac, gLocalMac, XNET_MAC_ADDR_SIZE);
	etherHeader->protocol = protocol;

	return XNetDriverSend(packet);
}

XNetStatus EthernetIn(XNetPacket* packet) {
	if (packet->size <= sizeof(XEthernetHedaer)) {
		return XNetStatus::kOk;
	}

	auto etherHeader = (XEthernetHedaer*)packet->data;

	switch (etherHeader->protocol) {
	case XNetworkProtocol::kArp: {
		RemoveHeader(packet, sizeof(XEthernetHedaer));
		XArpIn(packet);
		break;
	}
	case XNetworkProtocol::kIp: {
		RemoveHeader(packet, sizeof(XEthernetHedaer));
		XIpIn(etherHeader->destMac, packet);
		break;
	}
	}

	return XNetStatus::kOk;
}

void EthernetPoll() {
	XNetPacket* packet;
	if (XNetDriverRead(&packet) == XNetStatus::kOk) {
		EthernetIn(packet);
	}
}

