#ifndef XNET_TINY_H_
#define XNET_TINY_H_

#include <stdint.h>
#include <time.h>

#define XNET_CFG_PACKET_MAX_SIZE 1516

#define TO_NETWORK_BYTE_ORDER16(v) ( (((uint16_t)v) & 0xff) << 8 | (((uint16_t)v) >> 8) & 0xff )
#define TO_NETWORK_BYTE_ORDER32(v) ( (((uint32_t)v) & 0xff) << 24 | (((uint32_t)v) & 0xff00) << 8 | (((uint32_t)v) & 0xff0000) >> 8 | (((uint32_t)v) & 0xff000000) >> 24 )
#define min(a, b) ((a) > (b) ? (b) : (a))


enum class XNetStatus {
	kOk = 0,
	kError_IO = -1,
	kError_None = -2,
};
#define XNetError(status) (((int32_t)(status)) < 0)

struct XNetPacket {
	uint16_t size;
	uint8_t* data;
	uint8_t payload[XNET_CFG_PACKET_MAX_SIZE];		// Êý¾ÝÇøÓò
};

typedef uint32_t XNetTime;

XNetPacket* XNetAllocForSend(uint16_t dataSize);
XNetPacket* XNetAllocForRead(uint16_t dataSize);

void AddHeader(XNetPacket* packet, uint16_t headerSize);
void RemoveHeader(XNetPacket* packet, uint16_t headerSize);

XNetTime XSysGetTime();
bool XNetCheckTmo(XNetTime* time, uint32_t ms);

uint16_t Checksum16(uint16_t* buf, size_t size, uint16_t preSum, bool complement);
union XIpv4Addr; 
enum class XTransportProtocol : uint8_t;
uint16_t ChecksumPeso(const XIpv4Addr* srcIp, const XIpv4Addr* destIp, XTransportProtocol protocol, uint16_t* buf, uint16_t len);

void XNetInit();
void XNetPoll();

#endif // XNET_TINY_H_
