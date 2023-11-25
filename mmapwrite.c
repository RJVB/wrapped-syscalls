#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE 4096*1000

#include "copyfd.c"

int main(int argc, char **argv)
{
    const char *fromfile = argv[1];
    const char *tofile = argv[2];
    struct stat stat_buf;

    int fromfd = open(fromfile, O_RDONLY);
    fstat(fromfd, &stat_buf);

    int tofd = open(tofile, O_WRONLY | O_CREAT | O_TRUNC, stat_buf.st_mode);    int n = 1;

    while (n > 0) {
       errno = 0;
        n = copyfd(tofd, fromfd, 0, BUF_SIZE);
       if (n < 0) {
           perror("copyfd error");
       }
    }
    close(tofd);
    close(fromfd);
}
