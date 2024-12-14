LOCAL_PATH := $(call my-dir)

##############################################################################
# libz
##############################################################################
include $(CLEAR_VARS)

LOCAL_MODULE := libz

LOCAL_CFLAGS := \
    -DHAVE_HIDDEN \
    -DZLIB_CONST

LOCAL_C_INCLUDES := \
    src/zlib  \
    src/e2fsdroid/ext2fs

LOCAL_SRC_FILES += \
    src/zlib/adler32.c \
    src/zlib/compress.c \
    src/zlib/crc32.c \
    src/zlib/deflate.c \
    src/zlib/gzclose.c \
    src/zlib/gzlib.c \
    src//zlib/gzread.c \
    src/zlib/gzwrite.c \
    src/zlib/infback.c \
    src/zlib/inffast.c \
    src/zlib/inflate.c \
    src/zlib/inftrees.c \
    src/zlib/trees.c \
    src/zlib/uncompr.c \
    src/zlib/zutil.c

include $(BUILD_STATIC_LIBRARY)

##############################################################################
# sparse
##############################################################################
include $(CLEAR_VARS)

LOCAL_MODULE := libsparse

LOCAL_CFLAGS := \
    -DHAVE_HIDDEN \
    -DZLIB_CONST

LOCAL_C_INCLUDES := \
    src/zlib  \
    src/e2fsdroid/ext2fs \
    src/sparse/sparse \
    src/sparse/android-base

LOCAL_SRC_FILES += \
src/sparse/android-base/mapped_file.cpp \
src/sparse/android-base/stringprintf.cpp \
src/sparse/append2simg.cpp \
src/sparse/asprintf.c \
src/sparse/backed_block.cpp \
src/sparse/img2simg.cpp \
src/sparse/output_file.cpp \
src/sparse/simg2img.cpp \
src/sparse/sparse.cpp \
src/sparse/sparse_crc32.cpp \
src/sparse/sparse_err.cpp \
src/sparse/sparse_fuzzer.cpp \
src/sparse/sparse_read.cpp \

include $(BUILD_STATIC_LIBRARY)

##############################################################################
# libpng
##############################################################################
include $(CLEAR_VARS)

LOCAL_MODULE := libpng

LOCAL_CFLAGS := \
    -DHAVE_HIDDEN \
    -DZLIB_CONST
    
LOCAL_C_INCLUDES := \
         src/libpng \
            
LOCAL_SRC_FILES += \
src/libpng/png.c \
src/libpng/pngerror.c \
src/libpng/pngget.c \
src/libpng/pngmem.c \
src/libpng/pngpread.c \
src/libpng/pngread.c \
src/libpng/pngrio.c \
src/libpng/pngrtran.c \
src/libpng/pngrutil.c \
src/libpng/pngset.c \
src/libpng/pngtrans.c \
src/libpng/pngwio.c \
src/libpng/pngwrite.c \
src/libpng/pngwtran.c \
src/libpng/pngwutil.c \

include $(BUILD_STATIC_LIBRARY)

##############################################################################
# minizip
##############################################################################
include $(CLEAR_VARS)

LOCAL_MODULE := libminizip

LOCAL_CFLAGS := \
    -DHAVE_HIDDEN \
    -DZLIB_CONST

LOCAL_C_INCLUDES := \
        src/minizip \

LOCAL_SRC_FILES += \
src/minizip/ioapi.c \
src/minizip/miniunz.c \
src/minizip/minizip.c \
src/minizip/mztools.c \
src/minizip/unzip.c \
src/minizip/zip.c \

include $(BUILD_STATIC_LIBRARY)

#####################################################################################
#utils
#####################################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := bin_utils

LOCAL_C_INCLUDES := \
	includes \
	src/zlib \
    src/e2fsdroid/ext2fs

LOCAL_CXXFLAGS := -fexceptions -std=c++2a -pipe -O2 -s

LOCAL_LDFLAGS := -fPIE -static -ldl

LOCAL_SRC_FILES := $(wildcard src/*.cpp) \
					$(wildcard src/*.cxx)

LOCAL_STATIC_LIBRARIES := \
z \
sparse \
libpng \
minizip

include $(BUILD_EXECUTABLE)

