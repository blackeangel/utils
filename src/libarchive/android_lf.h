/* android_lf.h — large-file support stub for Android NDK
 * On modern Android/Bionic, large file support is enabled by default.
 * This stub satisfies the #include without redefining anything.
 */
#pragma once
#if defined(__ANDROID__)
#  ifndef _FILE_OFFSET_BITS
#    define _FILE_OFFSET_BITS 64
#  endif
#  ifndef _LARGEFILE_SOURCE
#    define _LARGEFILE_SOURCE 1
#  endif
#endif
