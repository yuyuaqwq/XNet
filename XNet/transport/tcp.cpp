#include "tcp.h"

#include <stdlib.h>

#include "network/ip.h"


XTcpFlags operator|(XTcpFlags a, XTcpFlags b) {
	return static_cast<XTcpFlags>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}

static XNetStatus XTcpSendReset(uint32_t remoteAckNum, uint16_t localPort, XIpv4Addr* remoteIp, uint16_t remotePort) {
	auto packet = XNetAllocForSend(sizeof(XTcpHeader));
	auto tcpHeader = (XTcpHeader*)packet->data;

	tcpHeader->srcPort = TO_NETWORK_BYTE_ORDER16(localPort);
	tcpHeader->destPort = TO_NETWORK_BYTE_ORDER16(remotePort);
	tcpHeader->seqNum = 0;
	tcpHeader->ackNum = TO_NETWORK_BYTE_ORDER32(remoteAckNum);
	tcpHeader->flags.all = 0;
	tcpHeader->flags.headerLen = sizeof(XTcpHeader) / 4;
	tcpHeader->flags.vaild = XTcpFlags::kRst_1 | XTcpFlags::kAck_1;
	tcpHeader->flags.all = TO_NETWORK_BYTE_ORDER16(tcpHeader->flags.all);
	tcpHeader->window = 0;
	tcpHeader->urgentPtr = 0;
	tcpHeader->checksum = 0;
	tcpHeader->checksum = ChecksumPeso(&gLocalIp, remoteIp, XTransportProtocol::kTcp, (uint16_t*)packet->data, packet->size);
	tcpHeader->checksum = (tcpHeader->checksum == 0) ? 0xffff : tcpHeader->checksum;
	return XIpOut(XTransportProtocol::kTcp, remoteIp, packet);
}

void XTcpInit() {
	srand(XSysGetTime());
}

void XTcpIn(XIpv4Addr* remoteIp, XNetPacket* packet) {
	auto tcpHeader = (XTcpHeader*)packet->data;

	if (packet->size < sizeof(XTcpHeader)) {
		return;
	}

	uint16_t oldChecksum = tcpHeader->checksum;
	tcpHeader->checksum = 0;

	if(oldChecksum != 0) {
		uint16_t checksum = ChecksumPeso(remoteIp, &gLocalIp, XTransportProtocol::kTcp, (uint16_t*)tcpHeader, packet->size);
		checksum = (checksum == 0) ? 0xffff : checksum;
		if (checksum != oldChecksum) {
			return;
		}
	}

	tcpHeader->srcPort = TO_NETWORK_BYTE_ORDER16(tcpHeader->srcPort);
	tcpHeader->destPort = TO_NETWORK_BYTE_ORDER16(tcpHeader->destPort);
	tcpHeader->flags.all = TO_NETWORK_BYTE_ORDER16(tcpHeader->flags.all);
	tcpHeader->seqNum = TO_NETWORK_BYTE_ORDER32(tcpHeader->seqNum);
	tcpHeader->ackNum = TO_NETWORK_BYTE_ORDER32(tcpHeader->ackNum);
	tcpHeader->window = TO_NETWORK_BYTE_ORDER16(tcpHeader->window);

	// 找不到tcp控制块的时候响应无法处理该TCP数据报
	XTcpSendReset(tcpHeader->seqNum + 1, tcpHeader->destPort, remoteIp, tcpHeader->srcPort);
}
