#ifdef linux
// features.h gets included via sys/mman.h but unsets __USE_GNU which 
// we want to have defined so we include it first and then set the flag.
#include <features.h>
#endif

#define __USE_GNU
#include "sys/mman.h"

#ifdef MLOCK2_IS_WRAPPED

#define _GNU_SOURCE

#include <unistd.h>
#include <sys/syscall.h>

// we wrap mlock2(2) but only if it is not provided by the local glibc library!
int mlock2(const void *__addr, size_t __length, unsigned int __flags)
{
    return syscall(__NR_mlock2, __addr, __length, __flags);
}

#endif
