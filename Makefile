CC = gcc
DEFINES = -DWRAP_SYSCALLS=2 -DBUILD_AS_LIBRARY
CFLAGS = -Os -g

SOURCES = validate_symbol.c \
	sendfile.c \
	copyfd.c \
	copy_file_range.c \
	rename.c \
	get_process_name.c \
	memfd_create.c

libwrapped_syscalls.so : $(SOURCES)
	$(CC) -o $@ -fPIC -shared $(DEFINES) $(CFLAGS) $? -ldl
