#pragma once
/* libarchive config.h for Android NDK (ndk-build only)
 * cmake builds use find_package / FetchContent instead — this file
 * is only compiled when building via Android.mk / ndk-build.
 */

#define __LIBARCHIVE_CONFIG_H_INCLUDED 1
#define __LIBARCHIVE_BUILD 1

#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <wchar.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define LIBARCHIVE_VERSION_NUMBER 3007004
#define LIBARCHIVE_VERSION_STRING "3.7.4"

/* Standard headers */
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_STDINT_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_STDBOOL_H 1
#define HAVE_STDDEF_H 1
#define HAVE_FCNTL_H 1
#define HAVE_UNISTD_H 1
#define HAVE_ERRNO_H 1
#define HAVE_LIMITS_H 1
#define HAVE_LOCALE_H 1
#define HAVE_TIME_H 1
#define HAVE_WCHAR_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_IOCTL_H 1
#define HAVE_SYS_UTSNAME_H 1
#define HAVE_LANGINFO_H 1
#define HAVE_DIRENT_H 1
#define HAVE_GRP_H 1
#define HAVE_PWD_H 1
#define HAVE_POLL_H 1
#define HAVE_GLOB_H 1

/* Compression backends */
#define HAVE_ZLIB_H 1
#define HAVE_LIBZ 1
#define HAVE_LZMA_H 1
#define HAVE_LIBLZMA 1
#define HAVE_DECL_LZMA_STREAM_ENCODER_MT 0

/* POSIX functions */
#define HAVE_STRUCT_STAT 1
#define HAVE_STRUCT_TM_TM_GMTOFF 1
#define HAVE_FSTAT 1
#define HAVE_LSTAT 1
#define HAVE_STAT 1
#define HAVE_LSEEK 1
#define HAVE_MEMCPY 1
#define HAVE_MEMMOVE 1
#define HAVE_MEMSET 1
#define HAVE_MKDIR 1
#define HAVE_MKSTEMP 1
#define HAVE_STRDUP 1
#define HAVE_STRERROR 1
#define HAVE_STRFTIME 1
#define HAVE_SNPRINTF 1
#define HAVE_VPRINTF 1
#define HAVE_VASPRINTF 1
#define HAVE_WCSCPY 1
#define HAVE_WCSLEN 1
#define HAVE_WCSNLEN 0
#define HAVE_STRNCPY_S 0
#define HAVE_GETPID 1
#define HAVE_GETEUID 1
#define HAVE_READLINK 1
#define HAVE_SYMLINK 1
#define HAVE_LINK 1
#define HAVE_MKNOD 1
#define HAVE_MKFIFO 1
#define HAVE_CHMOD 1
#define HAVE_FCHMOD 1
#define HAVE_CHOWN 1
#define HAVE_FCHOWN 1
#define HAVE_LCHOWN 1
#define HAVE_PIPE 1
#define HAVE_POLL 1
#define HAVE_SELECT 1
#define HAVE_UTIME 1
#define HAVE_UTIMES 1
#define HAVE_FUTIMES 1
#define HAVE_LUTIMES 1
#define HAVE_FCHDIR 1
#define HAVE_OPENAT 1
#define HAVE_FDOPENDIR 1
#define HAVE_DIRFD 1

#define SIZEOF_INT    4
#define SIZEOF_LONG   8
#define SIZEOF_SIZE_T __SIZEOF_POINTER__
#define HAVE_DECL_INT64_MAX 1
#define HAVE_DECL_INT64_MIN 1
#define HAVE_DECL_UINT32_MAX 1
#define HAVE_DECL_SIZE_MAX 1

/* No iconv, no crypto */
#define NO_ICONV 1
#undef HAVE_ICONV
#undef HAVE_OPENSSL_EVP_H
#undef HAVE_LIBCRYPTO
#undef HAVE_BZLIB_H
#undef HAVE_LIBBZ2
#undef HAVE_LZ4_H
#undef HAVE_LIBLZ4
#undef HAVE_ZSTD_H
#undef HAVE_LIBZSTD
