cmake_minimum_required(VERSION 3.20)
project(utils)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
set(BUILD_SHARED_LIBS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ── Compiler flags ─────────────────────────────────────────────────────────────
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE   "/O2 /DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-pipe -O2")
    set(CMAKE_C_FLAGS_RELEASE   "-pipe -O2")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s")
endif()

# ── Output paths ───────────────────────────────────────────────────────────────
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)
foreach(cfg Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${cfg} CFG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG} ${PROJECT_SOURCE_DIR}/out)
endforeach()

# ── Static linking ─────────────────────────────────────────────────────────────
if(CMAKE_CROSSCOMPILING AND CMAKE_ANDROID_NDK)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(CMAKE_EXE_LINKER_FLAGS "-static")
elseif(ANDROID)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")
elseif(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s")
else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(CMAKE_EXE_LINKER_FLAGS "-static")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s")
endif()

include(GNUInstallDirs)
# ── Threads ────────────────────────────────────────────────────────────────────
if(ANDROID AND NOT CMAKE_CROSSCOMPILING)
    set(CMAKE_THREAD_LIBS_INIT     "" CACHE STRING "" FORCE)
    set(CMAKE_HAVE_THREADS_LIBRARY 1)
    set(Threads_FOUND              TRUE CACHE BOOL "" FORCE)
    if(NOT TARGET Threads::Threads)
        add_library(Threads::Threads INTERFACE IMPORTED GLOBAL)
    endif()
else()
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

# ── Global include dirs ────────────────────────────────────────────────────────
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/zlib
    ${PROJECT_SOURCE_DIR}/src/sparse
    ${PROJECT_SOURCE_DIR}/src/sparse/android-base
    ${PROJECT_SOURCE_DIR}/src/libpng
    ${PROJECT_SOURCE_DIR}/src/libarchive
    ${PROJECT_SOURCE_DIR}/src/libzip
    ${PROJECT_SOURCE_DIR}/src/md1img
    ${PROJECT_SOURCE_DIR}/src/bzip2
    ${PROJECT_SOURCE_DIR}/src/lz4
    ${PROJECT_SOURCE_DIR}/src/lzo
    ${PROJECT_SOURCE_DIR}/src/zstd
)

###############################################################################
# ── 1. zlib ────────────────────────────────────────────────────────────────────
###############################################################################
set(ZLIB_SRCS
    src/zlib/adler32.c   src/zlib/compress.c  src/zlib/crc32.c
    src/zlib/deflate.c   src/zlib/gzclose.c   src/zlib/gzlib.c
    src/zlib/gzread.c    src/zlib/gzwrite.c   src/zlib/infback.c
    src/zlib/inffast.c   src/zlib/inflate.c   src/zlib/inftrees.c
    src/zlib/trees.c     src/zlib/uncompr.c   src/zlib/zutil.c
)
add_library(z STATIC ${ZLIB_SRCS})
target_include_directories(z PUBLIC ${PROJECT_SOURCE_DIR}/src/zlib)

###############################################################################
# ── 2. libpng ──────────────────────────────────────────────────────────────────
###############################################################################
file(GLOB LIBPNG_SRCS src/libpng/*.c)
add_library(png STATIC ${LIBPNG_SRCS})
target_include_directories(png PUBLIC ${PROJECT_SOURCE_DIR}/src/libpng)
target_link_libraries(png PUBLIC z)

###############################################################################
# ── 3. liblzma (vendored XZ Utils, threads disabled) ─────────────────────────
###############################################################################
file(GLOB_RECURSE LIBLZMA_SRCS src/liblzma/**/*.c src/liblzma/*.c)
list(FILTER LIBLZMA_SRCS EXCLUDE REGEX
    "(tablegen|stream_encoder_mt|stream_decoder_mt|outqueue|hardware_cputhreads|crc32_small|crc64_small)\\.c$")
add_library(liblzma STATIC ${LIBLZMA_SRCS})
target_compile_definitions(liblzma PUBLIC HAVE_CONFIG_H LZMA_API_STATIC)
target_include_directories(liblzma PUBLIC
    ${PROJECT_SOURCE_DIR}/src/liblzma
    ${PROJECT_SOURCE_DIR}/src/liblzma/api
    ${PROJECT_SOURCE_DIR}/src/liblzma/common
    ${PROJECT_SOURCE_DIR}/src/liblzma/check
    ${PROJECT_SOURCE_DIR}/src/liblzma/lzma
    ${PROJECT_SOURCE_DIR}/src/liblzma/lz
    ${PROJECT_SOURCE_DIR}/src/liblzma/rangecoder
    ${PROJECT_SOURCE_DIR}/src/liblzma/delta
    ${PROJECT_SOURCE_DIR}/src/liblzma/simple
)
set(LZMA_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/liblzma/api)

###############################################################################
# ── 4. brotli (encoder + decoder, vendored) ───────────────────────────────────
###############################################################################
file(GLOB_RECURSE BROTLI_SRCS
    src/brotli/common/*.c src/brotli/enc/*.c src/brotli/dec/*.c)
add_library(brotli STATIC ${BROTLI_SRCS})
target_include_directories(brotli PUBLIC ${PROJECT_SOURCE_DIR}/src/brotli/include)
set(BROTLI_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/brotli/include)

###############################################################################
# ── 5. bzip2 ──────────────────────────────────────────────────────────────────
###############################################################################
file(GLOB BZIP2_SRCS src/bzip2/*.c)
add_library(bz2 STATIC ${BZIP2_SRCS})
target_include_directories(bz2 PUBLIC ${PROJECT_SOURCE_DIR}/src/bzip2)

###############################################################################
# ── 6. lz4 ────────────────────────────────────────────────────────────────────
###############################################################################
set(LZ4_SRCS src/lz4/lz4.c src/lz4/lz4hc.c src/lz4/lz4frame.c src/lz4/xxhash.c)
add_library(lz4 STATIC ${LZ4_SRCS})
target_include_directories(lz4 PUBLIC ${PROJECT_SOURCE_DIR}/src/lz4)

###############################################################################
# ── 7. lzo ────────────────────────────────────────────────────────────────────
###############################################################################
file(GLOB LZO_SRCS src/lzo/lzo*.c)
add_library(lzo2 STATIC ${LZO_SRCS})
target_compile_definitions(lzo2 PRIVATE HAVE_CONFIG_H)
target_include_directories(lzo2 PUBLIC ${PROJECT_SOURCE_DIR}/src/lzo)

###############################################################################
# ── 8. zstd ───────────────────────────────────────────────────────────────────
###############################################################################
file(GLOB ZSTD_SRCS src/zstd/*.c)
list(FILTER ZSTD_SRCS EXCLUDE REGEX "(zstd_v0[0-9]|legacy|fuzz)\\.c$")
add_library(zstd STATIC ${ZSTD_SRCS})
target_compile_definitions(zstd PRIVATE ZSTD_MULTITHREAD=0)
target_include_directories(zstd PUBLIC ${PROJECT_SOURCE_DIR}/src/zstd)

###############################################################################
# ── 9. libzip (zip only, zlib backend) ────────────────────────────────────────
###############################################################################
file(GLOB LIBZIP_SRCS src/libzip/*.c)
# Exclude backends we don't have (bzip2/lzma/zstd/lz4 algorithm files)
# and AES/crypto modules
list(FILTER LIBZIP_SRCS EXCLUDE REGEX
    "(algorithm_bzip|algorithm_xz|algorithm_zstd|algorithm_lz4|pkware|winzip_aes|crypto)\\.c$")
add_library(libzip STATIC ${LIBZIP_SRCS})
target_compile_definitions(libzip PRIVATE HAVE_CONFIG_H)
target_include_directories(libzip PUBLIC
    ${PROJECT_SOURCE_DIR}/src/libzip
    ${PROJECT_SOURCE_DIR}/src/zlib
)
target_link_libraries(libzip PUBLIC z)

###############################################################################
# ── 10. libarchive (tar / cpio / raw + format detection) ─────────────────────
###############################################################################
file(GLOB LIBARCHIVE_SRCS src/libarchive/*.c)
# Exclude platform-specific disk-traversal for wrong OS
if(NOT WIN32)
    list(FILTER LIBARCHIVE_SRCS EXCLUDE REGEX
        "(archive_read_disk_windows|archive_write_disk_windows|filter_fork_windows)\\.c$")
else()
    list(FILTER LIBARCHIVE_SRCS EXCLUDE REGEX
        "(archive_read_disk_posix|archive_write_disk_posix|filter_fork_posix)\\.c$")
endif()
# Exclude external-program filters (lzop, lrzip)
list(FILTER LIBARCHIVE_SRCS EXCLUDE REGEX "(filter_lzop|filter_lrzip)\\.c$")

add_library(archive STATIC ${LIBARCHIVE_SRCS})
target_compile_definitions(archive PUBLIC HAVE_CONFIG_H LZMA_API_STATIC)
target_compile_options(archive PRIVATE
    -include ${PROJECT_SOURCE_DIR}/src/libarchive/config.h)
target_include_directories(archive PUBLIC
    ${PROJECT_SOURCE_DIR}/src/libarchive
    ${PROJECT_SOURCE_DIR}/src/liblzma/api
    ${PROJECT_SOURCE_DIR}/src/zlib
)
target_link_libraries(archive PUBLIC liblzma z)

###############################################################################
# ── 12. utils executable ──────────────────────────────────────────────────────
###############################################################################
file(GLOB SRCS
    "${PROJECT_SOURCE_DIR}/include/*"
    "${PROJECT_SOURCE_DIR}/src/sparse/android-base/*"
    "${PROJECT_SOURCE_DIR}/src/sparse/*"
    "${PROJECT_SOURCE_DIR}/src/md1img/*"
    "${PROJECT_SOURCE_DIR}/src/tar_repacker/*"
    "${PROJECT_SOURCE_DIR}/src/*"
)
# Exclude vendored library dirs (built as separate targets above)
list(FILTER SRCS EXCLUDE REGEX "src/minizip/")
# Exclude standalone fuzz tools
list(FILTER SRCS EXCLUDE REGEX "sparse_fuzzer\\.cpp$")
# asprintf is in POSIX/bionic — only needed on Windows
if(NOT WIN32)
    list(FILTER SRCS EXCLUDE REGEX "asprintf\\.c$")
endif()

add_executable(utils ${SRCS})

target_include_directories(utils PRIVATE
    ${LZMA_INCLUDE_DIR}
    ${BROTLI_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/src/libarchive
    ${PROJECT_SOURCE_DIR}/src/libzip
    ${PROJECT_SOURCE_DIR}/src/bzip2
    ${PROJECT_SOURCE_DIR}/src/lz4
    ${PROJECT_SOURCE_DIR}/src/lzo
    ${PROJECT_SOURCE_DIR}/src/zstd
    ${PROJECT_SOURCE_DIR}/src/tar_repacker
)

target_link_libraries(utils
    png z liblzma brotli
    bz2 lz4 lzo2 zstd
    libzip archive
    Threads::Threads
)

# ── Strip binary in Release ────────────────────────────────────────────────────
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    if(NOT MSVC AND CMAKE_STRIP)
        add_custom_command(TARGET utils POST_BUILD
            COMMAND ${CMAKE_STRIP} --strip-all $<TARGET_FILE:utils>
            COMMENT "Stripping utils"
            VERBATIM)
    endif()
endif()
