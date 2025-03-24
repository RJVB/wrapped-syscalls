// features.h gets included via sys/mman.h but unsets __USE_GNU which 
// we want to have defined so we include it first and then set the flag.
#include <features.h>

#define __USE_GNU
#include "sys/mman.h"

#ifdef MEMFD_CREATE_IS_WRAPPED

#define _GNU_SOURCE

#include <unistd.h>
#include <sys/syscall.h>

// we wrap memfd_create(2) but only if it is not provided by the local glibc library!
int memfd_create(const char *__name, unsigned int __flags)
{
    return syscall(__NR_memfd_create, __name, __flags);
}

#endif
