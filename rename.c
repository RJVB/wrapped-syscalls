#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#if WRAP_SYSCALLS
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <unistd.h>
#   include <fcntl.h>
#   include <dlfcn.h>
#   include <sys/syscall.h>
#   ifdef linux
#       include <sys/sendfile.h>
#   else
#       include "copyfd.h"
#   endif

static int (*__real_rename)(const char *old, const char *new);

static void init_rename() __attribute__((constructor)) ;
static void init_rename()
{
    if (!(__real_rename = dlsym(RTLD_NEXT, "rename"))) {
        fputs(__FUNCTION__, stderr);
        perror(" couldn't overload rename(2)");
        abort();
    }
    if (getenv("RENAME_DEBUG")) {
        fputs(__FUNCTION__, stderr);
        fprintf(stderr, ": rename(2) wrapped with a fallback to handle cross-device renames\n");
    }
}

int rename(const char *oldf, const char *newf)
{
    errno = 0;
    int n;
    if ((n = __real_rename(oldf, newf)) < 0) {
        int verbose = getenv("RENAME_VERBOSE") != NULL;
        int doRAD = getenv("RENAME_ACROSS_DEVICES") != NULL;
        struct stat sbo;
        int statret, stat_errno;
        const char *_oldf_ = oldf? oldf : "<nullptr!>";
        const char *_newf_ = newf? newf : "<nullptr!>";
        int rename_errno = errno;
        // get the source file information, used further down and here
        // in order to determine if it even exists. Cache the errno info.
        errno = 0;
        statret = oldf? stat(oldf, &sbo) : -1;
        stat_errno = errno;
        errno = rename_errno;
        // inform the user even if we're dealing with a vulgar file-not-exist error.
        // While that's not our concern in this wrapper the calling code might report
        // the situation we're supposed to work around and we don't want to leave the
        // impression we're not doing our job.
        if (verbose && ((errno != ENOENT && stat_errno != ENOENT) || doRAD)) {
            fprintf(stderr, "syscall rename(%s,%s) failed with error %d: ", _oldf_, _newf_, errno);
            perror("");
        }
        if (errno == EXDEV && doRAD && oldf && newf) {
            struct stat sbn;
            if (statret == 0 && S_ISREG(sbo.st_mode)) {
                errno = 0;
                int ret = stat(newf, &sbn);
                if ( (ret < 0 && errno == ENOENT) || (ret == 0 && S_ISREG(sbn.st_mode))) {
                    errno = 0;
                    int fromfd = open(oldf, O_RDONLY);
                    int tofd = (fromfd > 0) ? open(newf, O_WRONLY | O_CREAT | O_TRUNC, sbo.st_mode) : -1;
                    int written;
                    if (fromfd < 0 || tofd < 0) {
                        if (verbose) {
                            fprintf(stderr, "failed to open \"%s\"(%d) or \"%s\"(%d): ", oldf, fromfd, newf, tofd);
                            perror("");
                        }
                        if (fromfd > 0) {
                            close(fromfd);
                        } else if (tofd > 0) {
                            close(tofd);
                        }
                    } else {
                        do {
                            written = sendfile(tofd, fromfd, NULL, sbo.st_size);
                        } while (written > 0);
                        if (written == 0) {
                            close(tofd), close(fromfd);
                            unlink(oldf);
                            fprintf(stderr, "successful cross-device rename \"%s\" -> \"%s\" using sendfile(2)\n", oldf, newf);
                            return 0;
                        } else if (verbose) {
                            fprintf(stderr, "cross-device rename \"%s\" -> \"%s\" using sendfile(2) failed: ", oldf, newf);
                            perror("");
                        }
                    }
                }
            }
            // fallback unless `old` doesn't exist
            if (!(statret < 0 && stat_errno == ENOENT)) {
                // kludgy workaround for code that assumes rename(2) works across devices...
                // note that we have to unset RENAME_ACROSS_DEVICES!!
                char *command = NULL;
#ifdef _darwin_
                // the system `env` command doesn't support the -u option, so assume the version from MacPorts is on the path.
                if (asprintf(&command, "genv -u RENAME_ACROSS_DEVICES RENAME_VERBOSE=1 mv \"%s\" \"%s\"", oldf, newf) > 0)
#else
                if (asprintf(&command, "env -u RENAME_ACROSS_DEVICES RENAME_VERBOSE=1 mv \"%s\" \"%s\"", oldf, newf) > 0)
#endif
                {
                    int status = system(command);
                    if (!(n = WEXITSTATUS(status)) && verbose) {
                        fprintf(stderr, "successful cross-device rename via >mv \"%s\" \"%s\"\n", oldf, newf);
                    }
                    free(command);
                }
            } else {
                fprintf(stderr, "non-existing \"%s\" cannot be renamed cross-devices or otherwise\n", oldf);
            }
        }
    }
    return n;
}
#endif


#ifndef BUILD_AS_LIBRARY
int main(int argc, char **argv)
{
    const char *fromfile = argv[1];
    const char *tofile = argv[2];
    errno = 0;
    if (rename(fromfile, tofile) < 0) {
        perror("rename(2) failed");
    }
}
#endif
