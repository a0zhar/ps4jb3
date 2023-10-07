#pragma once
#include <sys/types.h>

#define PAGE_SIZE 16384 // (16 * 1024)

// 
// Protections are chosen from these bits, or-ed together 
//
#define	PROT_NONE   0x00 // no permissions
#define	PROT_READ   0x01 // pages can be read
#define	PROT_WRITE  0x02 // pages can be written
#define	PROT_EXEC   0x04 // pages can be executed

#define PROT_CPU_READ 1
#define PROT_CPU_WRITE 2
#define PROT_CPU_EXEC 4
#define PROT_GPU_EXEC 8
#define PROT_GPU_READ 16
#define PROT_GPU_WRITE 32

//
// Mapping types
//
#define	MAP_FILE	  0x0000   // map from file (default) 
#define	MAP_ANON	  0x1000   // allocated from memory, swap space 
#define	MAP_ANONYMOUS     MAP_ANON // For compatibility. allocated from memory, swap space 
#define	MAP_SHARED	  0x0001   // share changes 
#define	MAP_PRIVATE	  0x0002   // changes are private 
#define MAP_TYPE          0xf
#define	MAP_FIXED	  0x0010   // map addr must be exactly as requested 
#define MAP_32BIT         0x80000

#define MAP_FAILED	((void *)-1) // error return from mmap()

// 
// msync() flags
//
#define	MS_SYNC	       0x0000 // msync synchronously
#define MS_ASYNC       0x0001 // return immediately
#define MS_INVALIDATE  0x0002 // invalidate all cached data

struct memoryRegionInfo {
	void* base; // 0x0
	void* end; // 0x8
	unsigned int flags; // 0x16
};

struct otherMemoryRegionInfo {
	void* base; // 0x0
	void* end; // 0x8
	char unknown[0xa]; // 0x16
	char name[32]; // 0x20
};

// allocate memory, or map files or devices into memory
void* mmap(void* addr, size_t len, int prot, int flags, int fd, off_t offset);
// remove a mapping
int munmap(void* addr, size_t len);
int mprotect(void* addr, size_t len, int prot);
int msync(void* addr, size_t len, int flags);
int mlock(void* addr, size_t len);
int munlock(void* addr, size_t len);

#define getMemoryInfo      query_memory_protection
#define getOtherMemoryInfo virtual_query

int getMemoryInfo(void* address, struct memoryRegionInfo* destination);
int getOtherMemoryInfo(void* address, int nextMatchIfUnmapped, struct otherMemoryRegionInfo* destination);
