#ifndef TRANSPORT_ICMP_H_
#define TRANSPORT_ICMP_H_

#include <stdint.h>

#include "XNet.h"
#include "network/ip.h"

enum class XIcmpType : uint8_t {
	kRequest = 8,
	kReply = 0,
	kUnreachable = 3,
};

enum class XIcmpCode : uint8_t {
	kOk = 0,
	kNetUnreachable = 0,
	kHostUnreachable = 1,
	kProtocolUnreachable = 2,
	kPortUnreachable = 3,

};

#pragma pack(1)
struct XIcmpHeader {
	XIcmpType type;
	XIcmpCode code;
	uint16_t checksum;
	uint16_t id;
	uint16_t seqNum;
};
#pragma pack()

void XIcmpInit();
void XIcmpIn(XIpv4Addr* srcIp, XNetPacket* packet);
XNetStatus XIcmpDestUnreach(XIcmpCode code, XIpv4Header* destIp);

#endif // TRANSPORT_ICMP_H_