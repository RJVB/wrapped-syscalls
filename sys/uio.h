#include_next <sys/uio.h>

#pragma once

#if defined(__linux__)
/* preadv2/pwritev2 were introduced in Linux 4.6 (20160515)
 * The RWF_NOWAIT flag was introduced in 4.14 (171112)
 * The wrappers were added only later to glibc 2.26!
 */
#       if !defined(__GLIBC__) || (__GLIBC__ >= 2 && __GLIBC_MINOR__ >= 26)
// all good
#       elif defined(__USE_GNU)
        /* Flags for preadv2/pwritev2.  */
        #define RWF_HIPRI   0x00000001 /* High priority request.  */
        #define RWF_DSYNC   0x00000002 /* per-IO O_DSYNC.  */
        #define RWF_SYNC    0x00000004 /* per-IO O_SYNC.  */
        #define RWF_NOWAIT  0x00000008 /* per-IO nonblocking mode.  */
        #define RWF_APPEND  0x00000010 /* per-IO O_APPEND.  */

        # define PREADV2_IS_WRAPPED
        # define PWRITEV2_IS_WRAPPED

        /* Same as preadv but with an additional flag argumenti defined at uio.h.  */
        extern ssize_t preadv2 (int __fp, const struct iovec *__iovec, int __count,
                       __off_t __offset, int ___flags) __wur;

        /* Same as preadv but with an additional flag argument defined at uio.h.  */
        extern ssize_t pwritev2 (int __fd, const struct iovec *__iodev, int __count,
                        __off_t __offset, int __flags) __wur;

        # ifdef __USE_LARGEFILE64
        /* Same as preadv but with an additional flag argumenti defined at uio.h.  */
        extern ssize_t preadv64v2 (int __fp, const struct iovec *__iovec,
                          int __count, __off64_t __offset,
                          int ___flags) __wur;

        /* Same as preadv but with an additional flag argument defined at uio.h.  */
        extern ssize_t pwritev64v2 (int __fd, const struct iovec *__iodev,
                           int __count, __off64_t __offset,
                           int __flags) __wur;
        # endif
#       endif
#endif
