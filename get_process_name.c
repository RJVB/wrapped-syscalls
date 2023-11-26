#include <stdio.h>
#include <unistd.h>

#include "get_process_name.h"

char *get_process_name(pid_t pid, char *buf, size_t len)
{
    char exename[256];
    snprintf(exename, sizeof(exename), "/proc/%d/exe", pid);
#ifdef linux
    ssize_t len2 = readlink(exename, buf, len);
    if (len2 < len ) {
        if (len2 < 0) {
            // we're not really interested in why (for now)
            len2 = 0;
        }
        buf[len2] = '\0';
    } else {
        buf[len-1] = '\0';
    }
#else
    buf[0] = '\0';
#endif
    return buf;
}

