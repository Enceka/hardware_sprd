// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

#include "includes.h"

#include <linux/sockios.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include "common.h"
#include "driver_nl80211.h"
#include "linux_ioctl.h"

/* Unisoc WLAN private ioctl ABI, shared with the kernel driver. */
struct sprd_wifi_priv_cmd {
	char *buf;
	int used_len;
	int total_len;
};

int wpa_driver_nl80211_driver_cmd(void *priv, char *cmd, char *buf,
				 size_t buf_len)
{
	struct i802_bss *bss = priv;
	int sock = bss->drv->global->ioctl_sock;
	struct ifreq ifr = { 0 };
	char request[128] = { 0 };
	struct sprd_wifi_priv_cmd arg = {
		.buf = request,
		.total_len = sizeof(request),
	};
	unsigned long operation;
	int len;

	if (os_strcmp(cmd, "MACADDR") == 0) {
		u8 addr[ETH_ALEN];

		if (linux_get_ifhwaddr(sock, bss->ifname, addr) < 0)
			return -1;
		len = os_snprintf(buf, buf_len, "Macaddr = " MACSTR "\n", MAC2STR(addr));
		return os_snprintf_error(buf_len, len) ? -ENOSPC : len;
	}

	if (os_strcmp(cmd, "SETSUSPENDMODE 0") == 0 ||
	    os_strcmp(cmd, "SETSUSPENDMODE 1") == 0) {
		operation = SIOCDEVPRIVATE + 4;
	} else if (os_strncmp(cmd, "COUNTRY ", 8) == 0 ||
		   os_strncmp(cmd, "RXFILTER-", 9) == 0 ||
		   os_strncmp(cmd, "BTCOEXMODE ", 11) == 0 ||
		   os_strcmp(cmd, "BTCOEXSCAN-START") == 0 ||
		   os_strcmp(cmd, "BTCOEXSCAN-STOP") == 0) {
		operation = SIOCDEVPRIVATE + 1;
	} else {
		return -EOPNOTSUPP;
	}

	if (os_strlcpy(request, cmd, sizeof(request)) >= sizeof(request))
		return -EINVAL;
	arg.used_len = os_strlen(request) + 1;
	os_strlcpy(ifr.ifr_name, bss->ifname, sizeof(ifr.ifr_name));
	ifr.ifr_data = &arg;
	return ioctl(sock, operation, &ifr) < 0 ? -errno : 0;
}

/* Preserve the upstream stub behavior for P2P features this library does not implement. */
int wpa_driver_set_p2p_noa(void *priv, u8 count, int start, int duration)
{
	return 0;
}

int wpa_driver_get_p2p_noa(void *priv, u8 *buf, size_t len)
{
	return 0;
}

int wpa_driver_set_p2p_ps(void *priv, int legacy_ps, int opp_ps, int ctwindow)
{
	return -1;
}

int wpa_driver_set_ap_wps_p2p_ie(void *priv, const struct wpabuf *beacon,
				 const struct wpabuf *proberesp,
				 const struct wpabuf *assocresp)
{
	return 0;
}
