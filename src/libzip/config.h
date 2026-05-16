#pragma once
#define PACKAGE "libzip"
#define VERSION "1.10.1"
#define HAVE_STDINT_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_STDBOOL_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_STDLIB_H 1
#define HAVE_UNISTD_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_FCNTL_H 1
#define HAVE_CLONEFILE 0
#define HAVE_FICLONERANGE 0
#define HAVE_EXPLICIT_BZERO 0
#define HAVE_EXPLICIT_MEMSET 0
#define HAVE_ARC4RANDOM 0
#define HAVE_LOCALTIME_R 1
#define HAVE_LOCALTIME_S 0
#define HAVE_OPEN_TEMP_FILE 0
#define HAVE_SETMODE 0
#define HAVE_FDOPEN 1
#define HAVE_FILENO 1
#define HAVE_MKSTEMP 1
#define HAVE_FSEEKO 1
#define HAVE_FTELLO 1
#define HAVE_NULLABLE 0
#define SIZEOF_SIZE_T __SIZEOF_POINTER__
/* Backends: only zlib */
#define HAVE_ZLIB_H 1
#define HAVE_LIBBZ2 0
#define HAVE_LIBLZMA 0
#define HAVE_LIBZSTD 0
#define HAVE_LIBLZ4 0
/* No crypto */
#define HAVE_CRYPTO 0
#define HAVE_OPENSSL 0
#define HAVE_GNUTLS 0
#define HAVE_MBEDTLS 0
#define HAVE_WINDOWS_CRYPTO 0
/* No AES */
#define ZIP_ENCRYPTION_SUPPORT_AES 0
#ifdef _WIN32
# define HAVE_WINDOWS_H 1
# define HAVE__SETMODE 1
# define HAVE__SNWPRINTF_S 1
# undef  HAVE_FDOPEN
# undef  HAVE_FSEEKO
# undef  HAVE_FTELLO
# undef  HAVE_LOCALTIME_R
# define HAVE_FSEEKI64 1
# define HAVE_FTELLI64 1
#endif
