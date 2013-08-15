#include "w32compat.h"
#include "centralpark.h"
#include <strings.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#ifdef WITH_VG_MEMCHECK
#include <memcheck.h>
#endif

#ifdef linux
#include <sys/mman.h>
#include <stdlib.h>

static void* _BigMalloc(size_t sz) {
	int flags = MAP_PRIVATE | MAP_ANONYMOUS;
	
	if (getenv("NOPREFAULT") == NULL)
		flags |= MAP_POPULATE;
	
	void* mem = mmap(NULL, sz, PROT_READ | PROT_WRITE, flags, -1, 0);
	return mem;
}

static void _BigFree(void* ptr, size_t sz) {
	munmap(ptr, sz);
}

#define _Malloc(sz) _BigMalloc((sz))
#define _Free(p,sz) _BigFree((p),(sz))
#else
#define _Malloc(sz) malloc((sz))
#define _Free(p,sz) free((p))
#endif

static struct CentralPark* newCentralPark_(size_t blocksize, size_t memsz, struct CentralPark* cp, size_t alignment) {	
	if (alignment == 0)
		alignment = 16;
		
	if (blocksize % alignment)
		blocksize += alignment - (blocksize % alignment);
		
	assert(blocksize < memsz);
	assert(alignment < memsz);
		
	cp->memsz = memsz;
	cp->blocksize = blocksize;
	cp->alignment = alignment;
	cp->metablock = (unsigned int*) (cp + 1);
	
	if ((long)cp->metablock % alignment)
		cp->metablock = (unsigned int*)( ((char*)cp->metablock) + (alignment - ((long)cp->metablock % alignment)) );
	cp->curptr = cp->metablock;
	
	/* now there is memsz - sizeof(*cp) bytes of memory left;
	 * we can address sizeof(int) * CHAR_BIT * blocksize = X bytes directly,
	 * resulting in:
	 * CHAR_BIT * blocksize * metamem_b = (memsz - cp) - metamem_b
	 * => (memsz - cp) / (CHAR_BIT * blocksize + 1) = metamem_b
	 */
	size_t metamem_bytes = (memsz - sizeof(*cp)) / (CHAR_BIT * blocksize + 1);
	
	assert(alignment % sizeof(int) == 0);
	if (metamem_bytes < alignment) {
		cp->mainstart = (char*)(cp->metablock + alignment);
		memset(cp->metablock, -1, alignment);
		cp->blockcount = CHAR_BIT * metamem_bytes;
	} else {
		size_t metamem_ints = ((metamem_bytes / alignment) * alignment) / sizeof(int);
		
		cp->mainstart = (char*)(cp->metablock + metamem_ints);
		memset(cp->metablock, -1, metamem_ints * sizeof(int));
		cp->blockcount = sizeof(int) * CHAR_BIT * metamem_ints;
	}
	
	cp->freecount = cp->blockcount;
	assert(cp->freecount * blocksize < memsz);
	
#ifdef WITH_VG_MEMCHECK
	VALGRIND_MAKE_MEM_NOACCESS(cp->mainstart, cp->memsz - ((char*)cp->mainstart - (char*)cp));
	VALGRIND_CREATE_MEMPOOL(cp, 0, 0);
#endif
	
	return cp;
}

struct CentralPark* newCentralPark(size_t blocksize, size_t memsz, size_t alignment) {
	struct CentralPark* cp = _Malloc(memsz);
	if (!cp)
		return NULL;
	return newCentralPark_(blocksize, memsz, cp, alignment);
}

void clearCentralPark(struct CentralPark* cp) {
#ifdef WITH_VG_MEMCHECK
	VALGRIND_DESTROY_MEMPOOL(cp);
#endif

	newCentralPark_(cp->blocksize, cp->memsz, cp, cp->alignment);
}

void* cpAlloc(struct CentralPark* cp) {
	int ffsbit;
	size_t offset;
		
	for (;;) {
		assert(cp->curptr >= cp->metablock);
		assert((char*)cp->curptr <= cp->mainstart);
		
		ffsbit = ffs(*cp->curptr);
		
		if (!(ffsbit--)) {
			if (!cp->freecount)
				return NULL;
			
			cp->curptr++;
			
			if ((char*)cp->curptr == cp->mainstart)
				cp->curptr = cp->metablock;
				
			continue;
		}
			
		assert(ffsbit >= 0 && ffsbit < sizeof(int) * CHAR_BIT);
		
		offset = (cp->curptr - cp->metablock) * sizeof(int) * CHAR_BIT + ffsbit;
		assert((*cp->curptr & (1U << (unsigned int)ffsbit)) != 0);
		
		*cp->curptr &= ~(1U << (unsigned int)ffsbit);
		
		cp->freecount--;
		void* rv = cp->mainstart + offset * cp->blocksize;
#ifdef WITH_VG_MEMCHECK
		VALGRIND_MEMPOOL_ALLOC(cp, rv, cp->blocksize);
#endif
#ifdef __GNUC__
		__builtin_prefetch(((char*)rv) +   0, 1, 3);
		__builtin_prefetch(((char*)rv) +  64, 1, 3);
#endif
		return rv;
	}
}

void cpFree(struct CentralPark* cp, void* memory) {
	if (!memory)
		return;
		
	assert((char*)memory >= cp->mainstart);
	assert((char*)memory < ((char*)cp) + cp->memsz);
		
	size_t offset = (char*)memory - cp->mainstart;
	assert(offset % cp->blocksize == 0);
	
	size_t intindex = (offset / cp->blocksize) / (sizeof(int) * CHAR_BIT);
	unsigned int bit = (offset / cp->blocksize) % (sizeof(int) * CHAR_BIT);
	
	assert(intindex * sizeof(int) < cp->mainstart - (char*) cp);
	
	assert((cp->metablock[intindex] & (1U << bit)) == 0);
	cp->metablock[intindex] |= 1U << bit;
	cp->freecount++;
	
#ifdef WITH_VG_MEMCHECK
	VALGRIND_MEMPOOL_FREE(cp, memory);
#endif
}

void destroyCentralPark(struct CentralPark* cp) {
#ifdef WITH_VG_MEMCHECK
	VALGRIND_DESTROY_MEMPOOL(cp);
#endif

	_Free(cp,cp->memsz); /* d'uh. */
}
