/*
 * libwebsockets - small server side websockets and web server implementation
 *
 * Copyright (C) 2010-2018 Andy Green <andy@warmcat.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation:
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 *
 * included from libwebsockets.h
 */

typedef int lws_sockfd_type;
typedef int lws_filefd_type;
#if 0
struct pollfd {
	lws_sockfd_type fd; /**< fd related to */
	short events; /**< which POLL... events to respond to */
	short revents; /**< which POLL... events occurred */
};
#define POLLIN		0x0001
#define POLLPRI		0x0002
#define POLLOUT		0x0004
#define POLLERR		0x0008
#define POLLHUP		0x0010
#define POLLNVAL	0x0020
#endif
#include <FreeRTOS.h>
#include <event_groups.h>
#include <string.h>
#include "lwip/sockets.h"
#include "timers.h"


//#define lws_pollfd pollfd
//#define LWS_POLLHUP (POLLHUP) //(POLLHUP | POLLERR)
//#define LWS_POLLIN (POLLIN)
//#define LWS_POLLOUT (POLLOUT)

#if !defined(CONFIG_FREERTOS_HZ)
#define CONFIG_FREERTOS_HZ 100
#endif

typedef TimerHandle_t uv_timer_t;
typedef void uv_cb_t(uv_timer_t *);
typedef void * uv_handle_t;

struct timer_mapping {
	uv_cb_t *cb;
	uv_timer_t *t;
};

#define UV_VERSION_MAJOR 1

#define lws_uv_getloop(a, b) (NULL)

static LWS_INLINE void uv_timer_init(void *l, uv_timer_t *t)
{
	(void)l;
	*t = NULL;
}

extern void esp32_uvtimer_cb(TimerHandle_t t);

static LWS_INLINE void uv_timer_start(uv_timer_t *t, uv_cb_t *cb, int first, int rep)
{
	struct timer_mapping *tm = (struct timer_mapping *)malloc(sizeof(*tm));

	if (!tm)
		return;

	tm->t = t;
	tm->cb = cb;

	*t = xTimerCreate("x", pdMS_TO_TICKS(first), !!rep, tm,
			  (TimerCallbackFunction_t)esp32_uvtimer_cb);
	xTimerStart(*t, 0);
}

static LWS_INLINE void uv_timer_stop(uv_timer_t *t)
{
	xTimerStop(*t, 0);
}

static LWS_INLINE void uv_close(uv_handle_t *h, void *v)
{
	free(pvTimerGetTimerID((uv_timer_t)h));
	xTimerDelete(*(uv_timer_t *)h, 0);
}

/* ESP32 helper declarations */

#include <mdns.h>
//#include <esp_partition.h>

#define LWS_PLUGIN_STATIC
#define LWS_MAGIC_REBOOT_TYPE_ADS 0x50001ffc
#define LWS_MAGIC_REBOOT_TYPE_REQ_FACTORY 0xb00bcafe
#define LWS_MAGIC_REBOOT_TYPE_FORCED_FACTORY 0xfaceb00b
#define LWS_MAGIC_REBOOT_TYPE_FORCED_FACTORY_BUTTON 0xf0cedfac
#define LWS_MAGIC_REBOOT_TYPE_REQ_FACTORY_ERASE_OTA 0xfac0eeee

/* user code provides these */

/* lws-plat-esp32 provides these */

struct lws_group_member {
	struct lws_group_member *next;
	uint64_t last_seen;
	char model[16];
	char role[16];
	char host[32];
	char mac[20];
	int width, height;
	struct ip4_addr addr;
	uint8_t	flags;
};

#define LWS_SYSTEM_GROUP_MEMBER_ADD		1
#define LWS_SYSTEM_GROUP_MEMBER_CHANGE		2
#define LWS_SYSTEM_GROUP_MEMBER_REMOVE		3

#define LWS_GROUP_FLAG_SELF 1

struct lws_stm32 {
	char sta_ip[16];
	char sta_mask[16];
	char sta_gw[16];
	char serial[16];
	char opts[16];
	char model[16];
	char group[16];
	char role[16];
	char ssid[4][64];
	char password[4][64];
	char active_ssid[64];
	char access_pw[16];
	char hostname[32];
	char mac[20];
	char le_dns[64];
	char le_email[64];
       	char region;
       	char inet;
	char conn_ap;

	void *scan_consumer_arg;
	struct lws_group_member *first;
	int extant_group_members;

	char acme;
	char upload;

	volatile char button_is_down;
};


extern struct lws_stm32 lws_stm32;
struct lws_vhost;


struct lws_context_creation_info;
extern void
lws_stm32_set_creation_defaults(struct lws_context_creation_info *info);
extern struct lws_context *
lws_stm32_init(struct lws_context_creation_info *, struct lws_vhost **pvh);


