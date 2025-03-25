#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUF_SIZE 4096*1000

int main(int argc, char **argv)
{
    char buf[BUF_SIZE];
    const char *fromfile = argv[1];
    const char *tofile = argv[2];
    const char *tempfile = "/tmp/readwrite.out";
    struct stat stat_buf;
    int fromfd = open(fromfile, O_RDONLY);
    if (fromfd < 0) {
        fputs("Error opening input ", stderr);
        perror(fromfile);
        exit(-1);
    }
    fstat(fromfd, &stat_buf);
    int tofd = open(tempfile, O_WRONLY | O_CREAT, stat_buf.st_mode);
    if (tofd < 0) {
        fputs("Error opening output ", stderr);
        perror(tempfile);
        exit(-1);
    }
    int n;
    while ((n = read(fromfd, &buf, sizeof(buf))) > 0) {
        write(tofd, &buf, n);
    }
    close(fromfd);
    close(tofd);
    rename(tempfile, tofile);
}
