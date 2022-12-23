#include <stdint.h>
#include <string.h>

#include "xnet/pcap_device.h"
#include "XNet.h"

#include "datalink/ethernet.h"

static pcap_t* gsPCap;

XNetStatus XNetDriverOpen(const char* networkCardIpStr, const uint8_t* macAddr) {
	gsPCap = pcap_device_open(networkCardIpStr, macAddr, 1);
	if (gsPCap == nullptr) {
		exit(-1);
	}
	return XNetStatus::kOk;
}

XNetStatus XNetDriverSend(XNetPacket* packet) {
	return (XNetStatus)pcap_device_send(gsPCap, packet->data, packet->size);
}

XNetStatus XNetDriverRead(XNetPacket** packet) {
	uint16_t size;
	XNetPacket* readPacket = XNetAllocForRead(XNET_CFG_PACKET_MAX_SIZE);
	size = pcap_device_read(gsPCap, readPacket->data, XNET_CFG_PACKET_MAX_SIZE);
	if (size) {
		readPacket->size = size;
		*packet = readPacket;
		return XNetStatus::kOk;
	}
	return XNetStatus::kError_IO;
}