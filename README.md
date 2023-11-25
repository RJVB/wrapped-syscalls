A quick hack for Linux to work around a few common syscalls that fail for "common" reasons.

*) `sendfile(2)` and `copy_file_range(2)` can fail with *EAGAIN* on certain ZFS versions
   This condition is caught and an internal workaround is attempted that `mmap`'s the
   source file and `write(2)`s to the destination (`copyfd()` in copyfd.c).
*) `rename(2`) doesn't support cross-device operation (and Chromium ignores this).
   Set *RENAME_ACROSS_DEVICES=1* to catch this *EXDEV* condition and attempt a workaround.
   The workaround uses `sendfile(2)` (the wrapped version) for renaming a file to another file
   or to a non-existing destination, or `system("mv a b")` otherwise.

Set *SENDFILE_VERBOSE*, *COPY_FILE_RANGE_VERBOSE* or *RENAME_VERBOSE* to be informed when
the fallbacks are triggered.
