cmake_minimum_required(VERSION 3.20)
project(utils)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

set(CMAKE_CXX_FLAGS_RELEASE "-pipe -O2 -s")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/out)

include(GNUInstallDirs)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/zlib
        ${PROJECT_SOURCE_DIR}/src/sparse
        ${PROJECT_SOURCE_DIR}/src/sparse/android-base
        ${PROJECT_SOURCE_DIR}/src/libpng
        ${PROJECT_SOURCE_DIR}/src/minizip
        #${PROJECT_SOURCE_DIR}/src/openssl
        #${PROJECT_SOURCE_DIR}/src/e2fsdroid
        #${PROJECT_SOURCE_DIR}/src/e2fsdroid/ext2fs
)
# Section: zlib
set(ZLIB_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/zlib/adler32.c
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

# Section: libpng
set(LIBPNG_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/libpng/png.c
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

# Section: minizip
set(LIBMINIZIP_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/minizip/ioapi.c
${CMAKE_CURRENT_SOURCE_DIR}/src/minizip/iowin32.c
${CMAKE_CURRENT_SOURCE_DIR}/src/minizip/miniunz.c
${CMAKE_CURRENT_SOURCE_DIR}/src/minizip/minizip.c
${CMAKE_CURRENT_SOURCE_DIR}/src/minizip/mztools.c
${CMAKE_CURRENT_SOURCE_DIR}/src/minizip/unzip.c
${CMAKE_CURRENT_SOURCE_DIR}/src/minizip/zip.c
)
add_library(minizip STATIC ${LIBMINIZIP_SRC_FILES})

target_link_libraries(png z minizip)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

file(GLOB SRCS
        "${PROJECT_SOURCE_DIR}/include/*"
        "${PROJECT_SOURCE_DIR}/src/sparse/android-base/*"
        "${PROJECT_SOURCE_DIR}/src/sparse/*"
        "${PROJECT_SOURCE_DIR}/src/*"
        #"${PROJECT_SOURCE_DIR}/src/openssl/*"
        # "${PROJECT_SOURCE_DIR}/src/e2fsdroid/*"
        # "${PROJECT_SOURCE_DIR}/src/e2fsdroid/ext2fs/*"
)
add_executable(utils ${SRCS})
target_link_libraries(utils png z)
