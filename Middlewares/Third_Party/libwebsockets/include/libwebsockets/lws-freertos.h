/*
 * libwebsockets - small server side websockets and web server implementation
 *
 * Copyright (C) 2010 - 2019 Andy Green <andy@warmcat.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * This is included from libwebsockets.h if LWS_PLAT_FREERTOS
 */

typedef int lws_sockfd_type;
typedef int lws_filefd_type;



#if defined(LWS_AMAZON_RTOS)
#include <FreeRTOS.h>
#include <event_groups.h>
#include <string.h>
#include "timers.h"
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#else /* LWS_AMAZON_RTOS */
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs.h"
#include "driver/gpio.h"
#include "esp_spi_flash.h"
#include "freertos/timers.h"
#endif /* LWS_AMAZON_RTOS */

#if !defined(CONFIG_FREERTOS_HZ)
#define CONFIG_FREERTOS_HZ 100
#endif
