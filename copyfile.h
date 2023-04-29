#ifndef _COPYFILE_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

/*
 * A simple clone of the Linux sendfile(2) syscall, that uses
 * mmap() to read the source data, and write(2) to copy it to
 * the destination. It probably does a lot less checking than
 * the example function.
 */
extern ssize_t copyfile(int out_fd, int in_fd, off_t *offset, size_t count);
#ifndef linux
#   define sendfile(o,i,ofs,c) copyfile((o),(i),(ofs),(c))
#endif

#define _COPYFILE_H
#endif
