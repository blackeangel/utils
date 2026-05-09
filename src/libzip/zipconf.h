#ifndef _ZIPCONF_H
#define _ZIPCONF_H
#include <stdint.h>
#define LIBZIP_VERSION "1.10.1"
#define LIBZIP_VERSION_MAJOR 1
#define LIBZIP_VERSION_MINOR 10
#define LIBZIP_VERSION_MICRO 1
#define ZIP_UINT8_MAX  0xffu
#define ZIP_INT8_MIN   (-0x7f - 1)
#define ZIP_INT8_MAX   0x7f
#define ZIP_UINT16_MAX 0xffffu
#define ZIP_INT16_MIN  (-0x7fff - 1)
#define ZIP_INT16_MAX  0x7fff
#define ZIP_UINT32_MAX 0xffffffffu
#define ZIP_INT32_MIN  (-0x7fffffff - 1)
#define ZIP_INT32_MAX  0x7fffffff
#define ZIP_UINT64_MAX 0xffffffffffffffffull
#define ZIP_INT64_MIN  (-0x7fffffffffffffffll - 1)
#define ZIP_INT64_MAX  0x7fffffffffffffffll
#define ZIP_OFF_MAX    ZIP_INT64_MAX
#define ZIP_OFF_MIN    ZIP_INT64_MIN
typedef  int8_t    zip_int8_t;
typedef uint8_t    zip_uint8_t;
typedef  int16_t   zip_int16_t;
typedef uint16_t   zip_uint16_t;
typedef  int32_t   zip_int32_t;
typedef uint32_t   zip_uint32_t;
typedef  int64_t   zip_int64_t;
typedef uint64_t   zip_uint64_t;
#endif /* _ZIPCONF_H */
