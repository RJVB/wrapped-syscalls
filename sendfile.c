#ifdef linux
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/mman.h>
#define _GNU_SOURCE
#define __USE_GNU
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#if WRAP_SYSCALLS == 2
#   include <dlfcn.h>
#   include <sys/syscall.h>
#endif

#include "copyfd.h"
#include "get_process_name.h"

#define BUF_SIZE 4096*1000

// build with `-DWRAP_SYSCALLS=1 -Wl,wrap,sendfile` to wrap the sendfile syscall in an executable
// build with `-DWRAP_SYSCALLS=2 -DBUILD_AS_LIBRARY -ldl` to wrap the sendfile syscall as an overload library

#if WRAP_SYSCALLS == 1
ssize_t __real_sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
#elif WRAP_SYSCALLS == 2
static ssize_t (*__real_sendfile)(int out_fd, int in_fd, off_t *offset, size_t count) = NULL;

#ifdef __NR_sendfile
static ssize_t syscall_sendfile(int out_fd, int in_fd, off_t *offset, size_t count)
{
#ifdef __NR_sendfile64
    return syscall(__NR_sendfile64, out_fd, in_fd, offset, count);
#else
    return syscall(__NR_sendfile, out_fd, in_fd, offset, count);
#endif
}
#endif

static void init_sendfile() __attribute__((constructor)) ;
static void init_sendfile()
{
    if (!(__real_sendfile = dlsym(RTLD_NEXT, "sendfile"))) {
#ifdef __NR_sendfile
        // libc (?) doesn't have the sendfile function, create our own. Sadly we
        // cannot seem to obtain the address of the syscall directly.
        __real_sendfile = syscall_sendfile;
        if (getenv("SENDFILE_DEBUG")) {
            fprintf(stderr, "sendfile(2) mapped to syscall(__NR_sendfile,...) wrapper\n");
        }
#else
        fputs(__FUNCTION__, stderr);
        perror(" couldn't overload sendfile(2)");
        abort();
#endif
    }
    if (getenv("SENDFILE_DEBUG")) {
        fputs(__FUNCTION__, stderr);
        pid_t self = getpid();
        char exename[1024];
        fprintf(stderr, " [pid %d=%s]: sendfile(2) wrapped with a fallback to handle EAGAIN situations\n",
            self, get_process_name(self, exename, sizeof(exename)));
    }
}
#else
#define __real_sendfile sendfile
#endif

#if WRAP_SYSCALLS < 2
ssize_t __wrap_sendfile(int out_fd, int in_fd, off_t *offset, size_t count)
#else
ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count)
#endif
{
    errno = 0;
    ssize_t n = __real_sendfile(out_fd, in_fd, offset, count);
#ifdef DEBUG
    if (getenv("SENDFILE_DEBUG")) {
        pid_t self = getpid();
        char exename[1024];
        fprintf(stderr, "[pid %d=%s]: sendfile(%d,%d,%p,%lu)=%lu errno=%d\n",
            self, get_process_name(self, exename, sizeof(exename)),
            out_fd, in_fd, offset, count, n, errno);
    }
#endif
    if (n < 0 && errno == EAGAIN) {
#if WRAP_SYSCALLS == 2
        if (getenv("SENDFILE_VERBOSE")) {
            fprintf(stderr, "sendfile(2) returned EAGAIN; trying with mmap()+write()\n");
        }
#endif
        errno = 0;
        ssize_t m = copyfd(out_fd, in_fd, offset, count);
        if (m >= 0) {
            n = m;
        }
    }
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
        n = sendfile(tofd, fromfd, 0, BUF_SIZE);
       if (n < 0) {
           perror("Sendfile error");
       }
    }
    close(tofd);
    close(fromfd);
}
#endif

#else

#include <errno.h>

#include "copyfd.h"
#undef sendfile

ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count)
{
    errno = 0;
    return copyfd(out_fd, in_fd, offset, count);
}

#endif
