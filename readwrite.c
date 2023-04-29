#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUF_SIZE 4096*1000

int main(int argc, char **argv)
{
    char buf[BUF_SIZE];
    const char *fromfile = argv[1];
    const char *tofile = argv[2];
    struct stat stat_buf;
    int fromfd = open(fromfile, O_RDONLY);
    fstat(fromfd, &stat_buf);
    int tofd = open(tofile, O_WRONLY | O_CREAT, stat_buf.st_mode);    int n;
    while ((n = read(fromfd, &buf, sizeof(buf))) > 0) {
        write(tofd, &buf, n);
    }
}
