#include_next <sys/mman.h>

#pragma once

#if defined(__linux__)
/* memfd_create and mlock2 were introduced in glibc 2.27 */
#       if !defined(__GLIBC__) || (__GLIBC__ >= 2 && __GLIBC_MINOR__ >= 27)
// all good
#       elif defined(__USE_GNU)

        /* Flags for memfd_create.  */
        # ifndef MFD_CLOEXEC
        #  define MFD_CLOEXEC 1U
        #  define MFD_ALLOW_SEALING 2U
        #  define MFD_HUGETLB 4U
        # endif

        # define MEMFD_CREATE_IS_WRAPPED
        /* Create a new memory file descriptor.  NAME is a name for debugging.
           FLAGS is a combination of the MFD_* constants.  */
        int memfd_create(const char *__name, unsigned int __flags) __THROW;

        /* Flags for mlock2.  */
        # ifndef MLOCK_ONFAULT
        #  define MLOCK_ONFAULT 1U
        # endif

        # define MLOCK2_IS_WRAPPED
        /* Lock pages from ADDR (inclusive) to ADDR + LENGTH (exclusive) into
           memory.  FLAGS is a combination of the MLOCK_* flags above.  */
        int mlock2 (const void *__addr, size_t __length, unsigned int __flags) __THROW;

        #ifndef MADV_POPULATE_READ
        // this is apparently defined since Linux 5.14 only, but it exists
        // in my glibc 2.17 headers..
        #  include <asm-generic/mman-common.h>
        #endif

#       endif
#endif
