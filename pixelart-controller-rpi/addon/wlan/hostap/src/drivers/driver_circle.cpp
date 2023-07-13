//
// driver_circle.cpp
//
// Driver interface for Circle network driver
// by R. Stange <rsta2@o2online.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// Alternatively, this software may be distributed under the terms of BSD
// license.
//

/*
 * The "struct brcmf_*_le" definitions in this file are:
 *
 * Copyright (c) 2012 Broadcom Corporation
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

extern "C" {

#include "includes.h"
#include "common.h"
#include "driver.h"
#include "eloop.h"

}

#include <circle/netdevice.h>
#include <circle/macaddress.h>
#include <circle/string.h>
#include <wlan/bcm4343.h>
#include <assert.h>

typedef u16 __le16;
typedef u32 __le32;

struct brcmf_bss_info_le
{
	__le32 version;		/* version field */
#define	BRCMF_BSS_INFO_VERSION	109 /* curr ver of brcmf_bss_info_le struct */
	__le32 length;		/* byte length of data in this record,
				 * starting at version and including IEs
				 */
	u8 BSSID[ETH_ALEN];
	__le16 beacon_period;	/* units are Kusec */
	__le16 capability;	/* Capability information */
	u8 SSID_len;
	u8 SSID[32];
	struct {
		__le32 count;	/* # rates in this set */
		u8 rates[16];	/* rates in 500kbps units w/hi bit set if basic */
	} rateset;		/* supported rates */
	__le16 chanspec;	/* chanspec for bss */
	__le16 atim_window;	/* units are Kusec */
	u8 dtim_period;		/* DTIM period */
	__le16 RSSI;		/* receive signal strength (in dBm) */
	s8 phy_noise;		/* noise (in dBm) */

	u8 n_cap;		/* BSS is 802.11N Capable */
	/* 802.11N BSS Capabilities (based on HT_CAP_*): */
	__le32 nbss_cap;
	u8 ctl_ch;		/* 802.11N BSS control channel number */
	__le32 reserved32[1];	/* Reserved for expansion of BSS properties */
	u8 flags;		/* flags */
	u8 reserved[3];		/* Reserved for expansion of BSS properties */
#define BRCMF_MCSSET_LEN		16
	u8 basic_mcs[BRCMF_MCSSET_LEN];	/* 802.11N BSS required MCS set */

	__le16 ie_offset;	/* offset at which IEs start, from beginning */
	__le32 ie_length;	/* byte length of Information Elements */
	__le16 SNR;		/* average SNR of during frame reception */
	/* Add new fields here */
	/* variable length Information Elements */
};

struct brcmf_escan_result_le
{
	__le32 buflen;
	__le32 version;
	__le16 sync_id;
	__le16 bss_count;
	struct brcmf_bss_info_le bss_info_le;
};

struct wpa_driver_circle_data
{
	void *ctx;
	CBcm4343Device *netdev;
	size_t ssid_len;
	u8 ssid[32];
	int country_set;
};

static const char countries[][3] =
{
	"AD","AE","AF","AI","AL","AM","AN","AR","AS","AT","AU","AW","AZ",
	"BA","BB","BD","BE","BF","BG","BH","BL","BM","BN","BO","BR","BS",
	"BT","BY","BZ","CA","CF","CH","CI","CL","CN","CO","CR","CU","CX",
	"CY","CZ","DE","DK","DM","DO","DZ","EC","EE","EG","ES","ET","FI",
	"FM","FR","GB","GD","GE","GF","GH","GL","GP","GR","GT","GU","GY",
	"HK","HN","HR","HT","HU","ID","IE","IL","IN","IR","IS","IT","JM",
	"JO","JP","KE","KH","KN","KP","KR","KW","KY","KZ","LB","LC","LI",
	"LK","LS","LT","LU","LV","MA","MC","MD","ME","MF","MH","MK","MN",
	"MO","MP","MQ","MR","MT","MU","MV","MW","MX","MY","NG","NI","NL",
	"NO","NP","NZ","OM","PA","PE","PF","PG","PH","PK","PL","PM","PR",
	"PT","PW","PY","QA","RE","RO","RS","RU","RW","SA","SE","SG","SI",
	"SK","SN","SR","SV","SY","TC","TD","TG","TH","TN","TR","TT","TW",
	"TZ","UA","UG","US","UY","UZ","VC","VE","VI","VN","VU","WF","WS",
	"YE","YT","ZA","ZW"
};

static int is_valid_country_code (const char *alpha2)
{
	assert (alpha2 != 0);

	for (unsigned i = 0; i < sizeof countries / sizeof countries[0]; i++)
	{
		if (   countries[i][0] == alpha2[0]
		    && countries[i][1] == alpha2[1])
		{
			return 1;
		}
	}

	return 0;
}

static int wpa_driver_circle_get_bssid (void *priv, u8 *bssid)
{
	wpa_driver_circle_data *drv = (wpa_driver_circle_data *) priv;
	assert (drv != 0);

	assert (drv->netdev != 0);
	const CMACAddress *mac = drv->netdev->GetBSSID ();

	assert (mac != 0);
	assert (bssid != 0);
	mac->CopyTo (bssid);

	return 0;
}

static int wpa_driver_circle_get_ssid (void *priv, u8 *ssid)
{
	wpa_driver_circle_data *drv = (wpa_driver_circle_data *) priv;
	assert (drv != 0);

	assert (ssid != 0);
	if (drv->ssid_len > 0)
	{
		os_memcpy (ssid, drv->ssid, drv->ssid_len);
	}

	return drv->ssid_len;
}

static int wpa_driver_circle_set_key (void *priv, wpa_alg alg, const u8 *addr,
				      int key_idx, int set_tx, const u8 *seq, size_t seq_len,
				      const u8 *key, size_t key_len)
{
	wpa_driver_circle_data *drv = (wpa_driver_circle_data *) priv;
	assert (drv != 0);
	assert (drv->netdev != 0);

	if (alg == WPA_ALG_NONE)	// TODO: clear key
	{
		return 0;
	}

	u8 key_tkip[32];
	if (alg == WPA_ALG_TKIP && key_len == 32)
	{
		// swap MIC keys, see set_key comment in driver.h
		os_memcpy (key_tkip, key, 16);
		os_memcpy (key_tkip+16, key+24, 8);
		os_memcpy (key_tkip+24, key+16, 8);

		key = key_tkip;
	}

	CString Number;
	CString Address;
	for (int i = 0; i < ETH_ALEN; i++)
	{
		Number.Format ("%02X", addr[i]);
		Address.Append (Number);
	}

	assert (alg == WPA_ALG_TKIP || alg == WPA_ALG_CCMP);
	CString Key (alg == WPA_ALG_TKIP ? "tkip:" : "ccmp:");

	assert (key_len > 0);
	for (unsigned i = 0; i < key_len; i++)
	{
		Number.Format ("%02X", key[i]);
		Key.Append (Number);
	}

	Key.Append ("@");

	assert (seq_len > 1);
	for (int i = seq_len-1; i >= 0; i--)
	{
		Number.Format ("%02X", seq[i]);
		Key.Append (Number);
	}

	CString Command;
	if (set_tx)
	{
		assert (key_idx == 0);
		Command = "txkey";
	}
	else
	{
		assert (key_idx <= 3);
		Command.Format ("rxkey%u", key_idx);
	}

	if (!drv->netdev->Control ("%s %s %s", (const char *) Command,
				   (const char *) Address, (const char *) Key))
	{
		return -1;
	}

	return 0;
}

static void wpa_driver_circle_event_handler (ether_event_type_t		 type,
					     const ether_event_params_t *params,
					     void			*context)
{
	wpa_driver_circle_data *drv = (wpa_driver_circle_data *) context;
	assert (drv != 0);

	wpa_event_data data;
	memset (&data, 0, sizeof data);

	switch (type)
	{
	case ether_event_disassoc:
		drv->ssid_len = 0;
		wpa_supplicant_event (drv->ctx, EVENT_DISASSOC, 0);
		break;

	case ether_event_deauth:
		wpa_supplicant_event (drv->ctx, EVENT_DEAUTH, 0);
		break;

	case ether_event_mic_error:
		assert (params != 0);
		data.michael_mic_failure.unicast = !params->mic_error.group;
		data.michael_mic_failure.src = params->mic_error.addr;
		wpa_supplicant_event (drv->ctx, EVENT_MICHAEL_MIC_FAILURE, &data);
		break;

	default:
		wpa_printf (MSG_DEBUG, "Unhandled event %u", type);
		break;
	}
}

static void *wpa_driver_circle_init (void *ctx, const char *ifname)
{
	CNetDevice *netdev = CNetDevice::GetNetDevice (NetDeviceTypeWLAN);
	if (netdev == 0)
	{
		return 0;
	}

	wpa_driver_circle_data *drv = (wpa_driver_circle_data *) os_zalloc (sizeof *drv);
	if (drv == 0)
	{
		return 0;
	}

	drv->ctx = ctx;
	drv->netdev = (CBcm4343Device *) netdev;	// netdev can only be of this type
	drv->country_set = 0;

	drv->netdev->RegisterEventHandler (wpa_driver_circle_event_handler, drv);

	return drv;
}

static void wpa_driver_circle_deinit (void *priv)
{
	wpa_driver_circle_data *drv = (wpa_driver_circle_data *) priv;
	assert (drv != 0);

	drv->netdev->RegisterEventHandler (0, 0);

	os_free (drv);
}

#define SCAN_DURATION_SECS	3

static void wpa_driver_circle_scan_timeout (void *eloop_ctx, void *timeout_ctx)
{
	wpa_driver_circle_data *drv = (wpa_driver_circle_data *) eloop_ctx;
	assert (drv != 0);

	assert (drv->netdev != 0);
	drv->netdev->Control ("escan 0");	// stop scan

	wpa_supplicant_event (timeout_ctx, EVENT_SCAN_RESULTS, 0);
}

static int wpa_driver_circle_scan2 (void *priv, wpa_driver_scan_params *params)
{
	wpa_driver_circle_data *drv = (wpa_driver_circle_data *) priv;
	assert (drv != 0);

	assert (params != 0);

	// TODO: allow scan params
	//assert (params->num_ssids == 0);
	assert (params->extra_ies == 0);
	assert (params->extra_ies_len == 0);
	assert (params->freqs == 0);

	assert (drv->netdev != 0);
	// increase scan duration here to be sure, scan is not started again
	if (!drv->netdev->Control ("escan %u", SCAN_DURATION_SECS+2))
	{
		return -1;
	}

	eloop_cancel_timeout (wpa_driver_circle_scan_timeout, drv, drv->ctx);
	eloop_register_timeout (SCAN_DURATION_SECS, 0, wpa_driver_circle_scan_timeout,
				drv, drv->ctx);

	return 0;
}

static int chanspec2freq (u16 chanspec)
{
	u8 chan = chanspec & 0xFF;

	if (1 <= chan && chan <= 14)
	{
		static const int low_freqs[] =
		{
			2412, 2417, 2422, 2427, 2432, 2437, 2442,
			2447, 2452, 2457, 2462, 2467, 2472, 2484
		};

		return low_freqs[chan-1];
	}

	if (32 <= chan && chan <= 173)
	{
		return 5160 + (chan-32) * 5;
	}

	return -1;
}

#define MAX_SCAN_RESULTS	128

static wpa_scan_results *wpa_driver_circle_get_scan_results2 (void *priv)
{
	wpa_driver_circle_data *drv = (wpa_driver_circle_data *) priv;
	assert (drv != 0);

	wpa_scan_res **res_vector =
		(wpa_scan_res **) os_zalloc (MAX_SCAN_RESULTS * sizeof (wpa_scan_res *));
	if (res_vector == 0)
	{
		return 0;
	}

	wpa_scan_results *results = (wpa_scan_results *) os_zalloc (sizeof (wpa_scan_results));
	if (results == 0)
	{
		os_free (res_vector);

		return 0;
	}
	results->res = res_vector;
	results->num = 0;

	unsigned len;
	u8 buf[FRAME_BUFFER_SIZE];
	assert (drv->netdev != 0);
	while (drv->netdev->ReceiveScanResult (buf, &len))
	{
		// remove remaining scan messages, if vector is full
		if (results->num == MAX_SCAN_RESULTS)
		{
			continue;
		}

		// TODO: validate escan result data
		brcmf_escan_result_le *scan_res = (brcmf_escan_result_le *) buf;

		brcmf_bss_info_le *bss = &scan_res->bss_info_le;
		for (unsigned i = 0; i < scan_res->bss_count; i++)
		{
			assert (bss->version == BRCMF_BSS_INFO_VERSION);

			int freq = chanspec2freq (bss->chanspec);
			if (freq <= 0)
			{
				continue;
			}

			wpa_scan_res *res =
				(wpa_scan_res *) os_zalloc (sizeof (wpa_scan_res) + bss->ie_length);
			if (res == 0)
			{
				break;
			}

			os_memset (res, 0, sizeof *res);

			res->flags = WPA_SCAN_LEVEL_DBM | WPA_SCAN_QUAL_INVALID;
			os_memcpy (res->bssid, bss->BSSID, ETH_ALEN);
			res->freq = freq;
			res->beacon_int = bss->beacon_period;
			res->caps = bss->capability;
			res->noise = bss->phy_noise;
			res->level = bss->RSSI;
			// TODO: set res->tsf
			// TODO: set res->age

			// append IEs
			res->ie_len = bss->ie_length;
			os_memcpy ((u8 *) res + sizeof *res, (u8 *) bss + bss->ie_offset,
				   bss->ie_length);

			*res_vector++ = res;
			results->num++;

			bss = (brcmf_bss_info_le *) ((u8 *) bss + bss->length);
		}
	}

	return results;
}

static int wpa_driver_circle_disassociate (void *priv, const u8 *addr, int reason_code)
{
	wpa_driver_circle_data *drv = (wpa_driver_circle_data *) priv;
	assert (drv != 0);

	assert (drv->netdev != 0);
	if (!drv->netdev->Control ("disassoc %d", reason_code))
	{
		return -1;
	}

	return 0;
}

static int wpa_driver_circle_associate (void *priv, wpa_driver_associate_params *params)
{
	wpa_driver_circle_data *drv = (wpa_driver_circle_data *) priv;
	assert (drv != 0);
	assert (params != 0);

	CString BSSID ("FFFFFFFFFFFF");
	if (params->bssid != 0)
	{
		BSSID = "";
		for (unsigned i = 0; i < ETH_ALEN; i++)
		{
			CString Number;
			Number.Format ("%02X", (unsigned) params->bssid[i]);

			BSSID.Append (Number);
		}
	}

	char ssid[32+1];
	assert (params->ssid != 0);
	assert (params->ssid_len < sizeof ssid);
	os_memcpy (ssid, params->ssid, params->ssid_len);
	ssid[params->ssid_len] = '\0';

	CString SSID (ssid);
	SSID.Replace (" ", "\\x20");

	if (params->auth_alg != AUTH_ALG_OPEN_SYSTEM)
	{
		wpa_printf (MSG_ERROR, "Auth algorithm not supported (%d)", params->auth_alg);

		return -1;
	}

	int chan = 0;		// TODO: set channel from params->freq

	CString Auth ("off");
	if (params->wpa_ie != 0 && params->wpa_ie_len > 0)
	{
		Auth = "";
		assert (params->wpa_ie != 0);
		for (unsigned i = 0; i < params->wpa_ie_len; i++)
		{
			CString Number;
			Number.Format ("%02X", (unsigned) params->wpa_ie[i]);

			Auth.Append (Number);
		}
	}

	if (!drv->country_set)
	{
		wpa_printf (MSG_ERROR, "Country code not set");

		return -1;
	}

	assert (drv->netdev != 0);
	if (!drv->netdev->Control ("join %s %s %u %s", (const char *) SSID,
				   (const char *) BSSID, chan, (const char *) Auth))
	{
		return -1;
	}

	os_memcpy (drv->ssid, params->ssid, params->ssid_len);
	drv->ssid_len = params->ssid_len;

	wpa_supplicant_event (drv->ctx, EVENT_ASSOC, 0);

	return 0;
}

static int wpa_driver_circle_set_country (void *priv, const char *alpha2)
{
	wpa_driver_circle_data *drv = (wpa_driver_circle_data *) priv;
	assert (drv != 0);

	char country[3];
	assert (alpha2 != 0);
	country[0] = alpha2[0];
	country[1] = alpha2[1];
	country[2] = '\0';

	if (!is_valid_country_code (alpha2))
	{
		wpa_printf (MSG_ERROR, "Invalid country code: '%s'", country);

		return -1;
	}

	wpa_printf (MSG_INFO, "Setting country code to '%s'", country);

	assert (drv->netdev != 0);
	if (!drv->netdev->Control ("country %s", country))
	{
		return -1;
	}

	drv->country_set = 1;

	return 0;
}

extern "C" const struct wpa_driver_ops wpa_driver_circle_ops =
{
	.name = "circle",
	.desc = "Circle WLAN driver",
	.get_bssid = wpa_driver_circle_get_bssid,
	.get_ssid = wpa_driver_circle_get_ssid,
	.set_key = wpa_driver_circle_set_key,
	.init = wpa_driver_circle_init,
	.deinit = wpa_driver_circle_deinit,
	.disassociate = wpa_driver_circle_disassociate,
	.associate = wpa_driver_circle_associate,
	.get_scan_results2 = wpa_driver_circle_get_scan_results2,
	.set_country = wpa_driver_circle_set_country,
	.scan2 = wpa_driver_circle_scan2,
};
