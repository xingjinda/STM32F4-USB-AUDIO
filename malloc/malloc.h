#ifndef __MALLOC_H
#define __MALLOC_H

#include "sys.h"


#define MT_TYPE     uint16_t


#define MEM_BLOCK_SIZE			32
#if defined(STM32F412Rx)
	#define MEM_MAX_SIZE			100*1024
#elif defined(STM32F401xC)
	#define MEM_MAX_SIZE			32*1024
#endif
#define MEM_ALLOC_TABLE_SIZE	MEM_MAX_SIZE/MEM_BLOCK_SIZE

#ifndef NULL
#define NULL 0
#endif



void mem_init(void);
uint16_t mem_perused(void) ;
void mem_set(void *s, uint8_t c, uint32_t count);
void mem_copy(void *des, void *src, uint32_t n);
void free(void *ptr);
void *malloc(uint32_t size);
void *realloc(void *ptr, uint32_t size);

#endif













