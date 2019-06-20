
/**
  ******************************************************************************
  * File Name          : lwipopts.h
  * Description        : This file overrides LwIP stack default configuration
  *                      done in opt.h file.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
 
/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __LWIPOPTS__H__
#define __LWIPOPTS__H__

#include "main.h"

/*-----------------------------------------------------------------------------*/
/* Current version of LwIP supported by CubeMx: 2.0.3 -*/
/*-----------------------------------------------------------------------------*/

/* Within 'USER CODE' section, code will be kept by default at each generation */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

#ifdef __cplusplus
 extern "C" {
#endif

/* STM32CubeMX Specific Parameters (not defined in opt.h) ---------------------*/
/* Parameters set in STM32CubeMX LwIP Configuration GUI -*/
/*----- WITH_RTOS enabled (Since FREERTOS is set) -----*/
#define WITH_RTOS 1
/*----- WITH_MBEDTLS enabled (Since MBEDTLS and FREERTOS are set) -----*/
#define WITH_MBEDTLS 1
#if 0
/*----- CHECKSUM_BY_HARDWARE enabled -----*/
#define CHECKSUM_BY_HARDWARE 0
/*-----------------------------------------------------------------------------*/

/* LwIP Stack Parameters (modified compared to initialization value in opt.h) -*/
/* Parameters set in STM32CubeMX LwIP Configuration GUI -*/
/*----- Value in opt.h for LWIP_DHCP: 0 -----*/
#define LWIP_DHCP 1
/*----- Value in opt.h for LWIP_DNS: 0 -----*/
#define LWIP_DNS 1
/*----- Default Value for MEMP_NUM_UDP_PCB: 4 ---*/
#define MEMP_NUM_UDP_PCB 6
/*----- Default Value for MEMP_NUM_TCP_PCB: 5 ---*/
#define MEMP_NUM_TCP_PCB 10
/*----- Value in opt.h for MEM_ALIGNMENT: 1 -----*/
#define MEM_ALIGNMENT 4
/*----- Default Value for MEM_SIZE: 1600 ---*/
#define MEM_SIZE 10240
/*----- Default Value for MEMP_NUM_PBUF: 16 ---*/
#define MEMP_NUM_PBUF 10
/*----- Default Value for MEMP_NUM_TCP_PCB_LISTEN: 8 ---*/
#define MEMP_NUM_TCP_PCB_LISTEN 5
/*----- Value supported for H7 devices: 1 -----*/
#define LWIP_SUPPORT_CUSTOM_PBUF 1
/*----- Default Value for PBUF_POOL_BUFSIZE: 592 ---*/
#define PBUF_POOL_BUFSIZE 1528
/*----- Value in opt.h for LWIP_ETHERNET: LWIP_ARP || PPPOE_SUPPORT -*/
#define LWIP_ETHERNET 1
/*----- Value in opt.h for LWIP_DNS_SECURE: (LWIP_DNS_SECURE_RAND_XID | LWIP_DNS_SECURE_NO_MULTIPLE_OUTSTANDING | LWIP_DNS_SECURE_RAND_SRC_PORT) -*/
#define LWIP_DNS_SECURE 7
/*----- Default Value for TCP_MSS: 536 ---*/
#define TCP_MSS 1460
/*----- Value in opt.h for TCP_SND_QUEUELEN: (4*TCP_SND_BUF + (TCP_MSS - 1))/TCP_MSS -----*/
#define TCP_SND_QUEUELEN 9
/*----- Value in opt.h for TCP_SNDQUEUELOWAT: LWIP_MAX(TCP_SND_QUEUELEN)/2, 5) -*/
#define TCP_SNDQUEUELOWAT 5
/*----- Default Value for LWIP_NETIF_HOSTNAME: 0 ---*/
#define LWIP_NETIF_HOSTNAME 1
/*----- Default Value for LWIP_NETIF_STATUS_CALLBACK: 0 ---*/
#define LWIP_NETIF_STATUS_CALLBACK 1
/*----- Default Value for LWIP_NETIF_LINK_CALLBACK: 0 ---*/
#define LWIP_NETIF_LINK_CALLBACK 1
/*----- Value in opt.h for TCPIP_THREAD_STACKSIZE: 0 -----*/
#define TCPIP_THREAD_STACKSIZE 1024
/*----- Value in opt.h for TCPIP_THREAD_PRIO: 1 -----*/
#define TCPIP_THREAD_PRIO 3
/*----- Value in opt.h for TCPIP_MBOX_SIZE: 0 -----*/
#define TCPIP_MBOX_SIZE 6
/*----- Value in opt.h for SLIPIF_THREAD_STACKSIZE: 0 -----*/
#define SLIPIF_THREAD_STACKSIZE 1024
/*----- Value in opt.h for SLIPIF_THREAD_PRIO: 1 -----*/
#define SLIPIF_THREAD_PRIO 3
/*----- Value in opt.h for DEFAULT_THREAD_STACKSIZE: 0 -----*/
#define DEFAULT_THREAD_STACKSIZE 1024
/*----- Value in opt.h for DEFAULT_THREAD_PRIO: 1 -----*/
#define DEFAULT_THREAD_PRIO 3
/*----- Value in opt.h for DEFAULT_UDP_RECVMBOX_SIZE: 0 -----*/
#define DEFAULT_UDP_RECVMBOX_SIZE 6
/*----- Value in opt.h for DEFAULT_TCP_RECVMBOX_SIZE: 0 -----*/
#define DEFAULT_TCP_RECVMBOX_SIZE 6
/*----- Value in opt.h for DEFAULT_ACCEPTMBOX_SIZE: 0 -----*/
#define DEFAULT_ACCEPTMBOX_SIZE 6
/*----- Value in opt.h for RECV_BUFSIZE_DEFAULT: INT_MAX -----*/
#define RECV_BUFSIZE_DEFAULT 2000000000
/*----- Value in opt.h for LWIP_USE_EXTERNAL_MBEDTLS: 0 -----*/
#define LWIP_USE_EXTERNAL_MBEDTLS 1
/*----- Value in opt.h for LWIP_STATS: 1 -----*/
#define LWIP_STATS 0
/*----- Value in opt.h for CHECKSUM_GEN_IP: 1 -----*/
#define CHECKSUM_GEN_IP 0
/*----- Value in opt.h for CHECKSUM_GEN_UDP: 1 -----*/
#define CHECKSUM_GEN_UDP 0
/*----- Value in opt.h for CHECKSUM_GEN_TCP: 1 -----*/
#define CHECKSUM_GEN_TCP 0
/*----- Value in opt.h for CHECKSUM_GEN_ICMP: 1 -----*/
#define CHECKSUM_GEN_ICMP 0
/*----- Value in opt.h for CHECKSUM_GEN_ICMP6: 1 -----*/
#define CHECKSUM_GEN_ICMP6 0
/*----- Value in opt.h for CHECKSUM_CHECK_IP: 1 -----*/
#define CHECKSUM_CHECK_IP 0
/*----- Value in opt.h for CHECKSUM_CHECK_UDP: 1 -----*/
#define CHECKSUM_CHECK_UDP 0
/*----- Value in opt.h for CHECKSUM_CHECK_TCP: 1 -----*/
#define CHECKSUM_CHECK_TCP 0
/*----- Value in opt.h for CHECKSUM_CHECK_ICMP: 1 -----*/
#define CHECKSUM_CHECK_ICMP 0
/*----- Value in opt.h for CHECKSUM_CHECK_ICMP6: 1 -----*/
#define CHECKSUM_CHECK_ICMP6 0
#endif
/*-----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */
 
 /**
  * SYS_LIGHTWEIGHT_PROT==1: if you want inter-task protection for certain
  * critical regions during buffer allocation, deallocation and memory
  * allocation and deallocation.
  */
 #define SYS_LIGHTWEIGHT_PROT    1

/**
  * NO_SYS==1: Provides VERY minimal functionality. Otherwise,
  * use lwIP facilities.
  */
 #define NO_SYS                  0

 /* ---------- Memory options ---------- */
 /* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
    lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
    byte alignment -> define MEM_ALIGNMENT to 2. */
 #define MEM_ALIGNMENT           4

 /* MEM_SIZE: the size of the heap memory. If the application will send
 a lot of data that needs to be copied, this should be set high. */
 #define MEM_SIZE                (10*1024)

 /* Relocate the LwIP RAM heap pointer */
 #define LWIP_RAM_HEAP_POINTER    (0x30044000)

 /* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
    sends a lot of data out of ROM (or other static memory), this
    should be set high. */
 #define MEMP_NUM_PBUF           10
 /* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
    per active UDP "connection". */
 #define MEMP_NUM_UDP_PCB        6
 /* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
    connections. */
 #define MEMP_NUM_TCP_PCB        10
 /* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
    connections. */
 #define MEMP_NUM_TCP_PCB_LISTEN 5
 /* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
    segments. */
 #define MEMP_NUM_TCP_SEG        12
 /* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
    timeouts. */
 #define MEMP_NUM_SYS_TIMEOUT    10


 /* ---------- Pbuf options ---------- */
 /* PBUF_POOL_SIZE: the number of buffers in the pbuf pool.
    @ note: used to allocate Tx pbufs only */
 #define PBUF_POOL_SIZE          8

 /* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
 #define PBUF_POOL_BUFSIZE       1528

 /* LWIP_SUPPORT_CUSTOM_PBUF == 1: to pass directly MAC Rx buffers to the stack
    no copy is needed */
 #define LWIP_SUPPORT_CUSTOM_PBUF      1


 /* ---------- TCP options ---------- */
 #define LWIP_TCP                1
 #define TCP_TTL                 255
 #define LWIP_IPV4               1
 #define LWIP_IPV6               0


#define LWIP_POSIX_SOCKETS_IO_NAMES     0

 /* Controls if TCP should queue segments that arrive out of
    order. Define to 0 if your device is low on memory. */
 #define TCP_QUEUE_OOSEQ         0

 /* TCP Maximum segment size. */
 #define TCP_MSS                 (1500 - 40)	  /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */

 /* TCP sender buffer space (bytes). */
 #define TCP_SND_BUF             (4*TCP_MSS)

 /*  TCP_SND_QUEUELEN: TCP sender buffer space (pbufs). This must be at least
   as much as (2 * TCP_SND_BUF/TCP_MSS) for things to work. */

 #define TCP_SND_QUEUELEN        (2* TCP_SND_BUF/TCP_MSS)

 /* TCP receive window. */
 #define TCP_WND                 (2*TCP_MSS)


 /* ---------- ICMP options ---------- */
 #define LWIP_ICMP                       1


 /* ---------- DHCP options ---------- */
 #define LWIP_DHCP               1


 /* ---------- UDP options ---------- */
 #define LWIP_UDP                1
 #define UDP_TTL                 255


 /* ---------- Statistics options ---------- */
 #define LWIP_STATS 0
 #define LWIP_PROVIDE_ERRNO
 /* ---------- link callback options ---------- */
 /* LWIP_NETIF_LINK_CALLBACK==1: Support a callback function from an interface
  * whenever the link changes (i.e., link down)
  */
 #define LWIP_NETIF_LINK_CALLBACK 1
 #define LWIP_DHCP_CHECK_LINK_UP         1

 /*
    --------------------------------------
    ---------- Checksum options ----------
    --------------------------------------
 */

 /*
 The STM32H7xx allows computing and verifying the IP, UDP, TCP and ICMP checksums by hardware:
  - To use this feature let the following define uncommented.
  - To disable it and process by CPU comment the  the checksum.
 */
 #define CHECKSUM_BY_HARDWARE


 #ifdef CHECKSUM_BY_HARDWARE
   /* CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.*/
   #define CHECKSUM_GEN_IP                 0
   /* CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.*/
   #define CHECKSUM_GEN_UDP                0
   /* CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.*/
   #define CHECKSUM_GEN_TCP                0
   /* CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.*/
   #define CHECKSUM_CHECK_IP               0
   /* CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.*/
   #define CHECKSUM_CHECK_UDP              0
   /* CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.*/
   #define CHECKSUM_CHECK_TCP              0
   /* CHECKSUM_CHECK_ICMP==0: Check checksums by hardware for incoming ICMP packets.*/
   #define CHECKSUM_GEN_ICMP               0
 #else
   /* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
   #define CHECKSUM_GEN_IP                 1
   /* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
   #define CHECKSUM_GEN_UDP                1
   /* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
   #define CHECKSUM_GEN_TCP                1
   /* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
   #define CHECKSUM_CHECK_IP               1
   /* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
   #define CHECKSUM_CHECK_UDP              1
   /* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
   #define CHECKSUM_CHECK_TCP              1
   /* CHECKSUM_CHECK_ICMP==1: Check checksums by hardware for incoming ICMP packets.*/
   #define CHECKSUM_GEN_ICMP               1
 #endif


 /*
    ----------------------------------------------
    ---------- Sequential layer options ----------
    ----------------------------------------------
 */
 /**
  * LWIP_NETCONN==1: Enable Netconn API (require to use api_lib.c)
  */
 #define LWIP_NETCONN                    1

 /*
    ------------------------------------
    ---------- Socket options ----------
    ------------------------------------
 */
 /**
  * LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
  */
 #define LWIP_SOCKET                     1
 #define LWIP_DNS                        1
 #define SO_REUSE                        1

 /*
    -----------------------------------
    ---------- DEBUG options ----------
    -----------------------------------
 */


 /*
    ---------------------------------
    ---------- OS options ----------
    ---------------------------------
 */

 #define TCPIP_THREAD_NAME              "TCP/IP"
 #define TCPIP_THREAD_STACKSIZE          1000
 #define TCPIP_MBOX_SIZE                 6
 #define DEFAULT_UDP_RECVMBOX_SIZE       6
 #define DEFAULT_TCP_RECVMBOX_SIZE       6
 #define DEFAULT_ACCEPTMBOX_SIZE         6
 #define DEFAULT_THREAD_STACKSIZE        500
 #define TCPIP_THREAD_PRIO               osPriorityHigh
 #define LWIP_COMPAT_MUTEX               1
 #define LWIP_COMPAT_MUTEX_ALLOWED       1

 /*
    ------------------------------------
    ---------- debug options ----------
    ------------------------------------
 */
//#define LWIP_DEBUG                      1
//#define SOCKETS_DEBUG                   LWIP_DBG_ON
//#define API_MSG_DEBUG                   LWIP_DBG_ON
//#define API_LIB_DEBUG                   LWIP_DBG_ON
//#define LWIP_DBG_TYPES_ON         (LWIP_DBG_ON|LWIP_DBG_TRACE|LWIP_DBG_STATE|LWIP_DBG_FRESH|LWIP_DBG_HALT|LWIP_DBG_STATE)
//#define TCP_DEBUG                       LWIP_DBG_ON
//#define TCP_OUTPUT_DEBUG                LWIP_DBG_ON
//#define ETHARP_DEBUG                    LWIP_DBG_ON
//#define TCPIP_DEBUG                     LWIP_DBG_ON
//#define TCP_INPUT_DEBUG                 LWIP_DBG_ON
 //#define PBUF_DEBUG          LWIP_DBG_ON
 //#define TIMERS_DEBUG          LWIP_DBG_ON
 //#define TCPIP_DEBUG          LWIP_DBG_ON

 /*
    ---------------------------------
    ---------- DEBUG options --------
    ---------------------------------
 */
//#define LWIP_STATS_DISPLAY              1
// #define LWIP_NOASSERT
/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /*__LWIPOPTS__H__ */

/************************* (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
