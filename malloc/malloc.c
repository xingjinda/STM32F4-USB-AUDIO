#include "malloc.h"


#if !(__ARMCC_VERSION >= 6010050) 
static __align(64) uint8_t membase[MEM_MAX_SIZE];         
static MT_TYPE memmap[MEM_ALLOC_TABLE_SIZE];
#else
static __ALIGNED(64) uint8_t membase[MEM_MAX_SIZE];
static MT_TYPE memmap[MEM_ALLOC_TABLE_SIZE];
#endif

/* 内存管理参数 */
const uint32_t memtblsize = MEM_ALLOC_TABLE_SIZE; 
const uint32_t memblksize = MEM_BLOCK_SIZE;        
const uint32_t memsize = MEM_MAX_SIZE;

uint8_t  memrdy=0;

void mem_copy(void *des, void *src, uint32_t n)
{
	uint8_t *xdes = des;
	uint8_t *xsrc = src;
	while (n--) *xdes++ = *xsrc++;
}

void mem_set(void *s, uint8_t c, uint32_t count)
{
	uint8_t *xs = s;
	while (count--) *xs++ = c;
}

void mem_init()
{
	uint8_t mttsize = sizeof(MT_TYPE);
	mem_set(memmap,0,memtblsize*mttsize);
	memrdy = 1;
}

uint16_t mem_perused()
{
	uint32_t used = 0;
	uint32_t i;

	for (i = 0; i < memtblsize; i++)
		if (memmap[i]) used++;
	return (used * 1000) / (memtblsize);
}

static uint32_t mem_malloc(uint32_t size)
{
	signed long offset = 0;
	uint32_t nmemb;
	uint32_t cmemb = 0;
	uint32_t i;

	if (!memrdy)
		mem_init();
	if (size == 0) return 0XFFFFFFFF;
	nmemb = size / memblksize;
	if (size % memblksize) nmemb++;
	for (offset = memtblsize - 1; offset >= 0; offset--)
	{
		if (!memmap[offset])
			cmemb++;
		else 
			cmemb = 0;
		if (cmemb == nmemb)
		{
			for (i = 0; i < nmemb; i++)
				memmap[offset + i] = nmemb;
			return (offset * memblksize);
		}
	}
	return 0XFFFFFFFF;
}

static uint8_t mem_free(uint32_t offset)
{
	int i;

	if (!memrdy)
	{
		mem_init();
		return 1;
	}
	if (offset < memsize)
	{
		int index = offset / memblksize;   
		int nmemb = memmap[index];
		for (i = 0; i < nmemb; i++)
			memmap[index + i] = 0;
		return 0;
	}
	else
		return 2;
}

void free(void *ptr)
{
	uint32_t offset;
	if (ptr == NULL) return;
	offset = (uint32_t)ptr - (uint32_t)membase;
	mem_free(offset);
}

void *malloc(uint32_t size)
{
	uint32_t offset;
	offset = mem_malloc(size);
	if (offset == 0XFFFFFFFF)
		return NULL;
	else 
		return (void *)((uint32_t)membase + offset);
}

void *realloc(void *ptr, uint32_t size)
{
	uint32_t offset;
	offset = mem_malloc(size);

	if (offset == 0XFFFFFFFF)
		return NULL;
	else
	{
		mem_copy((void *)((uint32_t)membase + offset), ptr, size);
		free(ptr);
		return (void *)((uint32_t)membase + offset);
	}
}












