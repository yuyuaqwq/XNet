#include "udp.h"

#include "network/ip.h"
#include "transport/icmp.h"

void XUdpIn(XIpv4Addr* srcIp, XNetPacket* packet) {
	auto udpHeader = (XUdpHeader*)packet->data;
	if (packet->size < sizeof(XUdpHeader) || packet->size < TO_NETWORK_BYTE_ORDER16(udpHeader->totalLen)) {
		return;
	}

	uint16_t oldChecksum = udpHeader->checksum;
	udpHeader->checksum = 0;
	if (oldChecksum != 0) {
		uint16_t checksum = ChecksumPeso(srcIp, &gLocalIp, XTransportProtocol::kUdp, (uint16_t*)udpHeader, TO_NETWORK_BYTE_ORDER16(udpHeader->totalLen));
		checksum = (checksum == 0) ? 0xffff : checksum;
		if (checksum != oldChecksum) {
			return;
		}
	}

	RemoveHeader(packet, sizeof(XUdpHeader));

	// 找不到UDP控制块的时候使用ICMP响应端口不可达，这里为了省事就不响应了
	// XIcmpDestUnreach(XIcmpCode::kPortUnreachable, );
}

XNetStatus XUdpOut(XIpv4Addr* destIp, uint16_t srcPort, uint16_t destPort, XNetPacket* packet) {
	AddHeader(packet, sizeof(XUdpHeader));
	auto udpHeader = (XUdpHeader*)packet->data;
	udpHeader->srcPort = TO_NETWORK_BYTE_ORDER16(srcPort);
	udpHeader->destPort = TO_NETWORK_BYTE_ORDER16(destPort);
	udpHeader->totalLen = TO_NETWORK_BYTE_ORDER16(packet->size);
	udpHeader->checksum = 0;
	udpHeader->checksum = ChecksumPeso(&gLocalIp, destIp, XTransportProtocol::kUdp, (uint16_t*)packet->data, packet->size);
	return XIpOut(XTransportProtocol::kUdp, destIp, packet);
}