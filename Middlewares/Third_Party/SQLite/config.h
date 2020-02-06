/*
 * config.h
 *
 *  Created on: Oct 31, 2017
 *      Author: tcpatel
 */

#ifndef THIRD_PARTY_SQLITE_SRC_CONFIG_H_
#define THIRD_PARTY_SQLITE_SRC_CONFIG_H_

/* Enable Nucleus Mutex and VFS subsystem. */
#define  SQLITE_OS_OTHER 1

#define SQLITE_OS_FREERTOS 1

/* Enable FreeRTOS Memory subsystem. */
#define  SQLITE_FREERTOS_MALLOC 1

/* Disable floating point support. */
//#define  SQLITE_OMIT_FLOATING_POINT 1

/* Disable auto inititialization of SQLite by itself. */
#define  SQLITE_OMIT_AUTOINIT 1

//#define SQLITE_ZERO_MALLOC

//#define SQLITE_ENABLE_MEMSYS5

#define SQLITE_ENABLE_MEMORY_MANAGEMENT

#define SQLITE_DEFAULT_CACHE_SIZE 10

#define SQLITE_DEFAULT_PAGE_SIZE 1024

#define SQLITE_DISABLE_LFS

#define SQLITE_DEFAULT_SECTOR_SIZE 512

#define SQLITE_OMIT_LOCALTIME

#define SQLITE_OMIT_DEPRECATED

#define SQLITE_MAX_WORKER_THREADS 0
//#define SQLITE_ENABLE_FTS5

#endif /* THIRD_PARTY_SQLITE_SRC_CONFIG_H_ */
