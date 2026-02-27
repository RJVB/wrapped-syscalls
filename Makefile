CC = gcc
DEFINES = -DWRAP_SYSCALLS=2 -DBUILD_AS_LIBRARY
CFLAGS = -Os -g

SOURCES = validate_symbol.c \
	sendfile.c \
	copyfd.c \
	copy_file_range.c \
	rename.c \
	get_process_name.c \
	memfd_create.c \
	mlock2.c \
	preadwritev2.c

libwrapped_syscalls.so : $(SOURCES)
	$(CC) -o $@ -fPIC -shared $(DEFINES) $(CFLAGS) $? -ldl

install: libwrapped_syscalls.so
	mkdir -p ${DESTDIR}${PREFIX}/lib
	cp -puv $? ${DESTDIR}${PREFIX}/lib
	mkdir -p ${DESTDIR}${PREFIX}/include
	cp -pruvd sys linux ${DESTDIR}${PREFIX}/include
