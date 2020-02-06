/*
 * sqlite_mutex_freertos.c
 *
 *  Created on: Oct 27, 2017
 *      Author: tcpatel
 */


#include "sqliteInt.h"

#ifndef SQLITE_MUTEX_OMIT
#ifdef SQLITE_MUTEX_FREERTOS

#include "FreeRTOS.h"
//#include "sqlite3.h"
#include "semphr.h"
#include "sqlite_mutex_freertos.h"
#include <assert.h>
#include <string.h>
/*
** The mutex object
*/

//#define sqlite3_mutex SemaphoreHandle_t
typedef struct sqlite3_mutex {
	SemaphoreHandle_t *mutex_ptr;     /* Pointer to mutex. */
	int   id;    /* The mutex type */
} sqlite3_mutex;


/* The STATIC Mutexes. */
static sqlite3_mutex xStatic[] = { {NULL, SQLITE_MUTEX_STATIC_MASTER},
                                   {NULL, SQLITE_MUTEX_STATIC_MEM},
                                   {NULL, SQLITE_MUTEX_STATIC_MEM2},
                                   {NULL, SQLITE_MUTEX_STATIC_OPEN},
                                   {NULL, SQLITE_MUTEX_STATIC_PRNG},
                                   {NULL, SQLITE_MUTEX_STATIC_LRU},
                                   {NULL, SQLITE_MUTEX_STATIC_PMEM} };


static int prvFreeRTOSMutexInit( void ){
	return SQLITE_OK;
}

static int prvFreeRTOSMutexEnd( void ){
	return SQLITE_OK;
}

static sqlite3_mutex *prvFreeRTOSMutexAlloc (int id){
    sqlite3_mutex *pxMutex;
    //STATUS status = NU_SUCCESS;

    switch( id ){
        case SQLITE_MUTEX_FAST:
			pxMutex = (sqlite3_mutex* )pvPortMalloc(sizeof(sqlite3_mutex));

			if(pxMutex != NULL){
				pxMutex->id = SQLITE_MUTEX_FAST;
				pxMutex->mutex_ptr = xSemaphoreCreateMutex();
			}

            break;

        case SQLITE_MUTEX_RECURSIVE:
    		pxMutex = (sqlite3_mutex* )pvPortMalloc(sizeof(sqlite3_mutex));

    		if(pxMutex != NULL){
    			pxMutex->id = SQLITE_MUTEX_RECURSIVE;
    			pxMutex->mutex_ptr = xSemaphoreCreateRecursiveMutex();
    		}
            break;


        default: /*static mutexes*/
        	assert( id-2 >= 0 );
        	assert( id-2 < (int)(sizeof(xStatic)/sizeof(xStatic[0])) );
        	pxMutex = &xStatic[id-2];

            /* The static mutex will be allocated once only, on first call. */
            if(pxMutex->mutex_ptr == NULL) {
                pxMutex->id = id;
                pxMutex->mutex_ptr = xSemaphoreCreateMutex();
            }
            break;

    }
    if(pxMutex->mutex_ptr == NULL)
    {
    	if(id == SQLITE_MUTEX_FAST || id == SQLITE_MUTEX_RECURSIVE)
    		vPortFree(pxMutex);

    	pxMutex = NULL;
    }

    return (sqlite3_mutex*)pxMutex;
}

static void prvFreeRTOSMutexFree(sqlite3_mutex *pxMutex){
	assert(xSemaphoreGetMutexHolder(*pxMutex->mutex_ptr) == NULL);

	vSemaphoreDelete(pxMutex->mutex_ptr);

	vPortFree(pxMutex);
}


static int prvFreeRTOSMutexTry (sqlite3_mutex *pxMutex){
	UNUSED_PARAMETER(pxMutex);

	return SQLITE_BUSY;
}

static void prvFreeRTOSMutexLeave(sqlite3_mutex *pxMutex){
	if(pxMutex->id == SQLITE_MUTEX_RECURSIVE)
	{
		xSemaphoreGiveRecursive(pxMutex->mutex_ptr);
	}
	else
	{
		xSemaphoreGive(pxMutex->mutex_ptr);
	}
}

static void prvFreeRTOSMutexEnter(sqlite3_mutex *pxMutex){
	assert(xSemaphoreGetMutexHolder(*pxMutex->mutex_ptr) == NULL);

	if(pxMutex->id == SQLITE_MUTEX_RECURSIVE)
	{
		xSemaphoreTakeRecursive(pxMutex->mutex_ptr,portMAX_DELAY);
	}
	else
	{
		xSemaphoreTake(pxMutex->mutex_ptr,portMAX_DELAY);
	}
}

void sqlite3MemoryBarrier(void){

}

sqlite3_mutex_methods const *sqlite3DefaultMutex(void){
    static const sqlite3_mutex_methods xMutex = {
    		prvFreeRTOSMutexInit,
			prvFreeRTOSMutexEnd,
			prvFreeRTOSMutexAlloc,
			prvFreeRTOSMutexFree,
			prvFreeRTOSMutexEnter,
			prvFreeRTOSMutexTry,
			prvFreeRTOSMutexLeave,
#if SQLITE_DEBUG && !defined(NDEBUG)
			NULL,
			NULL
#else
			0,
			0
#endif
    };

    return &xMutex;
}

#endif /* defined(SQLITE_MUTEX_FREERTOS) */
#endif /* !defined(SQLITE_MUTEX_OMIT) */
