#ifndef NETWORK_ARP_H_
#define NETWORK_ARP_H_

#include "XNet.h"
#include "datalink/ethernet.h"
#include "network/ip.h"
#include <CUtils/container/list.h>

#define XARP_ENTRY_FREE 0
#define XARP_CFG_ENTRY_OK_TMO 5		// 5s
#define XARP_CFG_ENTRY_PENDING_TMO 1		// 1s
#define XARP_CFG_MAX_RETRIES 4
#define XARP_TIMER_PERIOD 1		// 1s


enum class XArpEntryStatus : uint8_t {
	kOk,
	kPending,
};

struct XArpEntry {
	XIpv4Addr ip;
	uint8_t mac[XNET_MAC_ADDR_SIZE];
	XArpEntryStatus state;
	uint8_t tmo;
	uint8_t retryCnt;
	ListEntry listEntry;
};


#define XARP_HW_ETHER TO_NETWORK_BYTE_ORDER16(0x1)

enum class XArpOpcode : uint16_t {
	kRequest = TO_NETWORK_BYTE_ORDER16(0x1),
	kReply = TO_NETWORK_BYTE_ORDER16(0x2),
};

#pragma pack(1)
struct XArpPacket {
	uint16_t hardwareType;
	XNetworkProtocol protocolType;
	uint8_t hardwareSize;
	uint8_t protocolSize;
	XArpOpcode opcode;
	uint8_t senderMac[XNET_MAC_ADDR_SIZE];
	XIpv4Addr senderIp;
	uint8_t targetMac[XNET_MAC_ADDR_SIZE];
	XIpv4Addr targetIp;
};
#pragma pack()

void XArpInit();
void XArpIn(XNetPacket* packet);
void XArpPoll();
void XArpUpdateEntry(const uint8_t* mac, const XIpv4Addr* ip);
XNetStatus XArpResolve(const XIpv4Addr* ip, uint8_t* outMac);
XNetStatus XArpMakeRequest(const XIpv4Addr* targetIp);

#endif // NETWORK_ARP_H_