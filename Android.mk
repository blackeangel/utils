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
    src/ext2

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
    src/ext2 \
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

#####################################################################################
# libbz2
#####################################################################################
include $(CLEAR_VARS)
LOCAL_MODULE        := libbz2
LOCAL_CFLAGS        := -O2
LOCAL_C_INCLUDES    := src/bzip2
LOCAL_SRC_FILES     := \
src/bzip2/blocksort.c \
src/bzip2/bzlib.c \
src/bzip2/compress.c \
src/bzip2/crctable.c \
src/bzip2/decompress.c \
src/bzip2/huffman.c \
src/bzip2/randtable.c

include $(BUILD_STATIC_LIBRARY)

#####################################################################################
# liblz4
#####################################################################################
include $(CLEAR_VARS)
LOCAL_MODULE        := liblz4
LOCAL_CFLAGS        := -O2
LOCAL_C_INCLUDES    := src/lz4
LOCAL_SRC_FILES     := \
src/lz4/lz4.c \
src/lz4/lz4frame.c \
src/lz4/lz4hc.c \
src/lz4/xxhash.c

include $(BUILD_STATIC_LIBRARY)

#####################################################################################
# liblzo2
#####################################################################################
include $(CLEAR_VARS)
LOCAL_MODULE        := liblzo2
LOCAL_CFLAGS        := -O2 -DHAVE_CONFIG_H
LOCAL_C_INCLUDES    := src/lzo
LOCAL_SRC_FILES     := \
src/lzo/lzo1.c \
src/lzo/lzo1_99.c \
src/lzo/lzo1a.c \
src/lzo/lzo1a_99.c \
src/lzo/lzo1b_1.c \
src/lzo/lzo1b_2.c \
src/lzo/lzo1b_3.c \
src/lzo/lzo1b_4.c \
src/lzo/lzo1b_5.c \
src/lzo/lzo1b_6.c \
src/lzo/lzo1b_7.c \
src/lzo/lzo1b_8.c \
src/lzo/lzo1b_9.c \
src/lzo/lzo1b_99.c \
src/lzo/lzo1b_9x.c \
src/lzo/lzo1b_cc.c \
src/lzo/lzo1b_d1.c \
src/lzo/lzo1b_d2.c \
src/lzo/lzo1b_rr.c \
src/lzo/lzo1b_xx.c \
src/lzo/lzo1c_1.c \
src/lzo/lzo1c_2.c \
src/lzo/lzo1c_3.c \
src/lzo/lzo1c_4.c \
src/lzo/lzo1c_5.c \
src/lzo/lzo1c_6.c \
src/lzo/lzo1c_7.c \
src/lzo/lzo1c_8.c \
src/lzo/lzo1c_9.c \
src/lzo/lzo1c_99.c \
src/lzo/lzo1c_9x.c \
src/lzo/lzo1c_cc.c \
src/lzo/lzo1c_d1.c \
src/lzo/lzo1c_d2.c \
src/lzo/lzo1c_rr.c \
src/lzo/lzo1c_xx.c \
src/lzo/lzo1f_1.c \
src/lzo/lzo1f_9x.c \
src/lzo/lzo1f_d1.c \
src/lzo/lzo1f_d2.c \
src/lzo/lzo1x_1.c \
src/lzo/lzo1x_1k.c \
src/lzo/lzo1x_1l.c \
src/lzo/lzo1x_1o.c \
src/lzo/lzo1x_9x.c \
src/lzo/lzo1x_d1.c \
src/lzo/lzo1x_d2.c \
src/lzo/lzo1x_d3.c \
src/lzo/lzo1x_o.c \
src/lzo/lzo1y_1.c \
src/lzo/lzo1y_9x.c \
src/lzo/lzo1y_d1.c \
src/lzo/lzo1y_d2.c \
src/lzo/lzo1y_d3.c \
src/lzo/lzo1y_o.c \
src/lzo/lzo1z_9x.c \
src/lzo/lzo1z_d1.c \
src/lzo/lzo1z_d2.c \
src/lzo/lzo1z_d3.c \
src/lzo/lzo2a_9x.c \
src/lzo/lzo2a_d1.c \
src/lzo/lzo2a_d2.c \
src/lzo/lzo_crc.c \
src/lzo/lzo_init.c \
src/lzo/lzo_ptr.c \
src/lzo/lzo_str.c \
src/lzo/lzo_util.c

include $(BUILD_STATIC_LIBRARY)

#####################################################################################
# libzstd
#####################################################################################
include $(CLEAR_VARS)
LOCAL_MODULE        := libzstd
LOCAL_CFLAGS        := -O2 -DZSTD_MULTITHREAD=0
LOCAL_C_INCLUDES    := src/zstd
LOCAL_SRC_FILES     := \
src/zstd/debug.c \
src/zstd/entropy_common.c \
src/zstd/error_private.c \
src/zstd/fse_compress.c \
src/zstd/fse_decompress.c \
src/zstd/hist.c \
src/zstd/huf_compress.c \
src/zstd/huf_decompress.c \
src/zstd/pool.c \
src/zstd/threading.c \
src/zstd/xxhash.c \
src/zstd/zstd_common.c \
src/zstd/zstd_compress.c \
src/zstd/zstd_compress_literals.c \
src/zstd/zstd_compress_sequences.c \
src/zstd/zstd_compress_superblock.c \
src/zstd/zstd_ddict.c \
src/zstd/zstd_decompress.c \
src/zstd/zstd_decompress_block.c \
src/zstd/zstd_double_fast.c \
src/zstd/zstd_fast.c \
src/zstd/zstd_lazy.c \
src/zstd/zstd_ldm.c \
src/zstd/zstd_opt.c \
src/zstd/zstd_preSplit.c \
src/zstd/zstdmt_compress.c

include $(BUILD_STATIC_LIBRARY)

#####################################################################################
# libzip  (deflate-only, via zlib)
#####################################################################################
include $(CLEAR_VARS)
LOCAL_MODULE        := libzip
LOCAL_CFLAGS        := -O2 -DHAVE_CONFIG_H -D_FILE_OFFSET_BITS=64
LOCAL_C_INCLUDES    := \
	src/libzip \
	src/zlib
LOCAL_SRC_FILES     := \
src/libzip/zip_add.c \
src/libzip/zip_add_dir.c \
src/libzip/zip_add_entry.c \
src/libzip/zip_algorithm_deflate.c \
src/libzip/zip_buffer.c \
src/libzip/zip_close.c \
src/libzip/zip_delete.c \
src/libzip/zip_dir_add.c \
src/libzip/zip_dirent.c \
src/libzip/zip_discard.c \
src/libzip/zip_entry.c \
src/libzip/zip_err_str.c \
src/libzip/zip_error.c \
src/libzip/zip_error_clear.c \
src/libzip/zip_error_get.c \
src/libzip/zip_error_get_sys_type.c \
src/libzip/zip_error_strerror.c \
src/libzip/zip_error_to_str.c \
src/libzip/zip_extra_field.c \
src/libzip/zip_extra_field_api.c \
src/libzip/zip_fclose.c \
src/libzip/zip_fdopen.c \
src/libzip/zip_file_add.c \
src/libzip/zip_file_error_clear.c \
src/libzip/zip_file_error_get.c \
src/libzip/zip_file_get_comment.c \
src/libzip/zip_file_get_external_attributes.c \
src/libzip/zip_file_get_offset.c \
src/libzip/zip_file_rename.c \
src/libzip/zip_file_replace.c \
src/libzip/zip_file_set_comment.c \
src/libzip/zip_file_set_encryption.c \
src/libzip/zip_file_set_external_attributes.c \
src/libzip/zip_file_set_mtime.c \
src/libzip/zip_file_strerror.c \
src/libzip/zip_fopen.c \
src/libzip/zip_fopen_encrypted.c \
src/libzip/zip_fopen_index.c \
src/libzip/zip_fopen_index_encrypted.c \
src/libzip/zip_fread.c \
src/libzip/zip_fseek.c \
src/libzip/zip_ftell.c \
src/libzip/zip_get_archive_comment.c \
src/libzip/zip_get_archive_flag.c \
src/libzip/zip_get_file_comment.c \
src/libzip/zip_get_name.c \
src/libzip/zip_get_num_entries.c \
src/libzip/zip_get_num_files.c \
src/libzip/zip_hash.c \
src/libzip/zip_io_util.c \
src/libzip/zip_libzip_version.c \
src/libzip/zip_memdup.c \
src/libzip/zip_name_locate.c \
src/libzip/zip_new.c \
src/libzip/zip_open.c \
src/libzip/zip_progress.c \
src/libzip/zip_random_unix.c \
src/libzip/zip_realloc.c \
src/libzip/zip_rename.c \
src/libzip/zip_replace.c \
src/libzip/zip_set_archive_comment.c \
src/libzip/zip_set_archive_flag.c \
src/libzip/zip_set_default_password.c \
src/libzip/zip_set_file_comment.c \
src/libzip/zip_set_file_compression.c \
src/libzip/zip_set_name.c \
src/libzip/zip_source_accept_empty.c \
src/libzip/zip_source_begin_write.c \
src/libzip/zip_source_begin_write_cloning.c \
src/libzip/zip_source_buffer.c \
src/libzip/zip_source_call.c \
src/libzip/zip_source_close.c \
src/libzip/zip_source_commit_write.c \
src/libzip/zip_source_compress.c \
src/libzip/zip_source_crc.c \
src/libzip/zip_source_error.c \
src/libzip/zip_source_file_common.c \
src/libzip/zip_source_file_stdio.c \
src/libzip/zip_source_file_stdio_named.c \
src/libzip/zip_source_free.c \
src/libzip/zip_source_function.c \
src/libzip/zip_source_get_dostime.c \
src/libzip/zip_source_get_file_attributes.c \
src/libzip/zip_source_is_deleted.c \
src/libzip/zip_source_layered.c \
src/libzip/zip_source_open.c \
src/libzip/zip_source_pass_to_lower_layer.c \
src/libzip/zip_source_read.c \
src/libzip/zip_source_remove.c \
src/libzip/zip_source_rollback_write.c \
src/libzip/zip_source_seek.c \
src/libzip/zip_source_seek_write.c \
src/libzip/zip_source_stat.c \
src/libzip/zip_source_supports.c \
src/libzip/zip_source_tell.c \
src/libzip/zip_source_tell_write.c \
src/libzip/zip_source_window.c \
src/libzip/zip_source_write.c \
src/libzip/zip_source_zip.c \
src/libzip/zip_source_zip_new.c \
src/libzip/zip_stat.c \
src/libzip/zip_stat_index.c \
src/libzip/zip_stat_init.c \
src/libzip/zip_strerror.c \
src/libzip/zip_string.c \
src/libzip/zip_unchange.c \
src/libzip/zip_unchange_all.c \
src/libzip/zip_unchange_archive.c \
src/libzip/zip_unchange_data.c \
src/libzip/zip_utf-8.c

include $(BUILD_STATIC_LIBRARY)
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
# libbrotli (vendored Google Brotli, encoder + decoder)
include $(CLEAR_VARS)
LOCAL_MODULE := libbrotli
LOCAL_C_INCLUDES := src/brotli/include
LOCAL_CFLAGS := -O2
LOCAL_SRC_FILES := \
$(wildcard src/brotli/common/*.c) \
$(wildcard src/brotli/enc/*.c) \
$(wildcard src/brotli/dec/*.c)
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
	src/bzip2 \
	src/lz4 \
	src/lzo \
	src/zstd \
	src/libzip \
	src/brotli/include \
	src/ext2

LOCAL_CXXFLAGS := -fexceptions -std=c++2a -pipe -O2 -s -DLZMA_API_STATIC

LOCAL_LDFLAGS := -fPIE -static -ldl

LOCAL_SRC_FILES := $(wildcard src/*.cpp) \
					$(wildcard src/*.cxx) \
					$(wildcard src/md1img/*.cpp)

LOCAL_STATIC_LIBRARIES := \
z \
sparse \
libpng \
liblzma \
libbrotli \
libbz2 \
liblz4 \
liblzo2 \
libzstd \
libzip

include $(BUILD_EXECUTABLE)

