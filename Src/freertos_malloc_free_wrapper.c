/*
 * freertos_malloc_free_wrapper.c
 *
 *  Created on: Apr 12, 2019
 *      Author: tcpatel
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <FreeRTOS.h>






void *pvPortRealloc(void *mem, size_t newsize)
{
    if (newsize == 0) {
        vPortFree(mem);
        return NULL;
    }

    void *p;
    p = pvPortMalloc(newsize);
    if (p) {
        /* zero the memory */
        if (mem != NULL) {
            memcpy(p, mem, newsize);
            vPortFree(mem);
        }
    }
    return p;
}



void *__wrap__malloc_r(struct _reent *r, size_t size)
{
    return pvPortMalloc( size );
}

void *__wrap__realloc_r(struct _reent *r, void *ptr, size_t size)
{
	return pvPortRealloc( ptr, size );
}

void __wrap__free_r(struct _reent *r, void *ptr)
{
	vPortFree( ptr );
}

void *__wrap__calloc_r(struct _reent *r, size_t nmemb, size_t size)
{
    void *ptr = NULL;
    ptr = malloc(nmemb * size);
    if (ptr != NULL) {
        memset(ptr, 0, nmemb * size);
    }

    return ptr;
}



