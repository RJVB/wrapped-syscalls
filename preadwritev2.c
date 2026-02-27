#ifdef linux
// features.h gets included via sys/mman.h but unsets __USE_GNU which 
// we want to have defined so we include it first and then set the flag.
#include <features.h>
#endif

#define __USE_GNU
#include "sys/uio.h"

#ifdef PREADV2_IS_WRAPPED

#define _GNU_SOURCE

#include <unistd.h>
#include <sys/syscall.h>
#include <linux/types.h>
#include <asm/byteorder.h>

static inline void make_pos_h_l(unsigned long *pos_h, unsigned long *pos_l,
                    off_t offset)
{
#if __BITS_PER_LONG == 64
     *pos_l = offset;
     *pos_h = 0;
#else
#warning "detected 32 bit"
     *pos_l = offset & 0xffffffff;
     *pos_h = ((uint64_t) offset) >> 32;
#endif
}
// we wrap pread/writev2(2) but only if they are not provided by the local glibc library!
ssize_t preadv2 (int __fp, const struct iovec *__iovec, int __count,
               __off_t __offset, int ___flags) __wur
{
    unsigned long pos_l, pos_h;

    make_pos_h_l(&pos_h, &pos_l, __offset);
    return syscall(__NR_preadv2, __fp, __iovec, __count, pos_l, pos_h, ___flags);
}

ssize_t pwritev2 (int __fd, const struct iovec *__iodev, int __count,
                __off_t __offset, int __flags) __wur
{
    unsigned long pos_l, pos_h;

    make_pos_h_l(&pos_h, &pos_l, __offset);
    return syscall(__NR_pwritev2, __fd, __iodev, __count, pos_l, pos_h, __flags);
}

ssize_t preadv64v2 (int __fp, const struct iovec *__iovec,
                  int __count, __off64_t __offset,
                  int ___flags) __wur
{
    unsigned long pos_l, pos_h;

    make_pos_h_l(&pos_h, &pos_l, __offset);
    return syscall(__NR_preadv2, __fp, __iovec, __count, pos_l, pos_h, ___flags);
}

ssize_t pwritev64v2 (int __fd, const struct iovec *__iodev,
                   int __count, __off64_t __offset,
                   int __flags) __wur
{
    unsigned long pos_l, pos_h;

    make_pos_h_l(&pos_h, &pos_l, __offset);
    return syscall(__NR_pwritev2, __fd, __iodev, __count, pos_l, pos_h, __flags);
}
#endif
