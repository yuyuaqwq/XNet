#ifndef TRANSPORT_TCP_H_
#define TRANSPORT_TCP_H_

#include <stdint.h>

#include "XNet.h"
#include "network/ip.h"

enum class XTcpFlags : uint16_t {
	kFin_1 = (1 << 0),		// finished
	kSyn_1 = (1 << 1),		// sync
	kRst_1 = (1 << 2),		// reset
	kPsh_1 = (1 << 3),		// push
	kAck_1 = (1 << 4),		// Acknowledgment
	kUrg_1 = (1 << 5),		// urgent
	kEce_1 = (1 << 6),		// Explicit Congestion Notification
	KCwr_1 = (1 << 7),		// Congestion Window Reduced
	KEcn_1 = (1 << 8),		// Accurate ECN
};

#pragma pack(1)
struct XTcpHeader {
	uint16_t srcPort;
	uint16_t destPort;
	uint32_t seqNum;
	uint32_t ackNum;
	union {
		struct {
			XTcpFlags vaild : 6;
			uint16_t reserved : 6;
			uint16_t headerLen : 4;
		};
		uint16_t all;
	} flags;

	uint16_t window;
	uint16_t checksum;
	uint16_t urgentPtr;
};
#pragma pack()

enum class XTcpStatus {
	kFree,
	kClosed,
	kListen,
	kSynRecvd,
	kEstablished,
};

void XTcpIn(XIpv4Addr* remoteIp, XNetPacket* packet);


#endif // TRANSPORT_TCP_H_