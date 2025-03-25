#include "validate_symbol.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int validate_symbol(void **found, const char *name, void *overload)
{
//     fprintf(stderr, "%s: comparing found %p to overload %p for function \"%s\"\n",
//             __PRETTY_FUNCTION__, *found, overload, name);
    if (*found == overload) {
#ifdef linux
        void *libc = dlopen("libc.so", RTLD_LAZY|RTLD_LOCAL);
        // libc.so can be an ASCII loader script which dlopen() won't grok
        if (!libc) {
            libc = dlopen("libc.so.6", RTLD_LAZY|RTLD_LOCAL);
        }
#elif defined(__APPLE__)
        void *libc = dlopen("libc.dylib", RTLD_LAZY|RTLD_LOCAL);
#endif
        if (!libc) {
            fprintf(stderr, "%s: Error loading libc: %s\n", __PRETTY_FUNCTION__, dlerror());
            return 0;
        } else {
            dlerror();
            *found = dlsym(libc, name);
            dlclose(libc);
            if (!*found || *found == overload) {
                *found = NULL;
                return 0;
            }
        }
    }
    return 1;
}
