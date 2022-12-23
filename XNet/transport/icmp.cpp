#include "icmp.h"

#include <string.h>

void XIcmpInit() {

}

static XNetStatus XIcmpReplyRequest(XIcmpHeader* icmpHeader, XIpv4Addr* srcIp, XNetPacket* packet) {
	XNetPacket* replyPacket = XNetAllocForSend(packet->size);
	XIcmpHeader* replyHeader = (XIcmpHeader*)replyPacket->data;

	replyHeader->type = XIcmpType::kReply;
	replyHeader->code = icmpHeader->code;
	replyHeader->id = icmpHeader->id;
	replyHeader->seqNum = icmpHeader->seqNum;
	replyHeader->checksum = 0;
	memcpy((uint8_t*)replyHeader + sizeof(XIcmpHeader), (uint8_t*)icmpHeader + sizeof(XIcmpHeader), packet->size - sizeof(XIcmpHeader));
	replyHeader->checksum = Checksum16((uint16_t*)replyHeader, replyPacket->size, 0, true);
	return XIpOut(XTransportProtocol::kIcmp, srcIp, replyPacket);
}

XNetStatus XIcmpDestUnreach(XIcmpCode code, XIpv4Header* destIp) {
	auto ipHeaderSize = destIp->headerSize * 4;
	auto ipDataSize = TO_NETWORK_BYTE_ORDER16(destIp->totalSize) - ipHeaderSize;
	ipDataSize = ipHeaderSize + min(ipDataSize, 8);

	XNetPacket* packet = XNetAllocForSend(sizeof(XIcmpHeader) + ipDataSize);
	XIcmpHeader* icmpHeader = (XIcmpHeader*)packet->data;

	icmpHeader->type = XIcmpType::kUnreachable;
	icmpHeader->code = code;
	icmpHeader->id = 0;
	icmpHeader->seqNum = 0;
	memcpy((uint8_t*)icmpHeader + sizeof(XIcmpHeader), destIp, ipDataSize);
	icmpHeader->checksum = 0;
	icmpHeader->checksum = Checksum16((uint16_t*)icmpHeader, packet->size, 0, true);
	return XIpOut(XTransportProtocol::kIcmp, &destIp->srcIp, packet);
}

void XIcmpIn(XIpv4Addr* srcIp, XNetPacket* packet) {
	auto icmpHeader = (XIcmpHeader*)packet->data;

	if (packet->size < sizeof(XIcmpHeader)) {
		return;
	}

	if (icmpHeader->type == XIcmpType::kRequest) {
		XIcmpReplyRequest(icmpHeader, srcIp, packet);
	}

}