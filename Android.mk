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
src/liblzma/check/crc64_fast.c \
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
src/brotli/common/constants.c \
src/brotli/common/context.c \
src/brotli/common/dictionary.c \
src/brotli/common/platform.c \
src/brotli/common/shared_dictionary.c \
src/brotli/common/transform.c \
src/brotli/enc/backward_references.c \
src/brotli/enc/backward_references_hq.c \
src/brotli/enc/bit_cost.c \
src/brotli/enc/block_splitter.c \
src/brotli/enc/brotli_bit_stream.c \
src/brotli/enc/cluster.c \
src/brotli/enc/command.c \
src/brotli/enc/compound_dictionary.c \
src/brotli/enc/compress_fragment.c \
src/brotli/enc/compress_fragment_two_pass.c \
src/brotli/enc/dictionary_hash.c \
src/brotli/enc/encode.c \
src/brotli/enc/encoder_dict.c \
src/brotli/enc/entropy_encode.c \
src/brotli/enc/fast_log.c \
src/brotli/enc/histogram.c \
src/brotli/enc/literal_cost.c \
src/brotli/enc/memory.c \
src/brotli/enc/metablock.c \
src/brotli/enc/static_dict.c \
src/brotli/enc/static_dict_lut.c \
src/brotli/enc/static_init.c \
src/brotli/enc/utf8_util.c \
src/brotli/dec/bit_reader.c \
src/brotli/dec/decode.c \
src/brotli/dec/huffman.c \
src/brotli/dec/prefix.c \
src/brotli/dec/state.c \
src/brotli/dec/static_init.c
include $(BUILD_STATIC_LIBRARY)

#####################################################################################

#######################################################
# libarchive (tar/cpio/raw + format detection)
# Used by ndk-build only — cmake builds use FetchContent
#######################################################
include $(CLEAR_VARS)
LOCAL_MODULE        := libarchive
LOCAL_CFLAGS        := -O2 -DHAVE_CONFIG_H -DLZMA_API_STATIC \
                       -include $(LOCAL_PATH)/src/libarchive/config.h
LOCAL_C_INCLUDES    := \
	src/libarchive \
	src/liblzma \
	src/liblzma/api \
	src/zlib
LOCAL_SRC_FILES     := \
src/libarchive/archive_acl.c \
src/libarchive/archive_blake2s_ref.c \
src/libarchive/archive_blake2sp_ref.c \
src/libarchive/archive_check_magic.c \
src/libarchive/archive_cmdline.c \
src/libarchive/archive_cryptor.c \
src/libarchive/archive_digest.c \
src/libarchive/archive_disk_acl_darwin.c \
src/libarchive/archive_disk_acl_freebsd.c \
src/libarchive/archive_disk_acl_linux.c \
src/libarchive/archive_disk_acl_sunos.c \
src/libarchive/archive_entry.c \
src/libarchive/archive_entry_copy_bhfi.c \
src/libarchive/archive_entry_copy_stat.c \
src/libarchive/archive_entry_link_resolver.c \
src/libarchive/archive_entry_sparse.c \
src/libarchive/archive_entry_stat.c \
src/libarchive/archive_entry_strmode.c \
src/libarchive/archive_entry_xattr.c \
src/libarchive/archive_hmac.c \
src/libarchive/archive_match.c \
src/libarchive/archive_options.c \
src/libarchive/archive_pack_dev.c \
src/libarchive/archive_parse_date.c \
src/libarchive/archive_pathmatch.c \
src/libarchive/archive_ppmd7.c \
src/libarchive/archive_ppmd8.c \
src/libarchive/archive_random.c \
src/libarchive/archive_rb.c \
src/libarchive/archive_read.c \
src/libarchive/archive_read_add_passphrase.c \
src/libarchive/archive_read_append_filter.c \
src/libarchive/archive_read_data_into_fd.c \
src/libarchive/archive_read_disk_entry_from_file.c \
src/libarchive/archive_read_disk_posix.c \
src/libarchive/archive_read_disk_set_standard_lookup.c \
src/libarchive/archive_read_extract.c \
src/libarchive/archive_read_extract2.c \
src/libarchive/archive_read_open_fd.c \
src/libarchive/archive_read_open_file.c \
src/libarchive/archive_read_open_filename.c \
src/libarchive/archive_read_open_memory.c \
src/libarchive/archive_read_set_format.c \
src/libarchive/archive_read_set_options.c \
src/libarchive/archive_read_support_filter_all.c \
src/libarchive/archive_read_support_filter_by_code.c \
src/libarchive/archive_read_support_filter_compress.c \
src/libarchive/archive_read_support_filter_gzip.c \
src/libarchive/archive_read_support_filter_none.c \
src/libarchive/archive_read_support_filter_program.c \
src/libarchive/archive_read_support_filter_rpm.c \
src/libarchive/archive_read_support_filter_uu.c \
src/libarchive/archive_read_support_filter_xz.c \
src/libarchive/archive_read_support_format_all.c \
src/libarchive/archive_read_support_format_by_code.c \
src/libarchive/archive_read_support_format_cpio.c \
src/libarchive/archive_read_support_format_empty.c \
src/libarchive/archive_read_support_format_raw.c \
src/libarchive/archive_read_support_format_tar.c \
src/libarchive/archive_read_support_format_zip.c \
src/libarchive/archive_string.c \
src/libarchive/archive_string_sprintf.c \
src/libarchive/archive_time.c \
src/libarchive/archive_util.c \
src/libarchive/archive_version_details.c \
src/libarchive/archive_virtual.c \
src/libarchive/archive_windows.c \
src/libarchive/archive_write.c \
src/libarchive/archive_write_add_filter.c \
src/libarchive/archive_write_add_filter_b64encode.c \
src/libarchive/archive_write_add_filter_by_name.c \
src/libarchive/archive_write_add_filter_compress.c \
src/libarchive/archive_write_add_filter_gzip.c \
src/libarchive/archive_write_add_filter_none.c \
src/libarchive/archive_write_add_filter_program.c \
src/libarchive/archive_write_add_filter_uuencode.c \
src/libarchive/archive_write_add_filter_xz.c \
src/libarchive/archive_write_disk_posix.c \
src/libarchive/archive_write_disk_set_standard_lookup.c \
src/libarchive/archive_write_open_fd.c \
src/libarchive/archive_write_open_file.c \
src/libarchive/archive_write_open_filename.c \
src/libarchive/archive_write_open_memory.c \
src/libarchive/archive_write_set_format.c \
src/libarchive/archive_write_set_format_by_name.c \
src/libarchive/archive_write_set_format_cpio.c \
src/libarchive/archive_write_set_format_cpio_binary.c \
src/libarchive/archive_write_set_format_cpio_newc.c \
src/libarchive/archive_write_set_format_cpio_odc.c \
src/libarchive/archive_write_set_format_filter_by_ext.c \
src/libarchive/archive_write_set_format_gnutar.c \
src/libarchive/archive_write_set_format_pax.c \
src/libarchive/archive_write_set_format_raw.c \
src/libarchive/archive_write_set_format_shar.c \
src/libarchive/archive_write_set_format_ustar.c \
src/libarchive/archive_write_set_format_v7tar.c \
src/libarchive/archive_write_set_format_zip.c \
src/libarchive/archive_write_set_options.c \
src/libarchive/archive_write_set_passphrase.c \
src/libarchive/filter_fork_posix.c \
src/libarchive/xxhash.c

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := bin_utils

LOCAL_C_INCLUDES := \
	includes \
	src/zlib \
	src/liblzma \
	src/liblzma/api \
	src/md1img \
	src/tar_repacker \
	src/libarchive \
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
					$(wildcard src/md1img/*.cpp) \
					$(wildcard src/tar_repacker/*.cpp)

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
libzip \
libarchive

include $(BUILD_EXECUTABLE)

