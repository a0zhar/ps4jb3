#include <stdio.h>
#include <stddef.h>

#ifdef __PS4__
#include <ps4/mmap.h>
#else
#include <sys/mman.h>
#endif

#include "extcall.h" 

#define NEWSTACK_MAP_SIZE 65536
int pthread_create(void** retval, void* attr, void*(*start_routine)(void*), void* arg);
int pthread_create__rop(void** retval, void* attr, void* (*start_routine)(void*), void* arg) {
    // Create a new mapped memory region with READ and WRITE permissions.
    // and check if it failed or not
    char* new_stack = mmap(0, NEWSTACK_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (new_stack == MAP_FAILED) return -1;
    
    // Increase the offset of our mapped region by 65536 bytes.
    new_stack += NEWSTACK_MAP_SIZE;
    
    int extcall_sz = (sizeof(extcall_t) + 15) & ~15;
    // Create a pointer that points to the location in memory 
    // which is <extcall_sz> bytes before the new_stack.
    extcall_t* x = (extcall_t*)(new_stack - extcall_sz);
    create_extcall(*x, start_routine, new_stack - extcall_sz - 16, arg);
    return pthread_create(retval, attr, extcall_gadget, *x);
}
/*int pthread_create__rop(void** retval, void* attr, void* (*start_routine)(void*), void* arg) {
    char* new_stack = mmap(0, 65536, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0) + 65536;
    int extcall_sz = sizeof(extcall_t);
    extcall_sz -= 1;
    extcall_sz |= 15;
    extcall_sz += 1;
    extcall_t* x = (extcall_t*)(new_stack - extcall_sz);
    create_extcall(*x, start_routine, new_stack - extcall_sz - 16, arg);
    return pthread_create(retval, attr, extcall_gadget, *x);
}*/
