#include <errno.h>

#include "copyfile.h"

ssize_t copyfile(int out_fd, int in_fd, off_t *offset, size_t count)
{
    ssize_t n = -1;
    struct stat stat_buf;

    if (count < 0) {
        errno = EINVAL;
        return -1;
    }
    if (fstat(in_fd, &stat_buf) < 0) {
        return -1;
    }
    off_t curpos = lseek(in_fd, 0, SEEK_CUR);
    if (!count) {
        if (!offset) {
            count = stat_buf.st_size - curpos;
        } else if(*offset + curpos < stat_buf.st_size) {
            count = stat_buf.st_size - *offset - curpos;
        } else {
            errno = EINVAL;
            return -1;
        }
    }
    size_t remaining;
    if (offset) {
        if (*offset > stat_buf.st_size) {
            errno = EINVAL;
            return -1;
        }
        remaining = stat_buf.st_size - *offset;
    } else {
        remaining = stat_buf.st_size - curpos;
    }
    if (count > remaining) {
        count = remaining;
    }
    if (count > 0) {
        char *src = mmap(NULL, count, PROT_READ, MAP_PRIVATE, in_fd, (offset)? *offset : curpos);
        if (src != MAP_FAILED) {
            n = write(out_fd, src, count);
            munmap(src, count);
            if (n > 0) {
                if (offset) {
                    *offset += n;
                } else {
                    lseek(in_fd, n, SEEK_CUR);
                }
            }
        }
    } else {
        n = 0;
        if (offset) {
            *offset = stat_buf.st_size;
        } else {
            lseek(in_fd, 0, SEEK_END);
        }
    }
    return n;
}
