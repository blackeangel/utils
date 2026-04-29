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
    src/zlib/gzread.c \
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
src/sparse/simg2simg.cpp \
src/sparse/sparse.cpp \
src/sparse/sparse_crc32.cpp \
src/sparse/sparse_err.cpp \
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
# liblzma (vendored XZ Utils, threads disabled)
include $(CLEAR_VARS)
LOCAL_MODULE := liblzma
LOCAL_C_INCLUDES := \
	src/liblzma \
	src/liblzma/api \
	src/liblzma/common \
	src/liblzma/check \
	src/liblzma/lzma \
	src/liblzma/lz \
	src/liblzma/rangecoder \
	src/liblzma/delta \
	src/liblzma/simple 

LOCAL_CFLAGS := -DHAVE_CONFIG_H -DLZMA_API_STATIC

LOCAL_SRC_FILES := \
src/liblzma/check/check.c \
src/liblzma/check/crc32_fast.c \
src/liblzma/check/crc32_small.c \
src/liblzma/check/crc64_fast.c \
src/liblzma/check/crc64_small.c \
src/liblzma/check/sha256.c \
src/liblzma/common/alone_decoder.c \
src/liblzma/common/alone_encoder.c \
src/liblzma/common/auto_decoder.c \
src/liblzma/common/block_buffer_decoder.c \
src/liblzma/common/block_buffer_encoder.c \
src/liblzma/common/block_decoder.c \
src/liblzma/common/block_encoder.c \
src/liblzma/common/block_header_decoder.c \
src/liblzma/common/block_header_encoder.c \
src/liblzma/common/block_util.c \
src/liblzma/common/common.c \
src/liblzma/common/easy_buffer_encoder.c \
src/liblzma/common/easy_decoder_memusage.c \
src/liblzma/common/easy_encoder.c \
src/liblzma/common/easy_encoder_memusage.c \
src/liblzma/common/easy_preset.c \
src/liblzma/common/file_info.c \
src/liblzma/common/filter_buffer_decoder.c \
src/liblzma/common/filter_buffer_encoder.c \
src/liblzma/common/filter_common.c \
src/liblzma/common/filter_decoder.c \
src/liblzma/common/filter_encoder.c \
src/liblzma/common/filter_flags_decoder.c \
src/liblzma/common/filter_flags_encoder.c \
src/liblzma/common/hardware_physmem.c \
src/liblzma/common/index.c \
src/liblzma/common/index_decoder.c \
src/liblzma/common/index_encoder.c \
src/liblzma/common/index_hash.c \
src/liblzma/common/lzip_decoder.c \
src/liblzma/common/microlzma_decoder.c \
src/liblzma/common/microlzma_encoder.c \
src/liblzma/common/stream_buffer_decoder.c \
src/liblzma/common/stream_buffer_encoder.c \
src/liblzma/common/stream_decoder.c \
src/liblzma/common/stream_encoder.c \
src/liblzma/common/stream_flags_common.c \
src/liblzma/common/stream_flags_decoder.c \
src/liblzma/common/stream_flags_encoder.c \
src/liblzma/common/string_conversion.c \
src/liblzma/common/vli_decoder.c \
src/liblzma/common/vli_encoder.c \
src/liblzma/common/vli_size.c \
src/liblzma/delta/delta_common.c \
src/liblzma/delta/delta_decoder.c \
src/liblzma/delta/delta_encoder.c \
src/liblzma/lz/lz_decoder.c \
src/liblzma/lz/lz_encoder.c \
src/liblzma/lz/lz_encoder_mf.c \
src/liblzma/lzma/fastpos_table.c \
src/liblzma/lzma/lzma2_decoder.c \
src/liblzma/lzma/lzma2_encoder.c \
src/liblzma/lzma/lzma_decoder.c \
src/liblzma/lzma/lzma_encoder.c \
src/liblzma/lzma/lzma_encoder_optimum_fast.c \
src/liblzma/lzma/lzma_encoder_optimum_normal.c \
src/liblzma/lzma/lzma_encoder_presets.c \
src/liblzma/rangecoder/price_table.c \
src/liblzma/rangecoder/price_tablegen.c \
src/liblzma/simple/arm.c \
src/liblzma/simple/arm64.c \
src/liblzma/simple/armthumb.c \
src/liblzma/simple/ia64.c \
src/liblzma/simple/powerpc.c \
src/liblzma/simple/riscv.c \
src/liblzma/simple/simple_coder.c \
src/liblzma/simple/simple_decoder.c \
src/liblzma/simple/simple_encoder.c \
src/liblzma/simple/sparc.c \
src/liblzma/simple/x86.c \
src/liblzma/tuklib_cpucores.c \
src/liblzma/tuklib_physmem.c 

include $(BUILD_STATIC_LIBRARY)

#####################################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := bin_utils

LOCAL_C_INCLUDES := \
	includes \
	src/zlib \
	src/liblzma \
	src/liblzma/api \
	src/md1img \
    src/e2fsdroid/ext2fs

LOCAL_CXXFLAGS := -fexceptions -std=c++2a -pipe -O2 -s -DLZMA_API_STATIC

LOCAL_LDFLAGS := -fPIE -static -ldl

LOCAL_SRC_FILES := $(wildcard src/*.cpp) \
					$(wildcard src/*.cxx) \
					$(wildcard src/md1img/*.cpp)

LOCAL_STATIC_LIBRARIES := \
z \
sparse \
libpng \
minizip \
liblzma

include $(BUILD_EXECUTABLE)

