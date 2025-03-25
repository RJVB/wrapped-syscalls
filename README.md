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

*) Also provide a first function that is missing from "too old" [g]libc : memfd_create(2).
   For this, a partial rewrite was required to allow linking the library with a dependent
   rather than supporting only the use with LD_PRELOAD. The symbol lookup logic is now:
   1) use dlsym(RTLD_NEXT,"foo") which will succeed in the LD_PRELOAD case (and now look
      in "ourselves"
   2) on failure, use dlsym(RTLD_DEFAULT,"foo") which will scan the entire in-memory image
      and apparently still find the symbol in libc if provided by that library.
   3) call validate_symbol() which validates the found symbol against the specified wrapper
      and if they're equal, does another lookup, explicitly in libc.

NB: the code will build on Darwin too; this will provide a working rename(2) wrapper as well
as functional implementations of sendfile(2) and copy_file_range(2) with the same additional
features as their wrappers on Linux.
