#include_next <linux/falloc.h>

#pragma once

#if defined(__linux__)
/* FALLOC_FL_COLLAPSE_RANGE was introduced Linux 3.15 (20140608) => glibc 2.20 */
// idem for FALLOC_FL_ZERO_RANGE
#       if !defined(__GLIBC__) || (__GLIBC__ >= 2 && __GLIBC_MINOR__ >= 20)
// all good
#       elif defined(__USE_GNU)

        /* Flags for fallocate()  */
        # ifndef FALLOC_FL_COLLAPSE_RANGE
        #  define FALLOC_FL_COLLAPSE_RANGE   0x08
        #  define FALLOC_FL_ZERO_RANGE       0x10
        # endif

#       endif
/* FALLOC_FL_INSERT_RANGE was introduced in Linux 4.1 (20150621) => glibc 2.22 */
#       if !defined(__GLIBC__) || (__GLIBC__ >= 2 && __GLIBC_MINOR__ >= 22)
// all good
#       elif defined(__USE_GNU)

        /* Flags for fallocate()  */
        # ifndef FALLOC_FL_INSERT_RANGE
        #  define FALLOC_FL_INSERT_RANGE          0x20
        # endif

#       endif
#endif
