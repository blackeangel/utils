cmake_minimum_required(VERSION 3.20)
project(utils)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
set(BUILD_SHARED_LIBS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE   "/O2 /DNDEBUG")
else()
    # -s strips debug symbols at link time (linker flag, not compiler flag)
    # Pass it only to the linker to avoid clang -Wunused-command-line-argument
    set(CMAKE_CXX_FLAGS_RELEASE "-pipe -O2")
    set(CMAKE_C_FLAGS_RELEASE   "-pipe -O2")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s")
endif()
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/out)
# On multi-config generators (MSVC, Xcode) the binary lands in out/<Config>/
# Force single-level output so CI can find it at out/utils(.exe) reliably
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)
foreach(cfg Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${cfg} CFG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG} ${PROJECT_SOURCE_DIR}/out)
endforeach()

# ── Линковка ──────────────────────────────────────────────────────────────────
# NDK кросс-компиляция → полный -static (STL через ANDROID_STL=c++_static)
# Нативно в Termux        → только -static-libstdc++ (bionic libc.so без .a)
# Linux / macOS / Windows → полный -static
if(CMAKE_CROSSCOMPILING AND CMAKE_ANDROID_NDK)
    # NDK cross-compile: full static
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(CMAKE_EXE_LINKER_FLAGS "-static")
elseif(ANDROID)
    # Termux native: bionic has no static libc.so
    set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")
elseif(MSVC)
    # MSVC: static runtime via /MT
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(WIN32)
    # MinGW: static libgcc + libstdc++
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
else()
    # Linux native: full static
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

include(GNUInstallDirs)
include(FetchContent)

# ── Threads ───────────────────────────────────────────────────────────────────
# Termux нативно: pthread встроен в bionic — удовлетворяем CMake вручную.
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

# ── XZ / liblzma ──────────────────────────────────────────────────────────────
# Android cross-compile: FetchContent fails (can't run host binaries during configure).
# Use the vendored copy in src/liblzma/ instead.
# Linux / Windows native: FetchContent fetches and builds XZ Utils automatically.
if(CMAKE_CROSSCOMPILING OR ANDROID)
    # ── Vendored liblzma (src/liblzma/) ───────────────────────────────────────
    file(GLOB_RECURSE VENDORED_LZMA_SRCS
        "${PROJECT_SOURCE_DIR}/src/liblzma/*.c"
    )
    # Exclude tablegen helpers and MT-only files (not needed, may not compile)
    list(FILTER VENDORED_LZMA_SRCS EXCLUDE REGEX
        "(tablegen|stream_encoder_mt|stream_decoder_mt|outqueue|hardware_cputhreads|tuklib_cpucores|crc32_small|crc64_small)\.c$")

    add_library(liblzma STATIC ${VENDORED_LZMA_SRCS})
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
else()
    # ── FetchContent (native builds) ───────────────────────────────────────────
    FetchContent_Declare(
        xz
        GIT_REPOSITORY https://github.com/tukaani-project/xz.git
        GIT_TAG        v5.6.3
        GIT_SHALLOW    TRUE
    )
    set(XZ_BUILD_ENCODERS    ON  CACHE BOOL "" FORCE)
    set(XZ_BUILD_DECODERS    ON  CACHE BOOL "" FORCE)
    set(XZ_ENABLE_THREADS    OFF CACHE BOOL "" FORCE)
    set(XZ_TOOL_XZ           OFF CACHE BOOL "" FORCE)
    set(XZ_TOOL_XZDEC        OFF CACHE BOOL "" FORCE)
    set(XZ_TOOL_LZMADEC      OFF CACHE BOOL "" FORCE)
    set(XZ_TOOL_LZMAINFO     OFF CACHE BOOL "" FORCE)
    set(XZ_TOOL_XZ_GREP      OFF CACHE BOOL "" FORCE)
    set(XZ_TOOL_XZ_LESS      OFF CACHE BOOL "" FORCE)
    set(XZ_TOOL_XZ_DIFF      OFF CACHE BOOL "" FORCE)
    set(CREATE_XZ_SYMLINKS   OFF CACHE BOOL "" FORCE)
    set(CREATE_LZMA_SYMLINKS OFF CACHE BOOL "" FORCE)
    set(XZ_ENABLE_DOXYGEN    OFF CACHE BOOL "" FORCE)
    set(XZ_ENABLE_SANDBOX    OFF CACHE BOOL "" FORCE)
    set(ENABLE_NLS           OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(xz)
    set(LZMA_INCLUDE_DIR ${xz_SOURCE_DIR}/src/liblzma/api)
endif()

# ── include_directories ───────────────────────────────────────────────────────
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/zlib
    ${PROJECT_SOURCE_DIR}/src/sparse
    ${PROJECT_SOURCE_DIR}/src/sparse/android-base
    ${PROJECT_SOURCE_DIR}/src/libpng
    ${PROJECT_SOURCE_DIR}/src/minizip
    ${PROJECT_SOURCE_DIR}/src/md1img
)

# ── zlib (вендорная копия) ────────────────────────────────────────────────────
set(ZLIB_SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zlib/adler32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zlib/compress.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zlib/crc32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zlib/deflate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zlib/gzclose.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zlib/gzlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zlib/gzread.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zlib/gzwrite.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zlib/infback.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zlib/inffast.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zlib/inflate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zlib/inftrees.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zlib/trees.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zlib/uncompr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zlib/zutil.c
)
add_library(z STATIC ${ZLIB_SRC_FILES})

# ── libpng ────────────────────────────────────────────────────────────────────
set(LIBPNG_SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libpng/png.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libpng/pngerror.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libpng/pngget.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libpng/pngmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libpng/pngpread.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libpng/pngread.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libpng/pngrio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libpng/pngrtran.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libpng/pngrutil.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libpng/pngset.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libpng/pngtrans.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libpng/pngwio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libpng/pngwrite.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libpng/pngwtran.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libpng/pngwutil.c
)
add_library(png STATIC ${LIBPNG_SRC_FILES})

# ── minizip ───────────────────────────────────────────────────────────────────
set(LIBMINIZIP_SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/minizip/ioapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/minizip/miniunz.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/minizip/minizip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/minizip/mztools.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/minizip/unzip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/minizip/zip.c
)
if(WIN32)
    list(APPEND LIBMINIZIP_SRC_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/minizip/iowin32.c)
endif()
add_library(minizip STATIC ${LIBMINIZIP_SRC_FILES})
target_link_libraries(png z minizip)

# ── utils executable ──────────────────────────────────────────────────────────
file(GLOB SRCS
    "${PROJECT_SOURCE_DIR}/include/*"
    "${PROJECT_SOURCE_DIR}/src/sparse/android-base/*"
    "${PROJECT_SOURCE_DIR}/src/sparse/*"
    "${PROJECT_SOURCE_DIR}/src/md1img/*"
    "${PROJECT_SOURCE_DIR}/src/*"
)
# sparse_fuzzer is a standalone tool - not part of utils
list(FILTER SRCS EXCLUDE REGEX "sparse_fuzzer\\.cpp$")

add_executable(utils ${SRCS})

target_include_directories(utils PRIVATE
    ${LZMA_INCLUDE_DIR}
)

target_link_libraries(utils
    png
    z
    liblzma
    Threads::Threads
)
