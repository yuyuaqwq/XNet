#include "XNet.h"

#include <string.h>

#include "driver/NetDriver.h"
#include "datalink/ethernet.h"
#include "network/ip.h"
#include "network/arp.h"

static XNetPacket gsSendPacket, gsReadPacket;
XNetPacket* XNetAllocForSend(uint16_t dataSize) {
	// 从后向前使用缓冲区
	gsSendPacket.data = gsSendPacket.payload + XNET_CFG_PACKET_MAX_SIZE - dataSize;
	gsSendPacket.size = dataSize;
	return &gsSendPacket;
}

XNetPacket* XNetAllocForRead(uint16_t dataSize) {
	gsReadPacket.data = gsReadPacket.payload;
	gsReadPacket.size = dataSize;
	return &gsReadPacket;
}


void AddHeader(XNetPacket* packet, uint16_t headerSize) {
	packet->data -= headerSize;
	packet->size += headerSize;
}

void RemoveHeader(XNetPacket* packet, uint16_t headerSize) {
	packet->data += headerSize;
	packet->size -= headerSize;
}

static void TruncatePacket(XNetPacket* packet, uint16_t size) {
	packet->size = min(packet->size, size);
}


XNetTime XSysGetTime(void) {
	return clock() / CLOCKS_PER_SEC;
}

bool XNetCheckTmo(XNetTime* time, uint32_t sec) {
	XNetTime cur = XSysGetTime();
	if (sec == 0) {
		*time = cur;
		return false;
	}
	else if (cur - *time >= sec) {
		*time = cur;
		return true;
	}
	return false;
}

uint16_t Checksum16(uint16_t* buf, size_t size, uint16_t preSum, bool complement) {
	uint32_t checksum = preSum;
	while (size > 1) {
		checksum += *buf++;
		size -= 2;
	}
	if (size > 0) {
		checksum += *(uint8_t*)buf;
	}

	uint16_t high;
	while ((high = checksum >> 16) != 0) {
		checksum = high + (checksum & 0xffff);
	}

	return complement ? (uint16_t)~checksum : (uint16_t)checksum;
}

/*
* 计算Udp伪校验和
*/
uint16_t ChecksumPeso(const XIpv4Addr* srcIp, const XIpv4Addr* destIp, XTransportProtocol protocol, uint16_t* buf, uint16_t len) {
	uint8_t zeroProtocol[] = { 0, (uint8_t)protocol };
	uint32_t chesksum = Checksum16((uint16_t*)srcIp, XNET_IPV4_ADDR_SIZE, 0, false);
	chesksum = Checksum16((uint16_t*)destIp, XNET_IPV4_ADDR_SIZE, chesksum, false);
	chesksum = Checksum16((uint16_t*)zeroProtocol, sizeof(zeroProtocol), chesksum, false);
	uint16_t cLen = TO_NETWORK_BYTE_ORDER16(len);
	chesksum = Checksum16((uint16_t*)&cLen, sizeof(cLen), chesksum, false);
	return Checksum16(buf, len, chesksum, true);
}



const uint8_t mac[] =  { 0x11,0x22,0x33,0x44,0x55,0x66 };
const char* ip = "192.168.25.1";
#define XNET_CFG_NETIF_IP {192,168,25,4}
static const XIpv4Addr localIp = XNET_CFG_NETIF_IP;

void XNetInit() {
	XNetDriverOpen(ip, mac);
	EthernetInit(mac);
	XIpInit(&localIp);
	XArpInit();
	XArpMakeRequest(&localIp);
}

void XNetPoll() {
	EthernetPoll();
	XArpPoll();
}