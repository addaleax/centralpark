#ifndef _CENTRALPARK_H
#define _CENTRALPARK_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Main data structure for the managed array.
 */
struct CentralPark {
	/**
	 * Total available memory size
	 */
	size_t memsz;
	
	/**
	 * Size of each single block (taking alignment into account)
	 */
	size_t blocksize;
	
	/**
	 * Byte alignment for each block
	 */
	size_t alignment;
	
	/**
	 * Start of meta (allocated/free) information
	 */
	unsigned int* metablock;
	
	/**
	 * Start of managed blocks
	 */
	char* mainstart;
	
	/**
	 * Current pointer for next allocation
	 */
	unsigned int* curptr;
	
	/**
	 * Free block count
	 */
	size_t freecount;
	
	/**
	 * Total block count
	 */
	size_t blockcount;
};

/**
 * Create a new managed memory region.
 * All other parameters are computed from these values, including the total block count.
 * On Linux, mmap() is used, and, unless the environment variable NOPREFAULT is set,
 * all pages will be initialized. On all other OS, malloc() will be used.
 * 
 * @param blocksize  Size of a single block (i. e. array entry)
 * @param memsz      Total available memory for this region (including all meta-information)
 * @param alignment  Alignment of a single block. If 0, a default value is chosen.
 *                   This need not be a power of 2, but you will most likely want one.
 * 
 * @return NULL on failure, a pointer to a control data structure otherwise.
 */
struct CentralPark* newCentralPark(size_t blocksize, size_t memsz, size_t alignment);

/**
 * Allocate (i. e., mark) a single block from the memory region.
 * 
 * @return A pointer to a block on success, NULL on failure (i. e., OOM)
 */
void* cpAlloc(struct CentralPark* cp);

/**
 * Frees a memory region.
 * 
 * @param memory Pointer to the block to be freed. If NULL, no action is taken.
 *               Must be a valid pointer or NULL, otherwise behaviour is undefined.
 */
void cpFree(struct CentralPark* cp, void* memory);

/**
 * Free all associated memory for this memory region.
 */
void destroyCentralPark(struct CentralPark* cp);

/**
 * Mark all blocks as available. Avoids reallocation.
 */
void clearCentralPark(struct CentralPark* cp);

#ifdef __cplusplus
}
#endif

#endif
