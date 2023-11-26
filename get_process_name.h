#ifndef GET_PROCESS_NAME_H

#include <sys/types.h>

extern char *get_process_name(pid_t pid, char *buf, size_t len);

#define GET_PROCESS_NAME_H
#endif
