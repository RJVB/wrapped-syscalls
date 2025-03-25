#pragma once

#include <dlfcn.h>

#define LOAD_SYMBOL(sym,name) ((sym=dlsym(RTLD_NEXT,name)) || (sym=dlsym(RTLD_DEFAULT,name)))

// check if we obtained an address for <name> that is not our own overload for it
// and correct it if necessary (assuming it's a symbol from libc)
int validate_symbol(void **found, const char *name, void *overload);
