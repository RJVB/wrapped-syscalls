#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/syscall.h>
#include "get_process_name.h"

#define BUF_SIZE 4096*1000

#ifdef linux

// build with `-DWRAP_COPY_FILE_RANGE -DBUILD_AS_LIBRARY -ldl` to wrap the copy_file_range syscall as an overload library

static ssize_t (*__real_copy_file_range)(int fd_in, off64_t *off_in, int fd_out, off64_t *off_out, size_t len, unsigned int flags) = NULL;

#ifdef __NR_copy_file_range
static ssize_t syscall_copy_file_range(int fd_in, off64_t *off_in, int fd_out, off64_t *off_out, size_t len, unsigned int flags)
{
    return syscall(__NR_copy_file_range, fd_in, off_in, fd_out, off_out, len, flags);
}
#endif

static void init_copy_file_range() __attribute__((constructor)) ;
static void init_copy_file_range()
{
    dlerror();
    if (!(__real_copy_file_range = dlsym(RTLD_NEXT, "copy_file_range"))
        && !(__real_copy_file_range = dlsym(RTLD_DEFAULT, "copy_file_range"))
    ) {
#ifdef __NR_copy_file_range
        // libc (?) doesn't have the copy_file_range function, create our own. Sadly we
        // cannot seem to obtain the address of the syscall directly.
        __real_copy_file_range = syscall_copy_file_range;
        if (getenv("COPY_FILE_RANGE_DEBUG")) {
            fputs(__FUNCTION__, stderr);
            fprintf(stderr, ": copy_file_range(2) mapped to syscall(__NR_copy_file_range,...) wrapper\n");
        }
#else
        fprintf(stderr, "%s couldn't overload copy_file_range(2) (%s)\n", __PRETTY_FUNCTION__, dlerror());
        abort();
#endif
    }
    if (getenv("COPY_FILE_RANGE_DEBUG")) {
        fputs(__PRETTY_FUNCTION__, stderr);
        pid_t self = getpid();
        char exename[1024];
        fprintf(stderr, " [pid %d=%s]: copy_file_range(2) wrapped with a fallback to handle EAGAIN situations\n",
                self, get_process_name(self, exename, sizeof(exename)));
    }
}
#endif

#ifdef __APPLE__
typedef off_t off64_t;
#endif

ssize_t copy_file_range(int fd_in, off64_t *off_in, int fd_out, off64_t *off_out, size_t len, unsigned int flags)
{
    errno = 0;
    ssize_t n;
#ifdef linux
    if ((n = __real_copy_file_range(fd_in, off_in, fd_out, off_out, len, flags)) < 0 
            && (errno == EAGAIN || errno == EXDEV)) {
        if (getenv("COPY_FILE_RANGE_VERBOSE")) {
            fprintf(stderr, "copy_file_range(2) returned %s; trying with mmap()+write()\n",
                (errno == EAGAIN)? "EAGAIN" : "EXDEV");
        }
#endif
        errno = 0;
        struct stat stb1, stb2;
        if (fstat(fd_in, &stb1) < 0 || fstat(fd_out, &stb2) < 0) {
            return -1;
        }
        size_t remains1;
        off_t curpos1 = lseek(fd_in, 0, SEEK_CUR);
        if (off_in) {
            if (*off_in > stb1.st_size) {
                errno = EINVAL;
                return -1;
            }
            remains1 = stb1.st_size - *off_in;
        } else {
            remains1 = stb1.st_size - curpos1;
        }
        if (len > remains1) {
            len = remains1;
        }
        if (len > 0) {
            // we should be good now w.r.t. the ranges, including the overlap condition
            // in case of reading and writing to the same file. The syscall would have
            // failed with a different errno if those had been wrong.
            char *src = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd_in, (off_in)? *off_in : curpos1);
            if (src != MAP_FAILED) {
                if (off_out) {
                    n = pwrite(fd_out, src, len, *off_out);
                } else {
                    n = write(fd_out, src, len);
                }
                munmap(src, len);
                if (n > 0) {
                    if (off_in) {
                        *off_in += n;
                    } else {
                        lseek(fd_in, n, SEEK_CUR);
                    }
                    if (off_out) {
                        *off_out += n;
                    }
                }
            }
        } else {
            n = 0;
            if (off_in) {
                *off_in = stb1.st_size;
            } else {
                lseek(fd_in, 0, SEEK_END);
            }
        }
        // n is not changed in case MAP_FAILED
#ifdef linux
    }
#endif
    return n;
}

#ifndef BUILD_AS_LIBRARY
int main(int argc, char **argv)
{
    const char *fromfile = argv[1];
    const char *tofile = argv[2];
    struct stat stat_buf;

    int fromfd = open(fromfile, O_RDONLY);
    fstat(fromfd, &stat_buf);

    int tofd = open(tofile, O_WRONLY | O_CREAT | O_TRUNC, stat_buf.st_mode);    int n = 1;

    while (n > 0) {
        n = copy_file_range(fromfd, 0, tofd, 0, BUF_SIZE, 0);
       if (n < 0) {
           perror("copy_file_range() error");
       }
    }
    close(tofd);
    close(fromfd);
}
#endif
