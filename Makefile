CC = gcc
DEFINES = -DWRAP_SYSCALLS=2 -DBUILD_AS_LIBRARY
CFLAGS = -Os -g

libwrapped_syscalls.so : sendfile.c copyfile.c copy_file_range.c rename.c
	$(CC) -o $@ -fPIC -shared $(DEFINES) $(CFLAGS) $? -ldl
