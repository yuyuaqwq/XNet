#include "arp.h"

#include <string.h>


ListEntry gArpTableHead;
static XNetTime gsArpTimer;

XNetStatus XArpMakeRequest(const XIpv4Addr* targetIp) {
	auto packet = XNetAllocForSend(sizeof(XArpPacket));
	auto arpPacket = (XArpPacket*)packet->data;

	arpPacket->hardwareType = XARP_HW_ETHER;
	arpPacket->protocolType = XNetworkProtocol::kIp;
	arpPacket->hardwareSize = XNET_MAC_ADDR_SIZE;
	arpPacket->protocolSize = XNET_IPV4_ADDR_SIZE;
	arpPacket->opcode = XArpOpcode::kRequest;
	memcpy(arpPacket->senderMac, gLocalMac, XNET_MAC_ADDR_SIZE);
	memcpy(arpPacket->senderIp.array, gLocalIp.array, XNET_IPV4_ADDR_SIZE);
	memset(arpPacket->targetMac, 0, XNET_MAC_ADDR_SIZE);
	memcpy(arpPacket->targetIp.array, targetIp->array, XNET_IPV4_ADDR_SIZE);
	return EthernetOut(XNetworkProtocol::kArp, kEthernetBroadcast, packet);
}

XNetStatus XArpMakeResponse(const uint8_t* targetMac, const XIpv4Addr* targetIp) {
	auto packet = XNetAllocForSend(sizeof(XArpPacket));
	auto arpPacket = (XArpPacket*)packet->data;

	arpPacket->hardwareType = XARP_HW_ETHER;
	arpPacket->protocolType = XNetworkProtocol::kIp;
	arpPacket->hardwareSize = XNET_MAC_ADDR_SIZE;
	arpPacket->protocolSize = XNET_IPV4_ADDR_SIZE;
	arpPacket->opcode = XArpOpcode::kReply;
	memcpy(arpPacket->senderMac, gLocalMac, XNET_MAC_ADDR_SIZE);
	memcpy(arpPacket->senderIp.array, gLocalIp.array, XNET_IPV4_ADDR_SIZE);
	memcpy(arpPacket->targetMac, targetMac, XNET_MAC_ADDR_SIZE);
	memcpy(arpPacket->targetIp.array, targetIp->array, XNET_IPV4_ADDR_SIZE);
	return EthernetOut(XNetworkProtocol::kArp, targetMac, packet);
}

void XArpUpdateEntry(const uint8_t* mac, const XIpv4Addr* ip) {
	XArpEntry* entry;
	ListFindKeyM(&gArpTableHead, entry, ip->addr, XArpEntry, listEntry, ip.addr);
	if (entry == NULL) {
		entry = CreateObject(XArpEntry);
		ListInsertHead(&gArpTableHead, &entry->listEntry);
	}
	entry->ip = *ip;
	memcpy(entry->mac, mac, XNET_MAC_ADDR_SIZE);
	entry->state = XArpEntryStatus::kOk;
	entry->tmo = XARP_CFG_ENTRY_OK_TMO;
	entry->retryCnt = XARP_CFG_MAX_RETRIES;
}

XNetStatus XArpResolve(const XIpv4Addr* targetIp, uint8_t* outMac) {
	XArpEntry* entry;
	ListFindKeyM(&gArpTableHead, entry, targetIp->addr, XArpEntry, listEntry, ip.addr);
	if (entry == NULL) {
		XArpMakeRequest(targetIp);
		return XNetStatus::kError_None;
	}
	memcpy(outMac, entry->mac, XNET_MAC_ADDR_SIZE);
	return XNetStatus::kOk;
}

void XArpInit() {
	ListHeadInit(&gArpTableHead);
	XNetCheckTmo(&gsArpTimer, 0);
}

void XArpIn(XNetPacket* packet) {
	if (packet->size < sizeof(XArpPacket)) {
		return;
	}

	auto arpPacket = (XArpPacket*)packet->data;
	auto opcode = arpPacket->opcode;

	if (arpPacket->hardwareType != XARP_HW_ETHER || arpPacket->hardwareSize != XNET_MAC_ADDR_SIZE
		|| arpPacket->protocolType != XNetworkProtocol::kIp || arpPacket->protocolSize != XNET_IPV4_ADDR_SIZE
		|| opcode != XArpOpcode::kRequest && opcode != XArpOpcode::kReply) {
		return;
	}

	if (!XIpAddrEqual(&gLocalIp, &arpPacket->targetIp)) {
		return;
	}
	
	switch (opcode) {
	case XArpOpcode::kRequest: {
		XArpMakeResponse(arpPacket->senderMac, &arpPacket->senderIp);
		XArpUpdateEntry(arpPacket->senderMac, &arpPacket->senderIp);
		break;
	}
	case XArpOpcode::kReply: {
		XArpUpdateEntry(arpPacket->senderMac, &arpPacket->senderIp);
		break;
	}
	}
}

void XArpPoll() {
	// 每隔一段时间扫描一次arp表
	if (!XNetCheckTmo(&gsArpTimer, XARP_TIMER_PERIOD)) {
		return;
	}

	ListEntry* cur = NULL;// gArpTableHead.next;
	while (ListIteration(&gArpTableHead, &cur)) {//cur != &gArpTableHead) {
		auto obj = GetObjFromField(cur, XArpEntry, listEntry);
		switch (obj->state) {
		case XArpEntryStatus::kOk: {
			// arp是可用的，但可能过期，准备重发以验证
			if (--obj->tmo == 0) {
				XArpMakeRequest(&obj->ip);
				obj->state = XArpEntryStatus::kPending;
				obj->tmo = XARP_CFG_ENTRY_PENDING_TMO;		// 每隔1秒发送一次
			}
			break;
		}
		case XArpEntryStatus::kPending: {
			// 多次重发验证
			if (--obj->tmo == 0) {
				if (obj->retryCnt-- == 0) {
					ListEntry* del = ListRemoveEntry(cur, false);		// 重发次数消耗完毕，删除该arp条目
					cur = cur->prev;
					DeleteObject(GetObjFromField(del, XArpEntry, listEntry));
				}
				else {
					XArpMakeRequest(&obj->ip);		// 再次发送
					obj->state = XArpEntryStatus::kPending;
					obj->tmo = XARP_CFG_ENTRY_PENDING_TMO;
				}
			}
			break;
		}
		}
		// cur = cur->next;
	}
}