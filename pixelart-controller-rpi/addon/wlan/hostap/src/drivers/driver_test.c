/*
 * WPA Supplicant - testing driver interface
 * Copyright (c) 2004-2008, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

/* Make sure we get winsock2.h for Windows build to get sockaddr_storage */
#include "build_config.h"
#ifdef CONFIG_NATIVE_WINDOWS
#include <winsock2.h>
#endif /* CONFIG_NATIVE_WINDOWS */

#include "includes.h"

#ifndef CONFIG_NATIVE_WINDOWS
#include <sys/un.h>
#include <dirent.h>
#include <sys/stat.h>
#define DRIVER_TEST_UNIX
#endif /* CONFIG_NATIVE_WINDOWS */

#include "common.h"
#include "driver.h"
#include "l2_packet/l2_packet.h"
#include "eloop.h"
#include "sha1.h"
#include "ieee802_11_defs.h"

#include "../../hostapd/hostapd.h"
#include "../../hostapd/wpa.h"
#include "../../hostapd/hw_features.h"


struct test_client_socket {
	struct test_client_socket *next;
	u8 addr[ETH_ALEN];
	struct sockaddr_un un;
	socklen_t unlen;
	struct test_driver_bss *bss;
};

struct test_driver_bss {
	struct test_driver_bss *next;
	char ifname[IFNAMSIZ + 1];
	u8 bssid[ETH_ALEN];
	u8 *ie;
	size_t ielen;
	u8 *wps_beacon_ie;
	size_t wps_beacon_ie_len;
	u8 *wps_probe_resp_ie;
	size_t wps_probe_resp_ie_len;
	u8 ssid[32];
	size_t ssid_len;
	int privacy;
};

struct wpa_driver_test_global {
	int dummy;
};

struct wpa_driver_test_data {
	struct wpa_driver_test_global *global;
	void *ctx;
	u8 own_addr[ETH_ALEN];
	int test_socket;
#ifdef DRIVER_TEST_UNIX
	struct sockaddr_un hostapd_addr;
#endif /* DRIVER_TEST_UNIX */
	int hostapd_addr_set;
	struct sockaddr_in hostapd_addr_udp;
	int hostapd_addr_udp_set;
	char *own_socket_path;
	char *test_dir;
	u8 bssid[ETH_ALEN];
	u8 ssid[32];
	size_t ssid_len;
#define MAX_SCAN_RESULTS 30
	struct wpa_scan_res *scanres[MAX_SCAN_RESULTS];
	size_t num_scanres;
	int use_associnfo;
	u8 assoc_wpa_ie[80];
	size_t assoc_wpa_ie_len;
	int use_mlme;
	int associated;
	u8 *probe_req_ie;
	size_t probe_req_ie_len;
	int ibss;
	int privacy;
	int ap;

	struct hostapd_data *hapd;
	struct test_client_socket *cli;
	struct test_driver_bss *bss;
	int udp_port;
};


static void wpa_driver_test_deinit(void *priv);
static int wpa_driver_test_attach(struct wpa_driver_test_data *drv,
				  const char *dir, int ap);
static void wpa_driver_test_close_test_socket(
	struct wpa_driver_test_data *drv);

#ifdef CONFIG_AP
void ap_mgmt_rx(void *ctx, u8 *buf, size_t len, u16 stype,
		struct hostapd_frame_info *fi);
void ap_mgmt_tx_cb(void *ctx, u8 *buf, size_t len, u16 stype, int ok);

#else /* CONFIG_AP */
#ifndef HOSTAPD
static inline void ap_mgmt_rx(void *ctx, u8 *buf, size_t len, u16 stype,
			      struct hostapd_frame_info *fi)
{
}

static inline void ap_mgmt_tx_cb(void *ctx, u8 *buf, size_t len, u16 stype,
				 int ok)
{
}
#endif /* HOSTAPD */
#endif /* CONFIG_AP */


static void test_driver_free_bss(struct test_driver_bss *bss)
{
	free(bss->ie);
	free(bss->wps_beacon_ie);
	free(bss->wps_probe_resp_ie);
	free(bss);
}


static void test_driver_free_bsses(struct wpa_driver_test_data *drv)
{
	struct test_driver_bss *bss, *prev_bss;

	bss = drv->bss;
	while (bss) {
		prev_bss = bss;
		bss = bss->next;
		test_driver_free_bss(prev_bss);
	}

	drv->bss = NULL;
}


static struct test_client_socket *
test_driver_get_cli(struct wpa_driver_test_data *drv, struct sockaddr_un *from,
		    socklen_t fromlen)
{
	struct test_client_socket *cli = drv->cli;

	while (cli) {
		if (cli->unlen == fromlen &&
		    strncmp(cli->un.sun_path, from->sun_path,
			    fromlen - sizeof(cli->un.sun_family)) == 0)
			return cli;
		cli = cli->next;
	}

	return NULL;
}


static int test_driver_send_eapol(void *priv, const u8 *addr, const u8 *data,
				  size_t data_len, int encrypt,
				  const u8 *own_addr)
{
	struct wpa_driver_test_data *drv = priv;
	struct test_client_socket *cli;
	struct msghdr msg;
	struct iovec io[3];
	struct l2_ethhdr eth;

	if (drv->test_socket < 0)
		return -1;

	cli = drv->cli;
	while (cli) {
		if (memcmp(cli->addr, addr, ETH_ALEN) == 0)
			break;
		cli = cli->next;
	}

	if (!cli) {
		wpa_printf(MSG_DEBUG, "%s: no destination client entry",
			   __func__);
		return -1;
	}

	memcpy(eth.h_dest, addr, ETH_ALEN);
	memcpy(eth.h_source, own_addr, ETH_ALEN);
	eth.h_proto = host_to_be16(ETH_P_EAPOL);

	io[0].iov_base = "EAPOL ";
	io[0].iov_len = 6;
	io[1].iov_base = &eth;
	io[1].iov_len = sizeof(eth);
	io[2].iov_base = (u8 *) data;
	io[2].iov_len = data_len;

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = io;
	msg.msg_iovlen = 3;
	msg.msg_name = &cli->un;
	msg.msg_namelen = cli->unlen;
	return sendmsg(drv->test_socket, &msg, 0);
}


static int test_driver_send_ether(void *priv, const u8 *dst, const u8 *src,
				  u16 proto, const u8 *data, size_t data_len)
{
	struct wpa_driver_test_data *drv = priv;
	struct msghdr msg;
	struct iovec io[3];
	struct l2_ethhdr eth;
	char desttxt[30];
	struct sockaddr_un addr;
	struct dirent *dent;
	DIR *dir;
	int ret = 0, broadcast = 0, count = 0;

	if (drv->test_socket < 0 || drv->test_dir == NULL) {
		wpa_printf(MSG_DEBUG, "%s: invalid parameters (sock=%d "
			   "test_dir=%p)",
			   __func__, drv->test_socket, drv->test_dir);
		return -1;
	}

	broadcast = memcmp(dst, "\xff\xff\xff\xff\xff\xff", ETH_ALEN) == 0;
	snprintf(desttxt, sizeof(desttxt), MACSTR, MAC2STR(dst));

	memcpy(eth.h_dest, dst, ETH_ALEN);
	memcpy(eth.h_source, src, ETH_ALEN);
	eth.h_proto = host_to_be16(proto);

	io[0].iov_base = "ETHER ";
	io[0].iov_len = 6;
	io[1].iov_base = &eth;
	io[1].iov_len = sizeof(eth);
	io[2].iov_base = (u8 *) data;
	io[2].iov_len = data_len;

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = io;
	msg.msg_iovlen = 3;

	dir = opendir(drv->test_dir);
	if (dir == NULL) {
		perror("test_driver: opendir");
		return -1;
	}
	while ((dent = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
		/* Skip the file if it is not a socket. Also accept
		 * DT_UNKNOWN (0) in case the C library or underlying file
		 * system does not support d_type. */
		if (dent->d_type != DT_SOCK && dent->d_type != DT_UNKNOWN)
			continue;
#endif /* _DIRENT_HAVE_D_TYPE */
		if (strcmp(dent->d_name, ".") == 0 ||
		    strcmp(dent->d_name, "..") == 0)
			continue;

		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/%s",
			 drv->test_dir, dent->d_name);

		if (strcmp(addr.sun_path, drv->own_socket_path) == 0)
			continue;
		if (!broadcast && strstr(dent->d_name, desttxt) == NULL)
			continue;

		wpa_printf(MSG_DEBUG, "%s: Send ether frame to %s",
			   __func__, dent->d_name);

		msg.msg_name = &addr;
		msg.msg_namelen = sizeof(addr);
		ret = sendmsg(drv->test_socket, &msg, 0);
		if (ret < 0)
			perror("driver_test: sendmsg");
		count++;
	}
	closedir(dir);

	if (!broadcast && count == 0) {
		wpa_printf(MSG_DEBUG, "%s: Destination " MACSTR " not found",
			   __func__, MAC2STR(dst));
		return -1;
	}

	return ret;
}


static int wpa_driver_test_send_mlme(void *priv, const u8 *data,
				     size_t data_len)
{
	struct wpa_driver_test_data *drv = priv;
	struct msghdr msg;
	struct iovec io[2];
	const u8 *dest;
	struct sockaddr_un addr;
	struct dirent *dent;
	DIR *dir;
	int broadcast;
	int ret = 0;
	struct ieee80211_hdr *hdr;
	u16 fc;
#ifdef HOSTAPD
	char desttxt[30];
#endif /* HOSTAPD */

	wpa_hexdump(MSG_MSGDUMP, "test_send_mlme", data, data_len);
	if (drv->test_socket < 0 || data_len < 10) {
		wpa_printf(MSG_DEBUG, "%s: invalid parameters (sock=%d len=%lu"
			   " test_dir=%p)",
			   __func__, drv->test_socket,
			   (unsigned long) data_len,
			   drv->test_dir);
		return -1;
	}

	dest = data + 4;
	broadcast = os_memcmp(dest, "\xff\xff\xff\xff\xff\xff", ETH_ALEN) == 0;

#ifdef HOSTAPD
	snprintf(desttxt, sizeof(desttxt), MACSTR, MAC2STR(dest));
#endif /* HOSTAPD */

	io[0].iov_base = "MLME ";
	io[0].iov_len = 5;
	io[1].iov_base = (void *) data;
	io[1].iov_len = data_len;

	os_memset(&msg, 0, sizeof(msg));
	msg.msg_iov = io;
	msg.msg_iovlen = 2;

#ifdef HOSTAPD
	if (drv->test_dir == NULL) {
		wpa_printf(MSG_DEBUG, "%s: test_dir == NULL", __func__);
		return -1;
	}

	dir = opendir(drv->test_dir);
	if (dir == NULL) {
		perror("test_driver: opendir");
		return -1;
	}
	while ((dent = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
		/* Skip the file if it is not a socket. Also accept
		 * DT_UNKNOWN (0) in case the C library or underlying file
		 * system does not support d_type. */
		if (dent->d_type != DT_SOCK && dent->d_type != DT_UNKNOWN)
			continue;
#endif /* _DIRENT_HAVE_D_TYPE */
		if (os_strcmp(dent->d_name, ".") == 0 ||
		    os_strcmp(dent->d_name, "..") == 0)
			continue;

		os_memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		os_snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/%s",
			    drv->test_dir, dent->d_name);

		if (os_strcmp(addr.sun_path, drv->own_socket_path) == 0)
			continue;
		if (!broadcast && os_strstr(dent->d_name, desttxt) == NULL)
			continue;

		wpa_printf(MSG_DEBUG, "%s: Send management frame to %s",
			   __func__, dent->d_name);

		msg.msg_name = &addr;
		msg.msg_namelen = sizeof(addr);
		ret = sendmsg(drv->test_socket, &msg, 0);
		if (ret < 0)
			perror("driver_test: sendmsg(test_socket)");
	}
	closedir(dir);
#else /* HOSTAPD */

	if (os_memcmp(dest, drv->bssid, ETH_ALEN) == 0 ||
	    drv->test_dir == NULL) {
		if (drv->hostapd_addr_udp_set) {
			msg.msg_name = &drv->hostapd_addr_udp;
			msg.msg_namelen = sizeof(drv->hostapd_addr_udp);
		} else {
#ifdef DRIVER_TEST_UNIX
			msg.msg_name = &drv->hostapd_addr;
			msg.msg_namelen = sizeof(drv->hostapd_addr);
#endif /* DRIVER_TEST_UNIX */
		}
	} else if (broadcast) {
		dir = opendir(drv->test_dir);
		if (dir == NULL)
			return -1;
		while ((dent = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
			/* Skip the file if it is not a socket.
			 * Also accept DT_UNKNOWN (0) in case
			 * the C library or underlying file
			 * system does not support d_type. */
			if (dent->d_type != DT_SOCK &&
			    dent->d_type != DT_UNKNOWN)
				continue;
#endif /* _DIRENT_HAVE_D_TYPE */
			if (os_strcmp(dent->d_name, ".") == 0 ||
			    os_strcmp(dent->d_name, "..") == 0)
				continue;
			wpa_printf(MSG_DEBUG, "%s: Send broadcast MLME to %s",
				   __func__, dent->d_name);
			os_memset(&addr, 0, sizeof(addr));
			addr.sun_family = AF_UNIX;
			os_snprintf(addr.sun_path, sizeof(addr.sun_path),
				    "%s/%s", drv->test_dir, dent->d_name);

			msg.msg_name = &addr;
			msg.msg_namelen = sizeof(addr);

			ret = sendmsg(drv->test_socket, &msg, 0);
			if (ret < 0)
				perror("driver_test: sendmsg(test_socket)");
		}
		closedir(dir);
		return ret;
	} else {
		struct stat st;
		os_memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		os_snprintf(addr.sun_path, sizeof(addr.sun_path),
			    "%s/AP-" MACSTR, drv->test_dir, MAC2STR(dest));
		if (stat(addr.sun_path, &st) < 0) {
			os_snprintf(addr.sun_path, sizeof(addr.sun_path),
				    "%s/STA-" MACSTR,
				    drv->test_dir, MAC2STR(dest));
		}
		msg.msg_name = &addr;
		msg.msg_namelen = sizeof(addr);
	}

	if (sendmsg(drv->test_socket, &msg, 0) < 0) {
		perror("sendmsg(test_socket)");
		return -1;
	}
#endif /* HOSTAPD */

	hdr = (struct ieee80211_hdr *) data;
	fc = le_to_host16(hdr->frame_control);
#ifdef HOSTAPD
	hostapd_mgmt_tx_cb(drv->hapd, (u8 *) data, data_len,
			   WLAN_FC_GET_STYPE(fc), ret >= 0);
#else /* HOSTAPD */
	if (drv->ap) {
		ap_mgmt_tx_cb(drv->ctx, (u8 *) data, data_len,
			      WLAN_FC_GET_STYPE(fc), ret >= 0);
	}
#endif /* HOSTAPD */

	return ret;
}


static void test_driver_scan(struct wpa_driver_test_data *drv,
			     struct sockaddr_un *from, socklen_t fromlen,
			     char *data)
{
	char buf[512], *pos, *end;
	int ret;
	struct test_driver_bss *bss;
	u8 sa[ETH_ALEN];
	u8 ie[512];
	size_t ielen;

	/* data: optional [ ' ' | STA-addr | ' ' | IEs(hex) ] */

	wpa_printf(MSG_DEBUG, "test_driver: SCAN");

	if (*data) {
		if (*data != ' ' ||
		    hwaddr_aton(data + 1, sa)) {
			wpa_printf(MSG_DEBUG, "test_driver: Unexpected SCAN "
				   "command format");
			return;
		}

		data += 18;
		while (*data == ' ')
			data++;
		ielen = os_strlen(data) / 2;
		if (ielen > sizeof(ie))
			ielen = sizeof(ie);
		if (hexstr2bin(data, ie, ielen) < 0)
			ielen = 0;

		wpa_printf(MSG_DEBUG, "test_driver: Scan from " MACSTR,
			   MAC2STR(sa));
		wpa_hexdump(MSG_MSGDUMP, "test_driver: scan IEs", ie, ielen);

#ifdef HOSTAPD
		hostapd_probe_req_rx(drv->hapd, sa, ie, ielen);
#endif /* HOSTAPD */
	}

	for (bss = drv->bss; bss; bss = bss->next) {
		pos = buf;
		end = buf + sizeof(buf);

		/* reply: SCANRESP BSSID SSID IEs */
		ret = snprintf(pos, end - pos, "SCANRESP " MACSTR " ",
			       MAC2STR(bss->bssid));
		if (ret < 0 || ret >= end - pos)
			return;
		pos += ret;
		pos += wpa_snprintf_hex(pos, end - pos,
					bss->ssid, bss->ssid_len);
		ret = snprintf(pos, end - pos, " ");
		if (ret < 0 || ret >= end - pos)
			return;
		pos += ret;
		pos += wpa_snprintf_hex(pos, end - pos, bss->ie, bss->ielen);
		pos += wpa_snprintf_hex(pos, end - pos, bss->wps_probe_resp_ie,
					bss->wps_probe_resp_ie_len);

		if (bss->privacy) {
			ret = snprintf(pos, end - pos, " PRIVACY");
			if (ret < 0 || ret >= end - pos)
				return;
			pos += ret;
		}

		sendto(drv->test_socket, buf, pos - buf, 0,
		       (struct sockaddr *) from, fromlen);
	}
}


static struct hostapd_data *
test_driver_get_hapd(struct wpa_driver_test_data *drv,
		     struct test_driver_bss *bss)
{
#ifdef HOSTAPD
	struct hostapd_iface *iface = drv->hapd->iface;
	struct hostapd_data *hapd = NULL;
	size_t i;

	if (bss == NULL) {
		wpa_printf(MSG_DEBUG, "%s: bss == NULL", __func__);
		return NULL;
	}

	for (i = 0; i < iface->num_bss; i++) {
		hapd = iface->bss[i];
		if (memcmp(hapd->own_addr, bss->bssid, ETH_ALEN) == 0)
			break;
	}
	if (i == iface->num_bss) {
		wpa_printf(MSG_DEBUG, "%s: no matching interface entry found "
			   "for BSSID " MACSTR, __func__, MAC2STR(bss->bssid));
		return NULL;
	}

	return hapd;
#else /* HOSTAPD */
	return NULL;
#endif /* HOSTAPD */
}


static int test_driver_new_sta(struct wpa_driver_test_data *drv,
			       struct test_driver_bss *bss, const u8 *addr,
			       const u8 *ie, size_t ielen)
{
	struct hostapd_data *hapd;

	hapd = test_driver_get_hapd(drv, bss);
	if (hapd == NULL)
		return -1;

#ifdef HOSTAPD
	return hostapd_notif_assoc(hapd, addr, ie, ielen);
#else /* HOSTAPD */
	return -1;
#endif /* HOSTAPD */
}


static void test_driver_assoc(struct wpa_driver_test_data *drv,
			      struct sockaddr_un *from, socklen_t fromlen,
			      char *data)
{
	struct test_client_socket *cli;
	u8 ie[256], ssid[32];
	size_t ielen, ssid_len = 0;
	char *pos, *pos2, cmd[50];
	struct test_driver_bss *bss;

	/* data: STA-addr SSID(hex) IEs(hex) */

	cli = os_zalloc(sizeof(*cli));
	if (cli == NULL)
		return;

	if (hwaddr_aton(data, cli->addr)) {
		printf("test_socket: Invalid MAC address '%s' in ASSOC\n",
		       data);
		free(cli);
		return;
	}
	pos = data + 17;
	while (*pos == ' ')
		pos++;
	pos2 = strchr(pos, ' ');
	ielen = 0;
	if (pos2) {
		ssid_len = (pos2 - pos) / 2;
		if (hexstr2bin(pos, ssid, ssid_len) < 0) {
			wpa_printf(MSG_DEBUG, "%s: Invalid SSID", __func__);
			free(cli);
			return;
		}
		wpa_hexdump_ascii(MSG_DEBUG, "test_driver_assoc: SSID",
				  ssid, ssid_len);

		pos = pos2 + 1;
		ielen = strlen(pos) / 2;
		if (ielen > sizeof(ie))
			ielen = sizeof(ie);
		if (hexstr2bin(pos, ie, ielen) < 0)
			ielen = 0;
	}

	for (bss = drv->bss; bss; bss = bss->next) {
		if (bss->ssid_len == ssid_len &&
		    memcmp(bss->ssid, ssid, ssid_len) == 0)
			break;
	}
	if (bss == NULL) {
		wpa_printf(MSG_DEBUG, "%s: No matching SSID found from "
			   "configured BSSes", __func__);
		free(cli);
		return;
	}

	cli->bss = bss;
	memcpy(&cli->un, from, sizeof(cli->un));
	cli->unlen = fromlen;
	cli->next = drv->cli;
	drv->cli = cli;
	wpa_hexdump_ascii(MSG_DEBUG, "test_socket: ASSOC sun_path",
			  (const u8 *) cli->un.sun_path,
			  cli->unlen - sizeof(cli->un.sun_family));

	snprintf(cmd, sizeof(cmd), "ASSOCRESP " MACSTR " 0",
		 MAC2STR(bss->bssid));
	sendto(drv->test_socket, cmd, strlen(cmd), 0,
	       (struct sockaddr *) from, fromlen);

	if (test_driver_new_sta(drv, bss, cli->addr, ie, ielen) < 0) {
		wpa_printf(MSG_DEBUG, "test_driver: failed to add new STA");
	}
}


static void test_driver_disassoc(struct wpa_driver_test_data *drv,
				 struct sockaddr_un *from, socklen_t fromlen)
{
	struct test_client_socket *cli;

	cli = test_driver_get_cli(drv, from, fromlen);
	if (!cli)
		return;

#ifdef HOSTAPD
	hostapd_notif_disassoc(drv->hapd, cli->addr);
#endif /* HOSTAPD */
}


static void test_driver_eapol(struct wpa_driver_test_data *drv,
			      struct sockaddr_un *from, socklen_t fromlen,
			      u8 *data, size_t datalen)
{
#ifdef HOSTAPD
	struct test_client_socket *cli;
#endif /* HOSTAPD */
	const u8 *src = NULL;

	if (datalen > 14) {
		/* Skip Ethernet header */
		src = data + ETH_ALEN;
		wpa_printf(MSG_DEBUG, "test_driver: dst=" MACSTR " src="
			   MACSTR " proto=%04x",
			   MAC2STR(data), MAC2STR(src),
			   WPA_GET_BE16(data + 2 * ETH_ALEN));
		data += 14;
		datalen -= 14;
	}
#ifdef HOSTAPD
	cli = test_driver_get_cli(drv, from, fromlen);
	if (cli) {
		struct hostapd_data *hapd;
		hapd = test_driver_get_hapd(drv, cli->bss);
		if (hapd == NULL)
			return;
		hostapd_eapol_receive(hapd, cli->addr, data, datalen);
	} else {
		wpa_printf(MSG_DEBUG, "test_socket: EAPOL from unknown "
			   "client");
	}
#else /* HOSTAPD */
	if (src)
		wpa_supplicant_rx_eapol(drv->ctx, src, data, datalen);
#endif /* HOSTAPD */
}


static void test_driver_ether(struct wpa_driver_test_data *drv,
			      struct sockaddr_un *from, socklen_t fromlen,
			      u8 *data, size_t datalen)
{
	struct l2_ethhdr *eth;

	if (datalen < sizeof(*eth))
		return;

	eth = (struct l2_ethhdr *) data;
	wpa_printf(MSG_DEBUG, "test_driver: RX ETHER dst=" MACSTR " src="
		   MACSTR " proto=%04x",
		   MAC2STR(eth->h_dest), MAC2STR(eth->h_source),
		   be_to_host16(eth->h_proto));

#ifdef CONFIG_IEEE80211R
	if (be_to_host16(eth->h_proto) == ETH_P_RRB) {
#ifdef HOSTAPD
		wpa_ft_rrb_rx(drv->hapd->wpa_auth, eth->h_source,
			      data + sizeof(*eth), datalen - sizeof(*eth));
#endif /* HOSTAPD */
	}
#endif /* CONFIG_IEEE80211R */
}


static void test_driver_mlme(struct wpa_driver_test_data *drv,
			     struct sockaddr_un *from, socklen_t fromlen,
			     u8 *data, size_t datalen)
{
	struct ieee80211_hdr *hdr;
	u16 fc;

	hdr = (struct ieee80211_hdr *) data;

	if (test_driver_get_cli(drv, from, fromlen) == NULL && datalen >= 16) {
		struct test_client_socket *cli;
		cli = os_zalloc(sizeof(*cli));
		if (cli == NULL)
			return;
		wpa_printf(MSG_DEBUG, "Adding client entry for " MACSTR,
			   MAC2STR(hdr->addr2));
		memcpy(cli->addr, hdr->addr2, ETH_ALEN);
		memcpy(&cli->un, from, sizeof(cli->un));
		cli->unlen = fromlen;
		cli->next = drv->cli;
		drv->cli = cli;
	}

	wpa_hexdump(MSG_MSGDUMP, "test_driver_mlme: received frame",
		    data, datalen);
	fc = le_to_host16(hdr->frame_control);
	if (WLAN_FC_GET_TYPE(fc) != WLAN_FC_TYPE_MGMT) {
		wpa_printf(MSG_ERROR, "%s: received non-mgmt frame",
			   __func__);
		return;
	}
#ifdef HOSTAPD
	hostapd_mgmt_rx(drv->hapd, data, datalen, WLAN_FC_GET_STYPE(fc), NULL);
#else /* HOSTAPD */
	ap_mgmt_rx(drv->ctx, data, datalen, WLAN_FC_GET_STYPE(fc), NULL);
#endif /* HOSTAPD */
}


static void test_driver_receive_unix(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct wpa_driver_test_data *drv = eloop_ctx;
	char buf[2000];
	int res;
	struct sockaddr_un from;
	socklen_t fromlen = sizeof(from);

	res = recvfrom(sock, buf, sizeof(buf) - 1, 0,
		       (struct sockaddr *) &from, &fromlen);
	if (res < 0) {
		perror("recvfrom(test_socket)");
		return;
	}
	buf[res] = '\0';

	wpa_printf(MSG_DEBUG, "test_driver: received %u bytes", res);

	if (strncmp(buf, "SCAN", 4) == 0) {
		test_driver_scan(drv, &from, fromlen, buf + 4);
	} else if (strncmp(buf, "ASSOC ", 6) == 0) {
		test_driver_assoc(drv, &from, fromlen, buf + 6);
	} else if (strcmp(buf, "DISASSOC") == 0) {
		test_driver_disassoc(drv, &from, fromlen);
	} else if (strncmp(buf, "EAPOL ", 6) == 0) {
		test_driver_eapol(drv, &from, fromlen, (u8 *) buf + 6,
				  res - 6);
	} else if (strncmp(buf, "ETHER ", 6) == 0) {
		test_driver_ether(drv, &from, fromlen, (u8 *) buf + 6,
				  res - 6);
	} else if (strncmp(buf, "MLME ", 5) == 0) {
		test_driver_mlme(drv, &from, fromlen, (u8 *) buf + 5, res - 5);
	} else {
		wpa_hexdump_ascii(MSG_DEBUG, "Unknown test_socket command",
				  (u8 *) buf, res);
	}
}


static struct test_driver_bss *
test_driver_get_bss(struct wpa_driver_test_data *drv, const char *ifname)
{
	struct test_driver_bss *bss;

	for (bss = drv->bss; bss; bss = bss->next) {
		if (strcmp(bss->ifname, ifname) == 0)
			return bss;
	}
	return NULL;
}


static int test_driver_set_generic_elem(const char *ifname, void *priv,
					const u8 *elem, size_t elem_len)
{
	struct wpa_driver_test_data *drv = priv;
	struct test_driver_bss *bss;

	bss = test_driver_get_bss(drv, ifname);
	if (bss == NULL)
		return -1;

	free(bss->ie);

	if (elem == NULL) {
		bss->ie = NULL;
		bss->ielen = 0;
		return 0;
	}

	bss->ie = malloc(elem_len);
	if (bss->ie == NULL) {
		bss->ielen = 0;
		return -1;
	}

	memcpy(bss->ie, elem, elem_len);
	bss->ielen = elem_len;
	return 0;
}


static int test_driver_set_wps_beacon_ie(const char *ifname, void *priv,
					 const u8 *ie, size_t len)
{
	struct wpa_driver_test_data *drv = priv;
	struct test_driver_bss *bss;

	wpa_hexdump(MSG_DEBUG, "test_driver: Beacon WPS IE", ie, len);
	bss = test_driver_get_bss(drv, ifname);
	if (bss == NULL)
		return -1;

	free(bss->wps_beacon_ie);

	if (ie == NULL) {
		bss->wps_beacon_ie = NULL;
		bss->wps_beacon_ie_len = 0;
		return 0;
	}

	bss->wps_beacon_ie = malloc(len);
	if (bss->wps_beacon_ie == NULL) {
		bss->wps_beacon_ie_len = 0;
		return -1;
	}

	memcpy(bss->wps_beacon_ie, ie, len);
	bss->wps_beacon_ie_len = len;
	return 0;
}


static int test_driver_set_wps_probe_resp_ie(const char *ifname, void *priv,
					     const u8 *ie, size_t len)
{
	struct wpa_driver_test_data *drv = priv;
	struct test_driver_bss *bss;

	wpa_hexdump(MSG_DEBUG, "test_driver: ProbeResp WPS IE", ie, len);
	bss = test_driver_get_bss(drv, ifname);
	if (bss == NULL)
		return -1;

	free(bss->wps_probe_resp_ie);

	if (ie == NULL) {
		bss->wps_probe_resp_ie = NULL;
		bss->wps_probe_resp_ie_len = 0;
		return 0;
	}

	bss->wps_probe_resp_ie = malloc(len);
	if (bss->wps_probe_resp_ie == NULL) {
		bss->wps_probe_resp_ie_len = 0;
		return -1;
	}

	memcpy(bss->wps_probe_resp_ie, ie, len);
	bss->wps_probe_resp_ie_len = len;
	return 0;
}


static int test_driver_sta_deauth(void *priv, const u8 *own_addr,
				  const u8 *addr, int reason)
{
	struct wpa_driver_test_data *drv = priv;
	struct test_client_socket *cli;

	if (drv->test_socket < 0)
		return -1;

	cli = drv->cli;
	while (cli) {
		if (memcmp(cli->addr, addr, ETH_ALEN) == 0)
			break;
		cli = cli->next;
	}

	if (!cli)
		return -1;

	return sendto(drv->test_socket, "DEAUTH", 6, 0,
		      (struct sockaddr *) &cli->un, cli->unlen);
}


static int test_driver_sta_disassoc(void *priv, const u8 *own_addr,
				    const u8 *addr, int reason)
{
	struct wpa_driver_test_data *drv = priv;
	struct test_client_socket *cli;

	if (drv->test_socket < 0)
		return -1;

	cli = drv->cli;
	while (cli) {
		if (memcmp(cli->addr, addr, ETH_ALEN) == 0)
			break;
		cli = cli->next;
	}

	if (!cli)
		return -1;

	return sendto(drv->test_socket, "DISASSOC", 8, 0,
		      (struct sockaddr *) &cli->un, cli->unlen);
}


static int test_driver_bss_add(void *priv, const char *ifname, const u8 *bssid)
{
	struct wpa_driver_test_data *drv = priv;
	struct test_driver_bss *bss;

	wpa_printf(MSG_DEBUG, "%s(ifname=%s bssid=" MACSTR ")",
		   __func__, ifname, MAC2STR(bssid));

	bss = os_zalloc(sizeof(*bss));
	if (bss == NULL)
		return -1;

	os_strlcpy(bss->ifname, ifname, IFNAMSIZ);
	memcpy(bss->bssid, bssid, ETH_ALEN);

	bss->next = drv->bss;
	drv->bss = bss;

	return 0;
}


static int test_driver_bss_remove(void *priv, const char *ifname)
{
	struct wpa_driver_test_data *drv = priv;
	struct test_driver_bss *bss, *prev;
	struct test_client_socket *cli, *prev_c;

	wpa_printf(MSG_DEBUG, "%s(ifname=%s)", __func__, ifname);

	for (prev = NULL, bss = drv->bss; bss; prev = bss, bss = bss->next) {
		if (strcmp(bss->ifname, ifname) != 0)
			continue;

		if (prev)
			prev->next = bss->next;
		else
			drv->bss = bss->next;

		for (prev_c = NULL, cli = drv->cli; cli;
		     prev_c = cli, cli = cli->next) {
			if (cli->bss != bss)
				continue;
			if (prev_c)
				prev_c->next = cli->next;
			else
				drv->cli = cli->next;
			free(cli);
			break;
		}

		test_driver_free_bss(bss);
		return 0;
	}

	return -1;
}


static int test_driver_if_add(const char *iface, void *priv,
			      enum hostapd_driver_if_type type, char *ifname,
			      const u8 *addr)
{
	wpa_printf(MSG_DEBUG, "%s(iface=%s type=%d ifname=%s)",
		   __func__, iface, type, ifname);
	return 0;
}


static int test_driver_if_update(void *priv, enum hostapd_driver_if_type type,
				 char *ifname, const u8 *addr)
{
	wpa_printf(MSG_DEBUG, "%s(type=%d ifname=%s)", __func__, type, ifname);
	return 0;
}


static int test_driver_if_remove(void *priv, enum hostapd_driver_if_type type,
				 const char *ifname, const u8 *addr)
{
	wpa_printf(MSG_DEBUG, "%s(type=%d ifname=%s)", __func__, type, ifname);
	return 0;
}


static int test_driver_valid_bss_mask(void *priv, const u8 *addr,
				      const u8 *mask)
{
	return 0;
}


static int test_driver_set_ssid(const char *ifname, void *priv, const u8 *buf,
				int len)
{
	struct wpa_driver_test_data *drv = priv;
	struct test_driver_bss *bss;

	wpa_printf(MSG_DEBUG, "%s(ifname=%s)", __func__, ifname);
	wpa_hexdump_ascii(MSG_DEBUG, "test_driver_set_ssid: SSID", buf, len);

	for (bss = drv->bss; bss; bss = bss->next) {
		if (strcmp(bss->ifname, ifname) != 0)
			continue;

		if (len < 0 || (size_t) len > sizeof(bss->ssid))
			return -1;

		memcpy(bss->ssid, buf, len);
		bss->ssid_len = len;

		return 0;
	}

	return -1;
}


static int test_driver_set_privacy(const char *ifname, void *priv, int enabled)
{
	struct wpa_driver_test_data *drv = priv;
	struct test_driver_bss *bss;

	wpa_printf(MSG_DEBUG, "%s(ifname=%s enabled=%d)",
		   __func__, ifname, enabled);

	for (bss = drv->bss; bss; bss = bss->next) {
		if (strcmp(bss->ifname, ifname) != 0)
			continue;

		bss->privacy = enabled;

		return 0;
	}

	return -1;
}


static int test_driver_set_key(const char *iface, void *priv, wpa_alg alg,
			       const u8 *addr, int key_idx, int set_tx,
			       const u8 *seq, size_t seq_len,
			       const u8 *key, size_t key_len)
{
	wpa_printf(MSG_DEBUG, "%s(iface=%s alg=%d idx=%d set_tx=%d)",
		   __func__, iface, alg, key_idx, set_tx);
	if (addr)
		wpa_printf(MSG_DEBUG, "   addr=" MACSTR, MAC2STR(addr));
	if (key)
		wpa_hexdump_key(MSG_DEBUG, "   key", key, key_len);
	return 0;
}


static int test_driver_set_sta_vlan(void *priv, const u8 *addr,
				    const char *ifname, int vlan_id)
{
	wpa_printf(MSG_DEBUG, "%s(addr=" MACSTR " ifname=%s vlan_id=%d)",
		   __func__, MAC2STR(addr), ifname, vlan_id);
	return 0;
}


static int test_driver_sta_add(const char *ifname, void *priv,
			       struct hostapd_sta_add_params *params)
{
	struct wpa_driver_test_data *drv = priv;
	struct test_client_socket *cli;
	struct test_driver_bss *bss;

	wpa_printf(MSG_DEBUG, "%s(ifname=%s addr=" MACSTR " aid=%d "
		   "capability=0x%x flags=0x%x listen_interval=%d)",
		   __func__, ifname, MAC2STR(params->addr), params->aid,
		   params->capability, params->flags,
		   params->listen_interval);
	wpa_hexdump(MSG_DEBUG, "test_driver_sta_add - supp_rates",
		    params->supp_rates, params->supp_rates_len);

	cli = drv->cli;
	while (cli) {
		if (os_memcmp(cli->addr, params->addr, ETH_ALEN) == 0)
			break;
		cli = cli->next;
	}
	if (!cli) {
		wpa_printf(MSG_DEBUG, "%s: no matching client entry",
			   __func__);
		return -1;
	}

	for (bss = drv->bss; bss; bss = bss->next) {
		if (strcmp(ifname, bss->ifname) == 0)
			break;
	}
	if (bss == NULL) {
		wpa_printf(MSG_DEBUG, "%s: No matching interface found from "
			   "configured BSSes", __func__);
		return -1;
	}

	cli->bss = bss;

	return 0;
}


static void * test_driver_init(struct hostapd_data *hapd,
			       struct wpa_init_params *params)
{
	struct wpa_driver_test_data *drv;
	struct sockaddr_un addr_un;
	struct sockaddr_in addr_in;
	struct sockaddr *addr;
	socklen_t alen;

	drv = os_zalloc(sizeof(struct wpa_driver_test_data));
	if (drv == NULL) {
		printf("Could not allocate memory for test driver data\n");
		return NULL;
	}
	drv->ap = 1;
	drv->bss = os_zalloc(sizeof(*drv->bss));
	if (drv->bss == NULL) {
		printf("Could not allocate memory for test driver BSS data\n");
		free(drv);
		return NULL;
	}

	drv->hapd = hapd;

	/* Generate a MAC address to help testing with multiple APs */
	params->own_addr[0] = 0x02; /* locally administered */
	sha1_prf((const u8 *) params->ifname, strlen(params->ifname),
		 "hostapd test bssid generation",
		 params->ssid, params->ssid_len,
		 params->own_addr + 1, ETH_ALEN - 1);

	os_strlcpy(drv->bss->ifname, params->ifname, IFNAMSIZ);
	memcpy(drv->bss->bssid, params->own_addr, ETH_ALEN);

	if (params->test_socket) {
		if (os_strlen(params->test_socket) >=
		    sizeof(addr_un.sun_path)) {
			printf("Too long test_socket path\n");
			wpa_driver_test_deinit(drv);
			return NULL;
		}
		if (strncmp(params->test_socket, "DIR:", 4) == 0) {
			size_t len = strlen(params->test_socket) + 30;
			drv->test_dir = strdup(params->test_socket + 4);
			drv->own_socket_path = malloc(len);
			if (drv->own_socket_path) {
				snprintf(drv->own_socket_path, len,
					 "%s/AP-" MACSTR,
					 params->test_socket + 4,
					 MAC2STR(params->own_addr));
			}
		} else if (strncmp(params->test_socket, "UDP:", 4) == 0) {
			drv->udp_port = atoi(params->test_socket + 4);
		} else {
			drv->own_socket_path = strdup(params->test_socket);
		}
		if (drv->own_socket_path == NULL && drv->udp_port == 0) {
			wpa_driver_test_deinit(drv);
			return NULL;
		}

		drv->test_socket = socket(drv->udp_port ? PF_INET : PF_UNIX,
					  SOCK_DGRAM, 0);
		if (drv->test_socket < 0) {
			perror("socket");
			wpa_driver_test_deinit(drv);
			return NULL;
		}

		if (drv->udp_port) {
			os_memset(&addr_in, 0, sizeof(addr_in));
			addr_in.sin_family = AF_INET;
			addr_in.sin_port = htons(drv->udp_port);
			addr = (struct sockaddr *) &addr_in;
			alen = sizeof(addr_in);
		} else {
			os_memset(&addr_un, 0, sizeof(addr_un));
			addr_un.sun_family = AF_UNIX;
			os_strlcpy(addr_un.sun_path, drv->own_socket_path,
				   sizeof(addr_un.sun_path));
			addr = (struct sockaddr *) &addr_un;
			alen = sizeof(addr_un);
		}
		if (bind(drv->test_socket, addr, alen) < 0) {
			perror("bind(PF_UNIX)");
			close(drv->test_socket);
			if (drv->own_socket_path)
				unlink(drv->own_socket_path);
			wpa_driver_test_deinit(drv);
			return NULL;
		}
		eloop_register_read_sock(drv->test_socket,
					 test_driver_receive_unix, drv, NULL);
	} else
		drv->test_socket = -1;

	return drv;
}


static void wpa_driver_test_poll(void *eloop_ctx, void *timeout_ctx)
{
	struct wpa_driver_test_data *drv = eloop_ctx;

#ifdef DRIVER_TEST_UNIX
	if (drv->associated && drv->hostapd_addr_set) {
		struct stat st;
		if (stat(drv->hostapd_addr.sun_path, &st) < 0) {
			wpa_printf(MSG_DEBUG, "%s: lost connection to AP: %s",
				   __func__, strerror(errno));
			drv->associated = 0;
			wpa_supplicant_event(drv->ctx, EVENT_DISASSOC, NULL);
		}
	}
#endif /* DRIVER_TEST_UNIX */

	eloop_register_timeout(1, 0, wpa_driver_test_poll, drv, NULL);
}


static int wpa_driver_test_set_wpa(void *priv, int enabled)
{
	wpa_printf(MSG_DEBUG, "%s: enabled=%d", __func__, enabled);
	return 0;
}


static void wpa_driver_test_scan_timeout(void *eloop_ctx, void *timeout_ctx)
{
	wpa_printf(MSG_DEBUG, "Scan timeout - try to get results");
	wpa_supplicant_event(timeout_ctx, EVENT_SCAN_RESULTS, NULL);
}


#ifdef DRIVER_TEST_UNIX
static void wpa_driver_scan_dir(struct wpa_driver_test_data *drv,
				const char *path)
{
	struct dirent *dent;
	DIR *dir;
	struct sockaddr_un addr;
	char cmd[512], *pos, *end;
	int ret;

	dir = opendir(path);
	if (dir == NULL)
		return;

	end = cmd + sizeof(cmd);
	pos = cmd;
	ret = os_snprintf(pos, end - pos, "SCAN " MACSTR,
			  MAC2STR(drv->own_addr));
	if (ret >= 0 && ret < end - pos)
		pos += ret;
	if (drv->probe_req_ie) {
		ret = os_snprintf(pos, end - pos, " ");
		if (ret >= 0 && ret < end - pos)
			pos += ret;
		pos += wpa_snprintf_hex(pos, end - pos, drv->probe_req_ie,
					drv->probe_req_ie_len);
	}
	end[-1] = '\0';

	while ((dent = readdir(dir))) {
		if (os_strncmp(dent->d_name, "AP-", 3) != 0 &&
		    os_strncmp(dent->d_name, "STA-", 4) != 0)
			continue;
		if (drv->own_socket_path) {
			size_t olen, dlen;
			olen = os_strlen(drv->own_socket_path);
			dlen = os_strlen(dent->d_name);
			if (olen >= dlen &&
			    os_strcmp(dent->d_name,
				      drv->own_socket_path + olen - dlen) == 0)
				continue;
		}
		wpa_printf(MSG_DEBUG, "%s: SCAN %s", __func__, dent->d_name);

		os_memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		os_snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/%s",
			    path, dent->d_name);

		if (sendto(drv->test_socket, cmd, os_strlen(cmd), 0,
			   (struct sockaddr *) &addr, sizeof(addr)) < 0) {
			perror("sendto(test_socket)");
		}
	}
	closedir(dir);
}
#endif /* DRIVER_TEST_UNIX */


static int wpa_driver_test_scan(void *priv,
				struct wpa_driver_scan_params *params)
{
	struct wpa_driver_test_data *drv = priv;
	size_t i;

	wpa_printf(MSG_DEBUG, "%s: priv=%p", __func__, priv);
	for (i = 0; i < params->num_ssids; i++)
		wpa_hexdump(MSG_DEBUG, "Scan SSID",
			    params->ssids[i].ssid, params->ssids[i].ssid_len);
	wpa_hexdump(MSG_DEBUG, "Scan extra IE(s)",
		    params->extra_ies, params->extra_ies_len);

	drv->num_scanres = 0;

#ifdef DRIVER_TEST_UNIX
	if (drv->test_socket >= 0 && drv->test_dir)
		wpa_driver_scan_dir(drv, drv->test_dir);

	if (drv->test_socket >= 0 && drv->hostapd_addr_set &&
	    sendto(drv->test_socket, "SCAN", 4, 0,
		   (struct sockaddr *) &drv->hostapd_addr,
		   sizeof(drv->hostapd_addr)) < 0) {
		perror("sendto(test_socket)");
	}
#endif /* DRIVER_TEST_UNIX */

	if (drv->test_socket >= 0 && drv->hostapd_addr_udp_set &&
	    sendto(drv->test_socket, "SCAN", 4, 0,
		   (struct sockaddr *) &drv->hostapd_addr_udp,
		   sizeof(drv->hostapd_addr_udp)) < 0) {
		perror("sendto(test_socket)");
	}

	eloop_cancel_timeout(wpa_driver_test_scan_timeout, drv, drv->ctx);
	eloop_register_timeout(1, 0, wpa_driver_test_scan_timeout, drv,
			       drv->ctx);
	return 0;
}


static struct wpa_scan_results * wpa_driver_test_get_scan_results2(void *priv)
{
	struct wpa_driver_test_data *drv = priv;
	struct wpa_scan_results *res;
	size_t i;

	res = os_zalloc(sizeof(*res));
	if (res == NULL)
		return NULL;

	res->res = os_zalloc(drv->num_scanres * sizeof(struct wpa_scan_res *));
	if (res->res == NULL) {
		os_free(res);
		return NULL;
	}

	for (i = 0; i < drv->num_scanres; i++) {
		struct wpa_scan_res *r;
		if (drv->scanres[i] == NULL)
			continue;
		r = os_malloc(sizeof(*r) + drv->scanres[i]->ie_len);
		if (r == NULL)
			break;
		os_memcpy(r, drv->scanres[i],
			  sizeof(*r) + drv->scanres[i]->ie_len);
		res->res[res->num++] = r;
	}

	return res;
}


static int wpa_driver_test_set_key(void *priv, wpa_alg alg, const u8 *addr,
				   int key_idx, int set_tx,
				   const u8 *seq, size_t seq_len,
				   const u8 *key, size_t key_len)
{
	wpa_printf(MSG_DEBUG, "%s: priv=%p alg=%d key_idx=%d set_tx=%d",
		   __func__, priv, alg, key_idx, set_tx);
	if (addr) {
		wpa_printf(MSG_DEBUG, "   addr=" MACSTR, MAC2STR(addr));
	}
	if (seq) {
		wpa_hexdump(MSG_DEBUG, "   seq", seq, seq_len);
	}
	if (key) {
		wpa_hexdump(MSG_DEBUG, "   key", key, key_len);
	}
	return 0;
}


static int wpa_driver_update_mode(struct wpa_driver_test_data *drv, int ap)
{
	if (ap && !drv->ap) {
		wpa_driver_test_close_test_socket(drv);
		wpa_driver_test_attach(drv, drv->test_dir, 1);
		drv->ap = 1;
	} else if (!ap && drv->ap) {
		wpa_driver_test_close_test_socket(drv);
		wpa_driver_test_attach(drv, drv->test_dir, 0);
		drv->ap = 0;
	}

	return 0;
}


static int wpa_driver_test_associate(
	void *priv, struct wpa_driver_associate_params *params)
{
	struct wpa_driver_test_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s: priv=%p freq=%d pairwise_suite=%d "
		   "group_suite=%d key_mgmt_suite=%d auth_alg=%d mode=%d",
		   __func__, priv, params->freq, params->pairwise_suite,
		   params->group_suite, params->key_mgmt_suite,
		   params->auth_alg, params->mode);
	if (params->bssid) {
		wpa_printf(MSG_DEBUG, "   bssid=" MACSTR,
			   MAC2STR(params->bssid));
	}
	if (params->ssid) {
		wpa_hexdump_ascii(MSG_DEBUG, "   ssid",
				  params->ssid, params->ssid_len);
	}
	if (params->wpa_ie) {
		wpa_hexdump(MSG_DEBUG, "   wpa_ie",
			    params->wpa_ie, params->wpa_ie_len);
		drv->assoc_wpa_ie_len = params->wpa_ie_len;
		if (drv->assoc_wpa_ie_len > sizeof(drv->assoc_wpa_ie))
			drv->assoc_wpa_ie_len = sizeof(drv->assoc_wpa_ie);
		os_memcpy(drv->assoc_wpa_ie, params->wpa_ie,
			  drv->assoc_wpa_ie_len);
	} else
		drv->assoc_wpa_ie_len = 0;

	wpa_driver_update_mode(drv, params->mode == IEEE80211_MODE_AP);

	drv->ibss = params->mode == IEEE80211_MODE_IBSS;
	drv->privacy = params->key_mgmt_suite &
		(WPA_KEY_MGMT_IEEE8021X |
		 WPA_KEY_MGMT_PSK |
		 WPA_KEY_MGMT_WPA_NONE |
		 WPA_KEY_MGMT_FT_IEEE8021X |
		 WPA_KEY_MGMT_FT_PSK |
		 WPA_KEY_MGMT_IEEE8021X_SHA256 |
		 WPA_KEY_MGMT_PSK_SHA256);
	if (params->wep_key_len[params->wep_tx_keyidx])
		drv->privacy = 1;

#ifdef DRIVER_TEST_UNIX
	if (drv->test_dir && params->bssid &&
	    params->mode != IEEE80211_MODE_IBSS) {
		os_memset(&drv->hostapd_addr, 0, sizeof(drv->hostapd_addr));
		drv->hostapd_addr.sun_family = AF_UNIX;
		os_snprintf(drv->hostapd_addr.sun_path,
			    sizeof(drv->hostapd_addr.sun_path),
			    "%s/AP-" MACSTR,
			    drv->test_dir, MAC2STR(params->bssid));
		drv->hostapd_addr_set = 1;
	}
#endif /* DRIVER_TEST_UNIX */

	if (params->mode == IEEE80211_MODE_AP) {
		struct test_driver_bss *bss;
		os_memcpy(drv->ssid, params->ssid, params->ssid_len);
		drv->ssid_len = params->ssid_len;

		test_driver_free_bsses(drv);
		bss = drv->bss = os_zalloc(sizeof(*drv->bss));
		if (bss == NULL)
			return -1;
		os_memcpy(bss->bssid, drv->own_addr, ETH_ALEN);
		os_memcpy(bss->ssid, params->ssid, params->ssid_len);
		bss->ssid_len = params->ssid_len;
		bss->privacy = drv->privacy;
		if (params->wpa_ie && params->wpa_ie_len) {
			bss->ie = os_malloc(params->wpa_ie_len);
			if (bss->ie) {
				os_memcpy(bss->ie, params->wpa_ie,
					  params->wpa_ie_len);
				bss->ielen = params->wpa_ie_len;
			}
		}
	} else if (drv->test_socket >= 0 &&
		   (drv->hostapd_addr_set || drv->hostapd_addr_udp_set)) {
		char cmd[200], *pos, *end;
		int ret;
		end = cmd + sizeof(cmd);
		pos = cmd;
		ret = os_snprintf(pos, end - pos, "ASSOC " MACSTR " ",
				  MAC2STR(drv->own_addr));
		if (ret >= 0 && ret < end - pos)
			pos += ret;
		pos += wpa_snprintf_hex(pos, end - pos, params->ssid,
					params->ssid_len);
		ret = os_snprintf(pos, end - pos, " ");
		if (ret >= 0 && ret < end - pos)
			pos += ret;
		pos += wpa_snprintf_hex(pos, end - pos, params->wpa_ie,
					params->wpa_ie_len);
		end[-1] = '\0';
#ifdef DRIVER_TEST_UNIX
		if (drv->hostapd_addr_set &&
		    sendto(drv->test_socket, cmd, os_strlen(cmd), 0,
			   (struct sockaddr *) &drv->hostapd_addr,
			   sizeof(drv->hostapd_addr)) < 0) {
			perror("sendto(test_socket)");
			return -1;
		}
#endif /* DRIVER_TEST_UNIX */
		if (drv->hostapd_addr_udp_set &&
		    sendto(drv->test_socket, cmd, os_strlen(cmd), 0,
			   (struct sockaddr *) &drv->hostapd_addr_udp,
			   sizeof(drv->hostapd_addr_udp)) < 0) {
			perror("sendto(test_socket)");
			return -1;
		}

		os_memcpy(drv->ssid, params->ssid, params->ssid_len);
		drv->ssid_len = params->ssid_len;
	} else {
		drv->associated = 1;
		if (params->mode == IEEE80211_MODE_IBSS) {
			os_memcpy(drv->ssid, params->ssid, params->ssid_len);
			drv->ssid_len = params->ssid_len;
			if (params->bssid)
				os_memcpy(drv->bssid, params->bssid, ETH_ALEN);
			else {
				os_get_random(drv->bssid, ETH_ALEN);
				drv->bssid[0] &= ~0x01;
				drv->bssid[0] |= 0x02;
			}
		}
		wpa_supplicant_event(drv->ctx, EVENT_ASSOC, NULL);
	}

	return 0;
}


static int wpa_driver_test_get_bssid(void *priv, u8 *bssid)
{
	struct wpa_driver_test_data *drv = priv;
	os_memcpy(bssid, drv->bssid, ETH_ALEN);
	return 0;
}


static int wpa_driver_test_get_ssid(void *priv, u8 *ssid)
{
	struct wpa_driver_test_data *drv = priv;
	os_memcpy(ssid, drv->ssid, 32);
	return drv->ssid_len;
}


static int wpa_driver_test_send_disassoc(struct wpa_driver_test_data *drv)
{
#ifdef DRIVER_TEST_UNIX
	if (drv->test_socket >= 0 &&
	    sendto(drv->test_socket, "DISASSOC", 8, 0,
		   (struct sockaddr *) &drv->hostapd_addr,
		   sizeof(drv->hostapd_addr)) < 0) {
		perror("sendto(test_socket)");
		return -1;
	}
#endif /* DRIVER_TEST_UNIX */
	if (drv->test_socket >= 0 && drv->hostapd_addr_udp_set &&
	    sendto(drv->test_socket, "DISASSOC", 8, 0,
		   (struct sockaddr *) &drv->hostapd_addr_udp,
		   sizeof(drv->hostapd_addr_udp)) < 0) {
		perror("sendto(test_socket)");
		return -1;
	}
	return 0;
}


static int wpa_driver_test_deauthenticate(void *priv, const u8 *addr,
					  int reason_code)
{
	struct wpa_driver_test_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s addr=" MACSTR " reason_code=%d",
		   __func__, MAC2STR(addr), reason_code);
	os_memset(drv->bssid, 0, ETH_ALEN);
	drv->associated = 0;
	wpa_supplicant_event(drv->ctx, EVENT_DISASSOC, NULL);
	return wpa_driver_test_send_disassoc(drv);
}


static int wpa_driver_test_disassociate(void *priv, const u8 *addr,
					int reason_code)
{
	struct wpa_driver_test_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s addr=" MACSTR " reason_code=%d",
		   __func__, MAC2STR(addr), reason_code);
	os_memset(drv->bssid, 0, ETH_ALEN);
	drv->associated = 0;
	wpa_supplicant_event(drv->ctx, EVENT_DISASSOC, NULL);
	return wpa_driver_test_send_disassoc(drv);
}


static void wpa_driver_test_scanresp(struct wpa_driver_test_data *drv,
				     struct sockaddr *from,
				     socklen_t fromlen,
				     const char *data)
{
	struct wpa_scan_res *res;
	const char *pos, *pos2;
	size_t len;
	u8 *ie_pos, *ie_start, *ie_end;
#define MAX_IE_LEN 1000

	wpa_printf(MSG_DEBUG, "test_driver: SCANRESP %s", data);
	if (drv->num_scanres >= MAX_SCAN_RESULTS) {
		wpa_printf(MSG_DEBUG, "test_driver: No room for the new scan "
			   "result");
		return;
	}

	/* SCANRESP BSSID SSID IEs */

	res = os_zalloc(sizeof(*res) + MAX_IE_LEN);
	if (res == NULL)
		return;
	ie_start = ie_pos = (u8 *) (res + 1);
	ie_end = ie_pos + MAX_IE_LEN;

	if (hwaddr_aton(data, res->bssid)) {
		wpa_printf(MSG_DEBUG, "test_driver: invalid BSSID in scanres");
		os_free(res);
		return;
	}

	pos = data + 17;
	while (*pos == ' ')
		pos++;
	pos2 = os_strchr(pos, ' ');
	if (pos2 == NULL) {
		wpa_printf(MSG_DEBUG, "test_driver: invalid SSID termination "
			   "in scanres");
		os_free(res);
		return;
	}
	len = (pos2 - pos) / 2;
	if (len > 32)
		len = 32;
	/*
	 * Generate SSID IE from the SSID field since this IE is not included
	 * in the main IE field.
	 */
	*ie_pos++ = WLAN_EID_SSID;
	*ie_pos++ = len;
	if (hexstr2bin(pos, ie_pos, len) < 0) {
		wpa_printf(MSG_DEBUG, "test_driver: invalid SSID in scanres");
		os_free(res);
		return;
	}
	ie_pos += len;

	pos = pos2 + 1;
	pos2 = os_strchr(pos, ' ');
	if (pos2 == NULL)
		len = os_strlen(pos) / 2;
	else
		len = (pos2 - pos) / 2;
	if ((int) len > ie_end - ie_pos)
		len = ie_end - ie_pos;
	if (hexstr2bin(pos, ie_pos, len) < 0) {
		wpa_printf(MSG_DEBUG, "test_driver: invalid IEs in scanres");
		os_free(res);
		return;
	}
	ie_pos += len;
	res->ie_len = ie_pos - ie_start;

	if (pos2) {
		pos = pos2 + 1;
		while (*pos == ' ')
			pos++;
		if (os_strstr(pos, "PRIVACY"))
			res->caps |= IEEE80211_CAP_PRIVACY;
		if (os_strstr(pos, "IBSS"))
			res->caps |= IEEE80211_CAP_IBSS;
	}

	os_free(drv->scanres[drv->num_scanres]);
	drv->scanres[drv->num_scanres++] = res;
}


static void wpa_driver_test_assocresp(struct wpa_driver_test_data *drv,
				      struct sockaddr *from,
				      socklen_t fromlen,
				      const char *data)
{
	/* ASSOCRESP BSSID <res> */
	if (hwaddr_aton(data, drv->bssid)) {
		wpa_printf(MSG_DEBUG, "test_driver: invalid BSSID in "
			   "assocresp");
	}
	if (drv->use_associnfo) {
		union wpa_event_data event;
		os_memset(&event, 0, sizeof(event));
		event.assoc_info.req_ies = drv->assoc_wpa_ie;
		event.assoc_info.req_ies_len = drv->assoc_wpa_ie_len;
		wpa_supplicant_event(drv->ctx, EVENT_ASSOCINFO, &event);
	}
	drv->associated = 1;
	wpa_supplicant_event(drv->ctx, EVENT_ASSOC, NULL);
}


static void wpa_driver_test_disassoc(struct wpa_driver_test_data *drv,
				     struct sockaddr *from,
				     socklen_t fromlen)
{
	drv->associated = 0;
	wpa_supplicant_event(drv->ctx, EVENT_DISASSOC, NULL);
}


static void wpa_driver_test_eapol(struct wpa_driver_test_data *drv,
				  struct sockaddr *from,
				  socklen_t fromlen,
				  const u8 *data, size_t data_len)
{
	const u8 *src = drv->bssid;

	if (data_len > 14) {
		/* Skip Ethernet header */
		src = data + ETH_ALEN;
		data += 14;
		data_len -= 14;
	}
#ifndef HOSTAPD
	wpa_supplicant_rx_eapol(drv->ctx, src, data, data_len);
#endif /* HOSTAPD */
}


static void wpa_driver_test_mlme(struct wpa_driver_test_data *drv,
				 struct sockaddr *from,
				 socklen_t fromlen,
				 const u8 *data, size_t data_len)
{
#ifdef CONFIG_CLIENT_MLME
	struct ieee80211_rx_status rx_status;
	os_memset(&rx_status, 0, sizeof(rx_status));
	wpa_supplicant_sta_rx(drv->ctx, data, data_len, &rx_status);
#endif /* CONFIG_CLIENT_MLME */
}


static void wpa_driver_test_scan_cmd(struct wpa_driver_test_data *drv,
				     struct sockaddr *from,
				     socklen_t fromlen,
				     const u8 *data, size_t data_len)
{
	char buf[512], *pos, *end;
	int ret;

	/* data: optional [ STA-addr | ' ' | IEs(hex) ] */

	if (!drv->ibss)
		return;

	pos = buf;
	end = buf + sizeof(buf);

	/* reply: SCANRESP BSSID SSID IEs */
	ret = snprintf(pos, end - pos, "SCANRESP " MACSTR " ",
		       MAC2STR(drv->bssid));
	if (ret < 0 || ret >= end - pos)
		return;
	pos += ret;
	pos += wpa_snprintf_hex(pos, end - pos,
				drv->ssid, drv->ssid_len);
	ret = snprintf(pos, end - pos, " ");
	if (ret < 0 || ret >= end - pos)
		return;
	pos += ret;
	pos += wpa_snprintf_hex(pos, end - pos, drv->assoc_wpa_ie,
				drv->assoc_wpa_ie_len);

	if (drv->privacy) {
		ret = snprintf(pos, end - pos, " PRIVACY");
		if (ret < 0 || ret >= end - pos)
			return;
		pos += ret;
	}

	ret = snprintf(pos, end - pos, " IBSS");
	if (ret < 0 || ret >= end - pos)
		return;
	pos += ret;

	sendto(drv->test_socket, buf, pos - buf, 0,
	       (struct sockaddr *) from, fromlen);
}


static void wpa_driver_test_receive_unix(int sock, void *eloop_ctx,
					 void *sock_ctx)
{
	struct wpa_driver_test_data *drv = eloop_ctx;
	char *buf;
	int res;
	struct sockaddr_storage from;
	socklen_t fromlen = sizeof(from);
	const size_t buflen = 2000;

	if (drv->ap) {
		test_driver_receive_unix(sock, eloop_ctx, sock_ctx);
		return;
	}

	buf = os_malloc(buflen);
	if (buf == NULL)
		return;
	res = recvfrom(sock, buf, buflen - 1, 0,
		       (struct sockaddr *) &from, &fromlen);
	if (res < 0) {
		perror("recvfrom(test_socket)");
		os_free(buf);
		return;
	}
	buf[res] = '\0';

	wpa_printf(MSG_DEBUG, "test_driver: received %u bytes", res);

	if (os_strncmp(buf, "SCANRESP ", 9) == 0) {
		wpa_driver_test_scanresp(drv, (struct sockaddr *) &from,
					 fromlen, buf + 9);
	} else if (os_strncmp(buf, "ASSOCRESP ", 10) == 0) {
		wpa_driver_test_assocresp(drv, (struct sockaddr *) &from,
					  fromlen, buf + 10);
	} else if (os_strcmp(buf, "DISASSOC") == 0) {
		wpa_driver_test_disassoc(drv, (struct sockaddr *) &from,
					 fromlen);
	} else if (os_strcmp(buf, "DEAUTH") == 0) {
		wpa_driver_test_disassoc(drv, (struct sockaddr *) &from,
					 fromlen);
	} else if (os_strncmp(buf, "EAPOL ", 6) == 0) {
		wpa_driver_test_eapol(drv, (struct sockaddr *) &from, fromlen,
				      (const u8 *) buf + 6, res - 6);
	} else if (os_strncmp(buf, "MLME ", 5) == 0) {
		wpa_driver_test_mlme(drv, (struct sockaddr *) &from, fromlen,
				     (const u8 *) buf + 5, res - 5);
	} else if (os_strncmp(buf, "SCAN ", 5) == 0) {
		wpa_driver_test_scan_cmd(drv, (struct sockaddr *) &from,
					 fromlen,
					 (const u8 *) buf + 5, res - 5);
	} else {
		wpa_hexdump_ascii(MSG_DEBUG, "Unknown test_socket command",
				  (u8 *) buf, res);
	}
	os_free(buf);
}


static void * wpa_driver_test_init2(void *ctx, const char *ifname,
				    void *global_priv)
{
	struct wpa_driver_test_data *drv;

	drv = os_zalloc(sizeof(*drv));
	if (drv == NULL)
		return NULL;
	drv->global = global_priv;
	drv->ctx = ctx;
	drv->test_socket = -1;

	/* Set dummy BSSID and SSID for testing. */
	drv->bssid[0] = 0x02;
	drv->bssid[1] = 0x00;
	drv->bssid[2] = 0x00;
	drv->bssid[3] = 0x00;
	drv->bssid[4] = 0x00;
	drv->bssid[5] = 0x01;
	os_memcpy(drv->ssid, "test", 5);
	drv->ssid_len = 4;

	/* Generate a MAC address to help testing with multiple STAs */
	drv->own_addr[0] = 0x02; /* locally administered */
	sha1_prf((const u8 *) ifname, os_strlen(ifname),
		 "wpa_supplicant test mac addr generation",
		 NULL, 0, drv->own_addr + 1, ETH_ALEN - 1);
	eloop_register_timeout(1, 0, wpa_driver_test_poll, drv, NULL);

	return drv;
}


static void wpa_driver_test_close_test_socket(struct wpa_driver_test_data *drv)
{
	if (drv->test_socket >= 0) {
		eloop_unregister_read_sock(drv->test_socket);
		close(drv->test_socket);
		drv->test_socket = -1;
	}

	if (drv->own_socket_path) {
		unlink(drv->own_socket_path);
		os_free(drv->own_socket_path);
		drv->own_socket_path = NULL;
	}
}


static void wpa_driver_test_deinit(void *priv)
{
	struct wpa_driver_test_data *drv = priv;
	struct test_client_socket *cli, *prev;
	int i;

	cli = drv->cli;
	while (cli) {
		prev = cli;
		cli = cli->next;
		os_free(prev);
	}

#ifdef HOSTAPD
	/* There should be only one BSS remaining at this point. */
	if (drv->bss == NULL)
		wpa_printf(MSG_ERROR, "%s: drv->bss == NULL", __func__);
	else if (drv->bss->next)
		wpa_printf(MSG_ERROR, "%s: drv->bss->next != NULL", __func__);
#endif /* HOSTAPD */

	test_driver_free_bsses(drv);

	wpa_driver_test_close_test_socket(drv);
	eloop_cancel_timeout(wpa_driver_test_scan_timeout, drv, drv->ctx);
	eloop_cancel_timeout(wpa_driver_test_poll, drv, NULL);
	os_free(drv->test_dir);
	for (i = 0; i < MAX_SCAN_RESULTS; i++)
		os_free(drv->scanres[i]);
	os_free(drv->probe_req_ie);
	os_free(drv);
}


static int wpa_driver_test_attach(struct wpa_driver_test_data *drv,
				  const char *dir, int ap)
{
#ifdef DRIVER_TEST_UNIX
	static unsigned int counter = 0;
	struct sockaddr_un addr;
	size_t len;

	os_free(drv->own_socket_path);
	if (dir) {
		len = os_strlen(dir) + 30;
		drv->own_socket_path = os_malloc(len);
		if (drv->own_socket_path == NULL)
			return -1;
		os_snprintf(drv->own_socket_path, len, "%s/%s-" MACSTR,
			    dir, ap ? "AP" : "STA", MAC2STR(drv->own_addr));
	} else {
		drv->own_socket_path = os_malloc(100);
		if (drv->own_socket_path == NULL)
			return -1;
		os_snprintf(drv->own_socket_path, 100,
			    "/tmp/wpa_supplicant_test-%d-%d",
			    getpid(), counter++);
	}

	drv->test_socket = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (drv->test_socket < 0) {
		perror("socket(PF_UNIX)");
		os_free(drv->own_socket_path);
		drv->own_socket_path = NULL;
		return -1;
	}

	os_memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	os_strlcpy(addr.sun_path, drv->own_socket_path, sizeof(addr.sun_path));
	if (bind(drv->test_socket, (struct sockaddr *) &addr,
		 sizeof(addr)) < 0) {
		perror("bind(PF_UNIX)");
		close(drv->test_socket);
		unlink(drv->own_socket_path);
		os_free(drv->own_socket_path);
		drv->own_socket_path = NULL;
		return -1;
	}

	eloop_register_read_sock(drv->test_socket,
				 wpa_driver_test_receive_unix, drv, NULL);

	return 0;
#else /* DRIVER_TEST_UNIX */
	return -1;
#endif /* DRIVER_TEST_UNIX */
}


static int wpa_driver_test_attach_udp(struct wpa_driver_test_data *drv,
				      char *dst)
{
	char *pos;

	pos = os_strchr(dst, ':');
	if (pos == NULL)
		return -1;
	*pos++ = '\0';
	wpa_printf(MSG_DEBUG, "%s: addr=%s port=%s", __func__, dst, pos);

	drv->test_socket = socket(PF_INET, SOCK_DGRAM, 0);
	if (drv->test_socket < 0) {
		perror("socket(PF_INET)");
		return -1;
	}

	os_memset(&drv->hostapd_addr_udp, 0, sizeof(drv->hostapd_addr_udp));
	drv->hostapd_addr_udp.sin_family = AF_INET;
#if defined(CONFIG_NATIVE_WINDOWS) || defined(CONFIG_ANSI_C_EXTRA)
	{
		int a[4];
		u8 *pos;
		sscanf(dst, "%d.%d.%d.%d", &a[0], &a[1], &a[2], &a[3]);
		pos = (u8 *) &drv->hostapd_addr_udp.sin_addr;
		*pos++ = a[0];
		*pos++ = a[1];
		*pos++ = a[2];
		*pos++ = a[3];
	}
#else /* CONFIG_NATIVE_WINDOWS or CONFIG_ANSI_C_EXTRA */
	inet_aton(dst, &drv->hostapd_addr_udp.sin_addr);
#endif /* CONFIG_NATIVE_WINDOWS or CONFIG_ANSI_C_EXTRA */
	drv->hostapd_addr_udp.sin_port = htons(atoi(pos));

	drv->hostapd_addr_udp_set = 1;

	eloop_register_read_sock(drv->test_socket,
				 wpa_driver_test_receive_unix, drv, NULL);

	return 0;
}


static int wpa_driver_test_set_param(void *priv, const char *param)
{
	struct wpa_driver_test_data *drv = priv;
	const char *pos;

	wpa_printf(MSG_DEBUG, "%s: param='%s'", __func__, param);
	if (param == NULL)
		return 0;

	wpa_driver_test_close_test_socket(drv);

#ifdef DRIVER_TEST_UNIX
	pos = os_strstr(param, "test_socket=");
	if (pos) {
		const char *pos2;
		size_t len;

		pos += 12;
		pos2 = os_strchr(pos, ' ');
		if (pos2)
			len = pos2 - pos;
		else
			len = os_strlen(pos);
		if (len > sizeof(drv->hostapd_addr.sun_path))
			return -1;
		os_memset(&drv->hostapd_addr, 0, sizeof(drv->hostapd_addr));
		drv->hostapd_addr.sun_family = AF_UNIX;
		os_memcpy(drv->hostapd_addr.sun_path, pos, len);
		drv->hostapd_addr_set = 1;
	}
#endif /* DRIVER_TEST_UNIX */

	pos = os_strstr(param, "test_dir=");
	if (pos) {
		char *end;
		os_free(drv->test_dir);
		drv->test_dir = os_strdup(pos + 9);
		if (drv->test_dir == NULL)
			return -1;
		end = os_strchr(drv->test_dir, ' ');
		if (end)
			*end = '\0';
		if (wpa_driver_test_attach(drv, drv->test_dir, 0))
			return -1;
	} else {
		pos = os_strstr(param, "test_udp=");
		if (pos) {
			char *dst, *epos;
			dst = os_strdup(pos + 9);
			if (dst == NULL)
				return -1;
			epos = os_strchr(dst, ' ');
			if (epos)
				*epos = '\0';
			if (wpa_driver_test_attach_udp(drv, dst))
				return -1;
			os_free(dst);
		} else if (wpa_driver_test_attach(drv, NULL, 0))
			return -1;
	}

	if (os_strstr(param, "use_associnfo=1")) {
		wpa_printf(MSG_DEBUG, "test_driver: Use AssocInfo events");
		drv->use_associnfo = 1;
	}

#ifdef CONFIG_CLIENT_MLME
	if (os_strstr(param, "use_mlme=1")) {
		wpa_printf(MSG_DEBUG, "test_driver: Use internal MLME");
		drv->use_mlme = 1;
	}
#endif /* CONFIG_CLIENT_MLME */

	return 0;
}


static const u8 * wpa_driver_test_get_mac_addr(void *priv)
{
	struct wpa_driver_test_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s", __func__);
	return drv->own_addr;
}


static int wpa_driver_test_send_eapol(void *priv, const u8 *dest, u16 proto,
				      const u8 *data, size_t data_len)
{
	struct wpa_driver_test_data *drv = priv;
	char *msg;
	size_t msg_len;
	struct l2_ethhdr eth;
	struct sockaddr *addr;
	socklen_t alen;
#ifdef DRIVER_TEST_UNIX
	struct sockaddr_un addr_un;
#endif /* DRIVER_TEST_UNIX */

	wpa_hexdump(MSG_MSGDUMP, "test_send_eapol TX frame", data, data_len);

	os_memset(&eth, 0, sizeof(eth));
	os_memcpy(eth.h_dest, dest, ETH_ALEN);
	os_memcpy(eth.h_source, drv->own_addr, ETH_ALEN);
	eth.h_proto = host_to_be16(proto);

	msg_len = 6 + sizeof(eth) + data_len;
	msg = os_malloc(msg_len);
	if (msg == NULL)
		return -1;
	os_memcpy(msg, "EAPOL ", 6);
	os_memcpy(msg + 6, &eth, sizeof(eth));
	os_memcpy(msg + 6 + sizeof(eth), data, data_len);

	if (os_memcmp(dest, drv->bssid, ETH_ALEN) == 0 ||
	    drv->test_dir == NULL) {
		if (drv->hostapd_addr_udp_set) {
			addr = (struct sockaddr *) &drv->hostapd_addr_udp;
			alen = sizeof(drv->hostapd_addr_udp);
		} else {
#ifdef DRIVER_TEST_UNIX
			addr = (struct sockaddr *) &drv->hostapd_addr;
			alen = sizeof(drv->hostapd_addr);
#else /* DRIVER_TEST_UNIX */
			os_free(msg);
			return -1;
#endif /* DRIVER_TEST_UNIX */
		}
	} else {
#ifdef DRIVER_TEST_UNIX
		struct stat st;
		os_memset(&addr_un, 0, sizeof(addr_un));
		addr_un.sun_family = AF_UNIX;
		os_snprintf(addr_un.sun_path, sizeof(addr_un.sun_path),
			    "%s/STA-" MACSTR, drv->test_dir, MAC2STR(dest));
		if (stat(addr_un.sun_path, &st) < 0) {
			os_snprintf(addr_un.sun_path, sizeof(addr_un.sun_path),
				    "%s/AP-" MACSTR,
				    drv->test_dir, MAC2STR(dest));
		}
		addr = (struct sockaddr *) &addr_un;
		alen = sizeof(addr_un);
#else /* DRIVER_TEST_UNIX */
		os_free(msg);
		return -1;
#endif /* DRIVER_TEST_UNIX */
	}

	if (sendto(drv->test_socket, msg, msg_len, 0, addr, alen) < 0) {
		perror("sendmsg(test_socket)");
		os_free(msg);
		return -1;
	}

	os_free(msg);
	return 0;
}


static int wpa_driver_test_get_capa(void *priv, struct wpa_driver_capa *capa)
{
	struct wpa_driver_test_data *drv = priv;
	os_memset(capa, 0, sizeof(*capa));
	capa->key_mgmt = WPA_DRIVER_CAPA_KEY_MGMT_WPA |
		WPA_DRIVER_CAPA_KEY_MGMT_WPA2 |
		WPA_DRIVER_CAPA_KEY_MGMT_WPA_PSK |
		WPA_DRIVER_CAPA_KEY_MGMT_WPA2_PSK |
		WPA_DRIVER_CAPA_KEY_MGMT_WPA_NONE |
		WPA_DRIVER_CAPA_KEY_MGMT_FT |
		WPA_DRIVER_CAPA_KEY_MGMT_FT_PSK;
	capa->enc = WPA_DRIVER_CAPA_ENC_WEP40 |
		WPA_DRIVER_CAPA_ENC_WEP104 |
		WPA_DRIVER_CAPA_ENC_TKIP |
		WPA_DRIVER_CAPA_ENC_CCMP;
	capa->auth = WPA_DRIVER_AUTH_OPEN |
		WPA_DRIVER_AUTH_SHARED |
		WPA_DRIVER_AUTH_LEAP;
	if (drv->use_mlme)
		capa->flags |= WPA_DRIVER_FLAGS_USER_SPACE_MLME;
	capa->flags |= WPA_DRIVER_FLAGS_AP;
	capa->max_scan_ssids = 2;

	return 0;
}


static int wpa_driver_test_mlme_setprotection(void *priv, const u8 *addr,
					      int protect_type,
					      int key_type)
{
	wpa_printf(MSG_DEBUG, "%s: protect_type=%d key_type=%d",
		   __func__, protect_type, key_type);

	if (addr) {
		wpa_printf(MSG_DEBUG, "%s: addr=" MACSTR,
			   __func__, MAC2STR(addr));
	}

	return 0;
}


static int wpa_driver_test_set_channel(void *priv, hostapd_hw_mode phymode,
				       int chan, int freq)
{
	wpa_printf(MSG_DEBUG, "%s: phymode=%d chan=%d freq=%d",
		   __func__, phymode, chan, freq);
	return 0;
}


static int wpa_driver_test_mlme_add_sta(void *priv, const u8 *addr,
					const u8 *supp_rates,
					size_t supp_rates_len)
{
	wpa_printf(MSG_DEBUG, "%s: addr=" MACSTR, __func__, MAC2STR(addr));
	return 0;
}


static int wpa_driver_test_mlme_remove_sta(void *priv, const u8 *addr)
{
	wpa_printf(MSG_DEBUG, "%s: addr=" MACSTR, __func__, MAC2STR(addr));
	return 0;
}


static int wpa_driver_test_set_ssid(void *priv, const u8 *ssid,
				    size_t ssid_len)
{
	wpa_printf(MSG_DEBUG, "%s", __func__);
	return 0;
}


static int wpa_driver_test_set_bssid(void *priv, const u8 *bssid)
{
	wpa_printf(MSG_DEBUG, "%s: bssid=" MACSTR, __func__, MAC2STR(bssid));
	return 0;
}


static int wpa_driver_test_set_probe_req_ie(void *priv, const u8 *ies,
					    size_t ies_len)
{
	struct wpa_driver_test_data *drv = priv;

	os_free(drv->probe_req_ie);
	if (ies) {
		drv->probe_req_ie = os_malloc(ies_len);
		if (drv->probe_req_ie == NULL) {
			drv->probe_req_ie_len = 0;
			return -1;
		}
		os_memcpy(drv->probe_req_ie, ies, ies_len);
		drv->probe_req_ie_len = ies_len;
	} else {
		drv->probe_req_ie = NULL;
		drv->probe_req_ie_len = 0;
	}
	return 0;
}


static void * wpa_driver_test_global_init(void)
{
	struct wpa_driver_test_global *global;

	global = os_zalloc(sizeof(*global));
	return global;
}


static void wpa_driver_test_global_deinit(void *priv)
{
	struct wpa_driver_test_global *global = priv;
	os_free(global);
}


static struct wpa_interface_info *
wpa_driver_test_get_interfaces(void *global_priv)
{
	/* struct wpa_driver_test_global *global = priv; */
	struct wpa_interface_info *iface;

	iface = os_zalloc(sizeof(*iface));
	if (iface == NULL)
		return iface;
	iface->ifname = os_strdup("sta0");
	iface->desc = os_strdup("test interface 0");
	iface->drv_name = "test";
	iface->next = os_zalloc(sizeof(*iface));
	if (iface->next) {
		iface->next->ifname = os_strdup("sta1");
		iface->next->desc = os_strdup("test interface 1");
		iface->next->drv_name = "test";
	}

	return iface;
}


static struct hostapd_hw_modes *
wpa_driver_test_get_hw_feature_data(void *priv, u16 *num_modes, u16 *flags)
{
	struct hostapd_hw_modes *modes;

	*num_modes = 3;
	*flags = 0;
	modes = os_zalloc(*num_modes * sizeof(struct hostapd_hw_modes));
	if (modes == NULL)
		return NULL;
	modes[0].mode = HOSTAPD_MODE_IEEE80211G;
	modes[0].num_channels = 1;
	modes[0].num_rates = 1;
	modes[0].channels = os_zalloc(sizeof(struct hostapd_channel_data));
	modes[0].rates = os_zalloc(sizeof(struct hostapd_rate_data));
	if (modes[0].channels == NULL || modes[0].rates == NULL)
		goto fail;
	modes[0].channels[0].chan = 1;
	modes[0].channels[0].freq = 2412;
	modes[0].channels[0].flag = 0;
	modes[0].rates[0].rate = 10;
	modes[0].rates[0].flags = HOSTAPD_RATE_BASIC | HOSTAPD_RATE_SUPPORTED |
		HOSTAPD_RATE_CCK | HOSTAPD_RATE_MANDATORY;

	modes[1].mode = HOSTAPD_MODE_IEEE80211B;
	modes[1].num_channels = 1;
	modes[1].num_rates = 1;
	modes[1].channels = os_zalloc(sizeof(struct hostapd_channel_data));
	modes[1].rates = os_zalloc(sizeof(struct hostapd_rate_data));
	if (modes[1].channels == NULL || modes[1].rates == NULL)
		goto fail;
	modes[1].channels[0].chan = 1;
	modes[1].channels[0].freq = 2412;
	modes[1].channels[0].flag = 0;
	modes[1].rates[0].rate = 10;
	modes[1].rates[0].flags = HOSTAPD_RATE_BASIC | HOSTAPD_RATE_SUPPORTED |
		HOSTAPD_RATE_CCK | HOSTAPD_RATE_MANDATORY;

	modes[2].mode = HOSTAPD_MODE_IEEE80211A;
	modes[2].num_channels = 1;
	modes[2].num_rates = 1;
	modes[2].channels = os_zalloc(sizeof(struct hostapd_channel_data));
	modes[2].rates = os_zalloc(sizeof(struct hostapd_rate_data));
	if (modes[2].channels == NULL || modes[2].rates == NULL)
		goto fail;
	modes[2].channels[0].chan = 60;
	modes[2].channels[0].freq = 5300;
	modes[2].channels[0].flag = 0;
	modes[2].rates[0].rate = 60;
	modes[2].rates[0].flags = HOSTAPD_RATE_BASIC | HOSTAPD_RATE_SUPPORTED |
		HOSTAPD_RATE_MANDATORY;

	return modes;

fail:
	if (modes) {
		size_t i;
		for (i = 0; i < *num_modes; i++) {
			os_free(modes[i].channels);
			os_free(modes[i].rates);
		}
		os_free(modes);
	}
	return NULL;
}


static int wpa_driver_test_set_mode(void *priv, int mode)
{
	struct wpa_driver_test_data *drv = priv;
	return wpa_driver_update_mode(drv, mode == IEEE80211_MODE_AP);
}


const struct wpa_driver_ops wpa_driver_test_ops = {
	"test",
	"wpa_supplicant test driver",
	.hapd_init = test_driver_init,
	.hapd_deinit = wpa_driver_test_deinit,
	.hapd_send_eapol = test_driver_send_eapol,
	.send_mlme = wpa_driver_test_send_mlme,
	.set_generic_elem = test_driver_set_generic_elem,
	.sta_deauth = test_driver_sta_deauth,
	.sta_disassoc = test_driver_sta_disassoc,
	.get_hw_feature_data = wpa_driver_test_get_hw_feature_data,
	.bss_add = test_driver_bss_add,
	.bss_remove = test_driver_bss_remove,
	.if_add = test_driver_if_add,
	.if_update = test_driver_if_update,
	.if_remove = test_driver_if_remove,
	.valid_bss_mask = test_driver_valid_bss_mask,
	.hapd_set_ssid = test_driver_set_ssid,
	.set_privacy = test_driver_set_privacy,
	.hapd_set_key = test_driver_set_key,
	.set_sta_vlan = test_driver_set_sta_vlan,
	.sta_add = test_driver_sta_add,
	.send_ether = test_driver_send_ether,
	.set_wps_beacon_ie = test_driver_set_wps_beacon_ie,
	.set_wps_probe_resp_ie = test_driver_set_wps_probe_resp_ie,
	.get_bssid = wpa_driver_test_get_bssid,
	.get_ssid = wpa_driver_test_get_ssid,
	.set_wpa = wpa_driver_test_set_wpa,
	.set_key = wpa_driver_test_set_key,
	.deinit = wpa_driver_test_deinit,
	.set_param = wpa_driver_test_set_param,
	.deauthenticate = wpa_driver_test_deauthenticate,
	.disassociate = wpa_driver_test_disassociate,
	.associate = wpa_driver_test_associate,
	.get_capa = wpa_driver_test_get_capa,
	.get_mac_addr = wpa_driver_test_get_mac_addr,
	.send_eapol = wpa_driver_test_send_eapol,
	.mlme_setprotection = wpa_driver_test_mlme_setprotection,
	.set_channel = wpa_driver_test_set_channel,
	.set_ssid = wpa_driver_test_set_ssid,
	.set_bssid = wpa_driver_test_set_bssid,
	.mlme_add_sta = wpa_driver_test_mlme_add_sta,
	.mlme_remove_sta = wpa_driver_test_mlme_remove_sta,
	.get_scan_results2 = wpa_driver_test_get_scan_results2,
	.set_probe_req_ie = wpa_driver_test_set_probe_req_ie,
	.global_init = wpa_driver_test_global_init,
	.global_deinit = wpa_driver_test_global_deinit,
	.init2 = wpa_driver_test_init2,
	.get_interfaces = wpa_driver_test_get_interfaces,
	.scan2 = wpa_driver_test_scan,
	.set_mode = wpa_driver_test_set_mode,
};
