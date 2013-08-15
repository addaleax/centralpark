#ifndef _CENTRALPARK_H
#define _CENTRALPARK_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct CentralPark {
	size_t memsz;
	size_t blocksize;
	size_t alignment;
	unsigned int* metablock;
	char* mainstart;
	unsigned int* curptr;
	size_t freecount;
	size_t blockcount;
};

struct CentralPark* newCentralPark(size_t blocksize, size_t memsz, size_t alignment);
void* cpAlloc(struct CentralPark* cp);
void  cpFree(struct CentralPark* cp, void* memory);
void destroyCentralPark(struct CentralPark* cp);
void clearCentralPark(struct CentralPark* cp);

#ifdef __cplusplus
}
#endif

#endif
