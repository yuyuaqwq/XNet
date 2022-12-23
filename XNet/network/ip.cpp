#include "ip.h"

#include <string.h>

#include "datalink/ethernet.h"
#include "network/arp.h"
#include "transport/icmp.h"
#include "transport/udp.h"
#include "transport/tcp.h"

XIpv4Addr gLocalIp;

bool XIpAddrEqual(const XIpv4Addr* ip1, const XIpv4Addr* ip2) {
	return ip1->addr == ip2->addr;
}

void XIpInit(const XIpv4Addr* localIp) {
	memcpy(&gLocalIp, localIp, XNET_IPV4_ADDR_SIZE);
}

void XIpIn(uint8_t* destMac, XNetPacket* packet) {
	auto ipHeader = (XIpv4Header*)packet->data;

	if (ipHeader->version != XNET_IP_VERSION_IPV4) {
		return;
	}

	auto headerSize = ipHeader->headerSize * 4;
	auto totalSize = TO_NETWORK_BYTE_ORDER16(ipHeader->totalSize);
	if (headerSize < sizeof(XIpv4Header) || totalSize < headerSize) {
		return;
	}

	// 置0校验和后再计算
	auto oldChecksum = ipHeader->headerChecksum;
	ipHeader->headerChecksum = 0;
	if (oldChecksum != Checksum16((uint16_t*)ipHeader, headerSize, 0, true)) {
		return;
	}

	if (!XIpAddrEqual(&gLocalIp, &ipHeader->destIp)) {
		return;
	}

	// 更新一下arp表
	XArpUpdateEntry(destMac, &ipHeader->destIp);

	switch (ipHeader->protocol) {
	case XTransportProtocol::kIcmp: {
		RemoveHeader(packet, headerSize);
		XIcmpIn(&ipHeader->srcIp, packet);
		break;
	case XTransportProtocol::kUdp: {
		RemoveHeader(packet, headerSize);
		XUdpIn(&ipHeader->srcIp, packet);
		break;
	}
	case XTransportProtocol::kTcp: {
		RemoveHeader(packet, headerSize);
		XTcpIn(&ipHeader->srcIp, packet);
		break;
	}
	default:
		XIcmpDestUnreach(XIcmpCode::kProtocolUnreachable, ipHeader);
		break;
	}
	}
}

#define XNET_IP_DEFAULT_TTL 64
XNetStatus XIpOut(XTransportProtocol protocol, XIpv4Addr* destIp, XNetPacket* packet) {
	static uint32_t ipPacketId = 0;
	AddHeader(packet, sizeof(XIpv4Header));
	auto ipHeader = (XIpv4Header*)packet->data;
	ipHeader->version = XNET_IP_VERSION_IPV4;
	ipHeader->headerSize = sizeof(XIpv4Header) / 4;
	ipHeader->tos = 0;
	ipHeader->totalSize = TO_NETWORK_BYTE_ORDER16(packet->size);
	ipHeader->id = TO_NETWORK_BYTE_ORDER16(ipPacketId);
	ipPacketId++;
	ipHeader->flagsFragment = 0;
	ipHeader->ttl = XNET_IP_DEFAULT_TTL;
	ipHeader->protocol = protocol;
	ipHeader->srcIp = gLocalIp;
	ipHeader->destIp = *destIp;
	ipHeader->headerChecksum = 0;
	ipHeader->headerChecksum = Checksum16((uint16_t*)ipHeader, sizeof(XIpv4Header), 0, true);
	
	uint8_t mac[XNET_MAC_ADDR_SIZE];
	auto status = XArpResolve(destIp, mac);
	if (XNetError(status)) {
		return status;
	}
	return EthernetOut(XNetworkProtocol::kIp, mac, packet);
}