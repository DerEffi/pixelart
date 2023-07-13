//
// l2_packet_circle.cpp
//
// Layer2 packet handling interface for Circle
// by R. Stange <rsta2@o2online.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// Alternatively, this software may be distributed under the terms of BSD
// license.
//

extern "C" {

#include "includes.h"
#include "common.h"
#include "eloop.h"
#include "l2_packet.h"

}

#include <circle/net/netsubsystem.h>
#include <circle/net/netdevlayer.h>
#include <circle/net/linklayer.h>
#include <circle/sched/scheduler.h>
#include <assert.h>

#define SOCK_FD		1

struct l2_packet_data
{
	unsigned short protocol;
	void (*rx_callback) (void *ctx, const u8 *src_addr, const u8 *buf, size_t len);
	void *rx_callback_ctx;
	u8 own_addr[ETH_ALEN];
	CLinkLayer *link;
};

static void l2_packet_receive (int sock, void *eloop_ctx, void *sock_ctx);

static int auth_active;

l2_packet_data * l2_packet_init (const char *ifname, const u8 *own_addr, unsigned short protocol,
				 void (*rx_callback) (void *ctx, const u8 *src_addr,
						      const u8 *buf, size_t len),
				 void *rx_callback_ctx, int l2_hdr)
{
	assert (own_addr == 0);
	assert (protocol == 0x888E);
	assert (l2_hdr == 0);

	l2_packet_data *l2 = (l2_packet_data *) os_zalloc (sizeof *l2);
	if (l2 == 0)
	{
		return 0;
	}

	l2->protocol = protocol;
	l2->rx_callback = rx_callback;
	l2->rx_callback_ctx = rx_callback_ctx;

	auth_active = 0;

	const CMACAddress *mac = CNetSubSystem::Get ()->GetNetDeviceLayer ()->GetMACAddress ();
	assert (mac != 0);
	mac->CopyTo (l2->own_addr);

	l2->link = CNetSubSystem::Get ()->GetLinkLayer ();
	assert (l2->link != 0);
	if (!l2->link->EnableReceiveRaw (protocol))
	{
		os_free (l2);

		return 0;
	}

	eloop_register_read_sock (SOCK_FD, l2_packet_receive, l2, 0);

	return l2;
}

void l2_packet_deinit (l2_packet_data *l2)
{
	if (l2 == 0)
	{
		return;
	}

	eloop_unregister_read_sock (SOCK_FD);

	os_free (l2);
}

int l2_packet_get_own_addr (l2_packet_data *l2, u8 *addr)
{
	assert (l2 != 0);
	assert (addr != 0);
	os_memcpy (addr, l2->own_addr, ETH_ALEN);

	return 0;
}

int l2_packet_send (l2_packet_data *l2, const u8 *dst_addr, u16 proto, const u8 *buf, size_t len)
{
	if (l2 == 0)
	{
		return -1;
	}

	u8 Buffer[FRAME_BUFFER_SIZE];
	TEthernetHeader *pHeader = (TEthernetHeader *) Buffer;
	assert (dst_addr != 0);
	os_memcpy (pHeader->MACReceiver, dst_addr, MAC_ADDRESS_SIZE);
	os_memcpy (pHeader->MACSender, l2->own_addr, MAC_ADDRESS_SIZE);
	pHeader->nProtocolType = le2be16 (l2->protocol);

	assert (len > 0);
	if (len + sizeof (TEthernetHeader) > FRAME_BUFFER_SIZE)
	{
		return -1;
	}

	assert (buf != 0);
	os_memcpy (Buffer + sizeof (TEthernetHeader), buf, len);

	assert (l2->link != 0);
	l2->link->SendRaw (Buffer, len + sizeof (TEthernetHeader));

	return 0;
}

static void l2_packet_receive (int sock, void *eloop_ctx, void *sock_ctx)
{
	assert (sock == SOCK_FD);
	l2_packet_data *l2 = (l2_packet_data *) eloop_ctx;
	assert (l2 != 0);
	assert (l2->link != 0);
	assert (l2->rx_callback != 0);

	u8 Buffer[FRAME_BUFFER_SIZE];
	unsigned nResultLength;
	CMACAddress Sender;
	while (l2->link->ReceiveRaw (Buffer, &nResultLength, &Sender))
	{
		u8 src_addr[MAC_ADDRESS_SIZE];
		Sender.CopyTo (src_addr);

		(*l2->rx_callback) (l2->rx_callback_ctx, src_addr, Buffer, nResultLength);

		CScheduler::Get ()->Yield ();
	}
}

#define AUTH_DURATION_SECS	5

static void l2_packet_auth_end (void *eloop_ctx, void *timeout_ctx)
{
	auth_active = 0;
}

void l2_packet_notify_auth_start (l2_packet_data *l2)
{
	assert (l2 != 0);

	auth_active = 1;

	eloop_cancel_timeout (l2_packet_auth_end, l2, 0);
	eloop_register_timeout (AUTH_DURATION_SECS, 0, l2_packet_auth_end, l2, 0);
}

int l2_packet_get_ip_addr (l2_packet_data *l2, char *buf, size_t len)
{
	return -1;
}

extern "C" int l2_packet_auth_active (void)
{
	return auth_active;
}
