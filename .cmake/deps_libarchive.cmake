# =============================================================================
# deps.cmake — FetchContent build of all libarchive dependencies.
#
# Included by CMakeLists.txt when TARREPACKER_FETCH_DEPS=ON.
# Every library is built as a static archive (.a / .lib).
#
# Dependency graph:
#   zlib-ng   → gz filter  + zip/gzip format support
#   bzip2     → bz2 filter + bzip2 format support
#   xz-utils  → xz/lzma filter + lzma format support
#   zstd      → zstd filter support
#   lz4       → lz4 filter support
#   expat     → XAR format XML parsing
#   mbedTLS   → crypto for XAR checksums, ZIP AES encryption
#   libarchive → the actual archive library (uses all of the above)
#
# Note: libb2 (BLAKE2) is NOT fetched — libarchive ships its own
#       reference BLAKE2 implementation (archive_blake2s_ref.c etc.)
#       and falls back to it automatically when libb2 is not found.
#
# Note: LZO is intentionally skipped — it is GPL-licensed.
# =============================================================================

include(FetchContent)
include(ProcessorCount)
ProcessorCount(CPU_COUNT)

# ---------------------------------------------------------------------------
# Global flags — applied to ALL FetchContent sub-projects
# ---------------------------------------------------------------------------
set(BUILD_SHARED_LIBS    OFF CACHE BOOL "" FORCE)  # static only
set(BUILD_TESTING        OFF CACHE BOOL "" FORCE)  # no tests anywhere
set(BUILD_TESTS          OFF CACHE BOOL "" FORCE)
set(ENABLE_TESTING       OFF CACHE BOOL "" FORCE)

# Tell CMake not to rebuild unchanged source files
set(CMAKE_SKIP_INSTALL_RULES     ON  CACHE BOOL "" FORCE)
set(CMAKE_EXPORT_NO_PACKAGE_REGISTRY ON CACHE BOOL "" FORCE)

# =============================================================================
# 1. zlib-ng  (compat API = drop-in zlib replacement, faster)
# =============================================================================
message(STATUS "[deps] Fetching zlib-ng …")

set(ZLIB_COMPAT         ON  CACHE BOOL "" FORCE)
set(ZLIB_ENABLE_TESTS   OFF CACHE BOOL "" FORCE)
set(ZLIBNG_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(WITH_GTEST          OFF CACHE BOOL "" FORCE)
set(INSTALL_UTILS       OFF CACHE BOOL "" FORCE)

FetchContent_Declare(zlib
    GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng.git
    GIT_TAG        2.2.2
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(zlib)

# Resolve the target name (zlib-ng names it "zlib" in compat mode)
foreach(_t zlib zlibstatic zlib-ng)
    if(TARGET ${_t})
        set(ZLIB_TARGET ${_t})
        break()
    endif()
endforeach()
if(NOT ZLIB_TARGET)
    message(FATAL_ERROR "Could not find zlib target after FetchContent")
endif()

get_target_property(_zlib_inc ${ZLIB_TARGET} INCLUDE_DIRECTORIES)
set(ZLIB_INC_DIRS "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}")

# Variables libarchive's cmake looks for (via cmake's own FindZLIB module)
set(ZLIB_FOUND        TRUE  CACHE BOOL "" FORCE)
set(ZLIB_INCLUDE_DIRS "${ZLIB_INC_DIRS}" CACHE STRING "" FORCE)
set(ZLIB_LIBRARIES    "${ZLIB_TARGET}"   CACHE STRING "" FORCE)

if(NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS ${ZLIB_TARGET})
endif()

# =============================================================================
# 2. bzip2
# =============================================================================
message(STATUS "[deps] Fetching bzip2 …")

set(ENABLE_STATIC_LIB ON  CACHE BOOL "" FORCE)
set(ENABLE_SHARED_LIB OFF CACHE BOOL "" FORCE)
set(ENABLE_APP        OFF CACHE BOOL "" FORCE)
set(ENABLE_EXAMPLES   OFF CACHE BOOL "" FORCE)
set(ENABLE_TESTS      OFF CACHE BOOL "" FORCE)
set(ENABLE_DOCS       OFF CACHE BOOL "" FORCE)

FetchContent_Declare(bzip2
    GIT_REPOSITORY https://github.com/libarchive/bzip2.git
    GIT_TAG        1ea1ac188ad4b9cb662e3f8314673c63df95a589
    GIT_SHALLOW    FALSE
)
FetchContent_MakeAvailable(bzip2)

foreach(_t bz2_static bz2)
    if(TARGET ${_t})
        set(BZIP2_TARGET ${_t})
        break()
    endif()
endforeach()
if(NOT BZIP2_TARGET)
    message(FATAL_ERROR "Could not find bzip2 target after FetchContent")
endif()

set(BZIP2_FOUND        TRUE                CACHE BOOL   "" FORCE)
set(BZIP2_INCLUDE_DIRS "${bzip2_SOURCE_DIR}" CACHE STRING "" FORCE)
set(BZIP2_LIBRARIES    "${BZIP2_TARGET}"   CACHE STRING "" FORCE)

if(NOT TARGET BZip2::BZip2)
    add_library(BZip2::BZip2 ALIAS ${BZIP2_TARGET})
endif()

# =============================================================================
# 3. xz-utils / liblzma  (xz and lzma filter support)
# =============================================================================
message(STATUS "[deps] Fetching xz-utils (liblzma) …")

set(XZ_BUILD_SHARED       OFF CACHE BOOL "" FORCE)
set(XZ_BUILD_TESTS        OFF CACHE BOOL "" FORCE)
set(XZ_BUILD_LZMALINKS    OFF CACHE BOOL "" FORCE)
set(XZ_BUILD_LZIP_DECODER OFF CACHE BOOL "" FORCE)
set(XZ_TOOL_XZ            OFF CACHE BOOL "" FORCE)
set(XZ_TOOL_XZDEC         OFF CACHE BOOL "" FORCE)
set(XZ_TOOL_LZMADEC       OFF CACHE BOOL "" FORCE)
set(XZ_TOOL_LZMAINFO      OFF CACHE BOOL "" FORCE)
set(XZ_TOOL_SCRIPTS       OFF CACHE BOOL "" FORCE)
set(ALLOW_WARNINGS        ON  CACHE BOOL "" FORCE)  # some old cmake policy warnings

FetchContent_Declare(xz
    GIT_REPOSITORY https://github.com/tukaani-project/xz.git
    GIT_TAG        v5.8.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(xz)

# xz-utils builds target "liblzma"
set(LIBLZMA_TARGET liblzma)
set(LIBLZMA_INC    "${xz_SOURCE_DIR}/src/liblzma/api")

# libarchive checks: LIBLZMA_FOUND, LIBLZMA_INCLUDE_DIR, LIBLZMA_LIBRARIES
set(LIBLZMA_FOUND        TRUE              CACHE BOOL   "" FORCE)
set(LIBLZMA_INCLUDE_DIR  "${LIBLZMA_INC}" CACHE STRING "" FORCE)
set(LIBLZMA_INCLUDE_DIRS "${LIBLZMA_INC}" CACHE STRING "" FORCE)
set(LIBLZMA_LIBRARIES    "liblzma"        CACHE STRING "" FORCE)

if(NOT TARGET LibLZMA::LibLZMA)
    add_library(LibLZMA::LibLZMA ALIAS liblzma)
endif()

# =============================================================================
# 4. zstd
# =============================================================================
message(STATUS "[deps] Fetching zstd …")

set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_CONTRIB  OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_SHARED   OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_STATIC   ON  CACHE BOOL "" FORCE)

FetchContent_Declare(zstd
    GIT_REPOSITORY https://github.com/facebook/zstd.git
    GIT_TAG        v1.5.6
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  build/cmake
)
FetchContent_MakeAvailable(zstd)

# zstd static target is "libzstd_static"
set(ZSTD_TARGET libzstd_static)
set(ZSTD_INC    "${zstd_SOURCE_DIR}/lib")

# libarchive checks: ZSTD_FOUND, ZSTD_INCLUDE_DIR, ZSTD_LIBRARY
set(ZSTD_FOUND       TRUE          CACHE BOOL   "" FORCE)
set(ZSTD_INCLUDE_DIR "${ZSTD_INC}" CACHE STRING "" FORCE)
set(ZSTD_LIBRARY     "libzstd_static" CACHE STRING "" FORCE)

if(NOT TARGET zstd::libzstd_static)
    add_library(zstd::libzstd_static ALIAS libzstd_static)
endif()

# =============================================================================
# 5. lz4
# =============================================================================
message(STATUS "[deps] Fetching lz4 …")

set(LZ4_BUILD_CLI    OFF CACHE BOOL "" FORCE)
set(LZ4_BUILD_LEGACY OFF CACHE BOOL "" FORCE)

FetchContent_Declare(lz4
    GIT_REPOSITORY https://github.com/lz4/lz4.git
    GIT_TAG        v1.10.0
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  build/cmake
)
FetchContent_MakeAvailable(lz4)

# lz4 static target is "lz4_static"
set(LZ4_TARGET lz4_static)
set(LZ4_INC    "${lz4_SOURCE_DIR}/lib")

# libarchive checks: LZ4_FOUND, LZ4_INCLUDE_DIR, LZ4_LIBRARY
set(LZ4_FOUND       TRUE          CACHE BOOL   "" FORCE)
set(LZ4_INCLUDE_DIR "${LZ4_INC}"  CACHE STRING "" FORCE)
set(LZ4_LIBRARY     "lz4_static"  CACHE STRING "" FORCE)

if(NOT TARGET LZ4::lz4_static)
    add_library(LZ4::lz4_static ALIAS lz4_static)
endif()

# =============================================================================
# 6. expat  (XML parser — needed for XAR format)
# =============================================================================
message(STATUS "[deps] Fetching expat …")

set(EXPAT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(EXPAT_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(EXPAT_BUILD_TOOLS    OFF CACHE BOOL "" FORCE)
set(EXPAT_SHARED_LIBS    OFF CACHE BOOL "" FORCE)
set(EXPAT_BUILD_DOCS     OFF CACHE BOOL "" FORCE)

FetchContent_Declare(expat
    GIT_REPOSITORY https://github.com/libexpat/libexpat.git
    GIT_TAG        R_2_8_0
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  expat
)
FetchContent_MakeAvailable(expat)

set(EXPAT_INC "${expat_SOURCE_DIR}/expat/lib")

# libarchive checks: EXPAT_FOUND, EXPAT_INCLUDE_DIR, EXPAT_LIBRARIES
set(EXPAT_FOUND        TRUE         CACHE BOOL   "" FORCE)
set(EXPAT_INCLUDE_DIR  "${EXPAT_INC}" CACHE STRING "" FORCE)
set(EXPAT_INCLUDE_DIRS "${EXPAT_INC}" CACHE STRING "" FORCE)
set(EXPAT_LIBRARIES    "expat"      CACHE STRING "" FORCE)

if(NOT TARGET expat::expat)
    add_library(expat::expat ALIAS expat)
endif()

# =============================================================================
# 7. mbedTLS  (crypto — XAR checksums, ZIP AES encryption)
# =============================================================================
message(STATUS "[deps] Fetching mbedTLS …")

set(MBEDTLS_FATAL_WARNINGS OFF CACHE BOOL "" FORCE)
set(ENABLE_TESTING         OFF CACHE BOOL "" FORCE)
set(ENABLE_PROGRAMS        OFF CACHE BOOL "" FORCE)
set(MBEDTLS_AS_SUBPROJECT  ON  CACHE BOOL "" FORCE)

FetchContent_Declare(mbedtls
    GIT_REPOSITORY https://github.com/Mbed-TLS/mbedtls.git
    GIT_TAG        v3.6.4
    GIT_SHALLOW    TRUE
)
# mbedTLS 3.6.x declares cmake_minimum_required(VERSION 3.5.1..3.27).
# CMake 4.x warns about < 3.10. Set minimum to suppress the warning.
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
FetchContent_MakeAvailable(mbedtls)
unset(CMAKE_POLICY_VERSION_MINIMUM)

set(MBEDTLS_INC "${mbedtls_SOURCE_DIR}/include")

# libarchive/FindMbedTLS.cmake checks:
#   MBEDTLS_FOUND, MBEDTLS_INCLUDE_DIRS
#   MBEDTLS_TLS_LIBRARY, MBEDTLS_CRYPTO_LIBRARY, MBEDTLS_X509_LIBRARY
#   MBEDTLS_LIBRARIES = all three combined
set(MBEDTLS_FOUND          TRUE          CACHE BOOL   "" FORCE)
set(MBEDTLS_INCLUDE_DIRS   "${MBEDTLS_INC}" CACHE STRING "" FORCE)
set(MBEDTLS_TLS_LIBRARY    "mbedtls"     CACHE STRING "" FORCE)
set(MBEDTLS_CRYPTO_LIBRARY "mbedcrypto"  CACHE STRING "" FORCE)
set(MBEDTLS_X509_LIBRARY   "mbedx509"    CACHE STRING "" FORCE)
set(MBEDTLS_LIBRARIES      "mbedtls;mbedx509;mbedcrypto" CACHE STRING "" FORCE)

# =============================================================================
# 8. libarchive  (the actual library — uses all of the above)
# =============================================================================
# 8. LZO  (lossless data compression — GPL v2+)
#
# Used by libarchive for lzop (.lzo) format support.
# Source: https://github.com/nemequ/lzo  (CMake-enabled mirror of
#         http://www.oberhumer.com/opensource/lzo/, version 2.10)
# =============================================================================
message(STATUS "[deps] Fetching LZO …")

set(ENABLE_STATIC ON  CACHE BOOL "" FORCE)
set(ENABLE_SHARED OFF CACHE BOOL "" FORCE)

FetchContent_Declare(lzo
    GIT_REPOSITORY https://github.com/nemequ/lzo.git
    GIT_TAG        0083878c235a89ef96a009d1ff0b500f3a364e4b
    GIT_SHALLOW    FALSE
)
# LZO 2.10 declares cmake_minimum_required(VERSION 2.8).
# CMake 4.x removed compatibility with < 3.5 and will hard-error.
# CMAKE_POLICY_VERSION_MINIMUM tells CMake to treat the sub-project as if it
# declared at least 3.5, without touching the source files.
# LZO 2.10 has cmake_minimum_required(VERSION 3.0) — CMake 4.x hard-errors.
# We must patch BEFORE add_subdirectory processes the file.
# FetchContent_Populate downloads source without configuring it,
# then we patch, then add_subdirectory with the fixed file.
FetchContent_Populate(lzo)
file(READ "${lzo_SOURCE_DIR}/CMakeLists.txt" _lzo_cmake)
# Fix cmake_minimum_required for CMake 4.x
string(REPLACE
    "cmake_minimum_required(VERSION 3.0"
    "cmake_minimum_required(VERSION 3.5"
    _lzo_cmake "${_lzo_cmake}")
# Patch lzo_add_executable macro: add EXCLUDE_FROM_ALL to add_executable so
# test/example binaries are defined but never compiled as part of the default build.
# NOTE: "return()" inside a macro() breaks add_subdirectory() in CMake — do NOT use it.
string(REPLACE
    "    add_executable(${t} ${ARGN})"
    "    add_executable(${t} EXCLUDE_FROM_ALL ${ARGN})"
    _lzo_cmake "${_lzo_cmake}")
# testmini is defined outside the macro — patch it too
string(REPLACE
    "    add_executable(testmini minilzo/testmini.c minilzo/minilzo.c)"
    "    add_executable(testmini EXCLUDE_FROM_ALL minilzo/testmini.c minilzo/minilzo.c)"
    _lzo_cmake "${_lzo_cmake}")
file(WRITE "${lzo_SOURCE_DIR}/CMakeLists.txt" "${_lzo_cmake}")
add_subdirectory("${lzo_SOURCE_DIR}" "${lzo_BINARY_DIR}" EXCLUDE_FROM_ALL)

# LZO's CMakeLists creates target "lzo_static_lib" (OUTPUT_NAME → liblzo2)
# Include dir contains the "lzo/" sub-directory with lzoconf.h / lzo1x.h
set(LZO_TARGET  lzo_static_lib)
set(LZO_INC     "${lzo_SOURCE_DIR}/include")

# libarchive searches with ENABLE_LZO=ON:
#   FIND_PATH(LZO2_INCLUDE_DIR lzo/lzoconf.h)
#   FIND_LIBRARY(LZO2_LIBRARY NAMES lzo2 liblzo2)
# We bypass find_path/find_library by pre-setting the cache variables it
# would have produced.
set(LZO2_FOUND       TRUE           CACHE BOOL   "" FORCE)
set(LZO2_INCLUDE_DIR "${LZO_INC}"   CACHE PATH   "" FORCE)
set(LZO2_LIBRARY     "lzo_static_lib" CACHE STRING "" FORCE)

# =============================================================================
# 9. libarchive  (the actual library — uses all of the above)
# =============================================================================
message(STATUS "[deps] Fetching libarchive …")

# Disable what we didn't build / don't need
set(ENABLE_TEST       OFF CACHE BOOL "" FORCE)
set(ENABLE_TAR        OFF CACHE BOOL "" FORCE)
set(ENABLE_CPIO       OFF CACHE BOOL "" FORCE)
set(ENABLE_CAT        OFF CACHE BOOL "" FORCE)
set(ENABLE_UNZIP      OFF CACHE BOOL "" FORCE)
set(ENABLE_INSTALL    OFF CACHE BOOL "" FORCE)
set(ENABLE_ACL        OFF CACHE BOOL "" FORCE)  # requires libacl (Linux-only)
set(ENABLE_ICONV      OFF CACHE BOOL "" FORCE)  # optional; skip for portability
set(ENABLE_NETTLE     OFF CACHE BOOL "" FORCE)  # we use mbedtls instead
set(ENABLE_OPENSSL    OFF CACHE BOOL "" FORCE)  # we use mbedtls instead
set(ENABLE_CNG        OFF CACHE BOOL "" FORCE)  # Windows CNG (optional)
set(ENABLE_LIBGCC     OFF CACHE BOOL "" FORCE)
set(ENABLE_PCREPOSIX  OFF CACHE BOOL "" FORCE)
set(ENABLE_PCRE2POSIX OFF CACHE BOOL "" FORCE)
set(ENABLE_LIBXML2    OFF CACHE BOOL "" FORCE)  # we use expat instead

# Enable everything we DID build
set(ENABLE_ZLIB    ON CACHE BOOL "" FORCE)
set(ENABLE_BZip2   ON CACHE BOOL "" FORCE)
set(ENABLE_LZMA    ON CACHE BOOL "" FORCE)
set(ENABLE_ZSTD    ON CACHE BOOL "" FORCE)
set(ENABLE_LZ4     ON CACHE BOOL "" FORCE)
set(ENABLE_LZO     ON CACHE BOOL "" FORCE)
set(ENABLE_EXPAT   ON CACHE BOOL "" FORCE)
set(ENABLE_MBEDTLS ON CACHE BOOL "" FORCE)
set(ENABLE_LIBB2   OFF CACHE BOOL "" FORCE)  # use libarchive's built-in blake2
set(ENABLE_XATTR   OFF CACHE BOOL "" FORCE)  # requires kernel headers; optional

FetchContent_Declare(libarchive
    GIT_REPOSITORY https://github.com/libarchive/libarchive.git
    GIT_TAG        v3.7.4
    GIT_SHALLOW    TRUE
)
# =============================================================================
# BRIDGE: Set exact cache variable names that libarchive's internal
# find_package / find_library calls use, pointing to OUR static .a files.
# Without this, libarchive picks up system .so files and the final binary
# has unexpected dynamic dependencies.
#
# Variable naming rules (these are the cache names CMake's Find modules use):
#   FindZLIB       → ZLIB_INCLUDE_DIR,  ZLIB_LIBRARY              (no S, no _RELEASE)
#   FindBZip2      → BZIP2_INCLUDE_DIR, BZIP2_LIBRARY_RELEASE
#   FindLibLZMA    → LIBLZMA_INCLUDE_DIR, LIBLZMA_LIBRARY (legacy)
#                  → LibLZMA_INCLUDE_DIR, LibLZMA_LIBRARY  (≥3.14)
#   ZSTD (custom)  → ZSTD_INCLUDE_DIR,  ZSTD_LIBRARY
#   LZ4  (custom)  → LZ4_INCLUDE_DIR,   LZ4_LIBRARY
#   LZO2 (custom)  → LZO2_INCLUDE_DIR,  LZO2_LIBRARY
#   FindEXPAT      → EXPAT_INCLUDE_DIR, EXPAT_LIBRARY
#   FindMbedTLS    → MBEDTLS_LIBRARY, MBEDX509_LIBRARY, MBEDCRYPTO_LIBRARY
#                    (NOT MBEDTLS_TLS_LIBRARY — that was our earlier bug!)
# =============================================================================

set(ZLIB_INCLUDE_DIR   "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}"
    CACHE PATH     "" FORCE)
set(ZLIB_LIBRARY       "${zlib_BINARY_DIR}/libz.a"
    CACHE FILEPATH "" FORCE)
set(ZLIB_FOUND TRUE CACHE BOOL "" FORCE)

set(BZIP2_INCLUDE_DIR      "${bzip2_SOURCE_DIR}"
    CACHE PATH     "" FORCE)
set(BZIP2_LIBRARY_RELEASE  "${bzip2_BINARY_DIR}/libbz2_static.a"
    CACHE FILEPATH "" FORCE)
set(BZIP2_FOUND TRUE CACHE BOOL "" FORCE)

# Both old (LIBLZMA_) and new (LibLZMA_) spellings
set(LIBLZMA_INCLUDE_DIR "${xz_SOURCE_DIR}/src/liblzma/api"
    CACHE PATH     "" FORCE)
set(LIBLZMA_LIBRARY     "${xz_BINARY_DIR}/liblzma.a"
    CACHE FILEPATH "" FORCE)
set(LibLZMA_INCLUDE_DIR "${xz_SOURCE_DIR}/src/liblzma/api"
    CACHE PATH     "" FORCE)
set(LibLZMA_LIBRARY     "${xz_BINARY_DIR}/liblzma.a"
    CACHE FILEPATH "" FORCE)
set(LIBLZMA_FOUND TRUE CACHE BOOL "" FORCE)
set(LibLZMA_FOUND TRUE CACHE BOOL "" FORCE)
# libarchive checks these capability vars via try_compile; since we already
# built liblzma from source they are all guaranteed to exist.
set(LIBLZMA_HAS_AUTO_DECODER   TRUE CACHE BOOL "" FORCE)
set(LIBLZMA_HAS_EASY_ENCODER   TRUE CACHE BOOL "" FORCE)
set(LIBLZMA_HAS_LZMA_PRESET    TRUE CACHE BOOL "" FORCE)

set(ZSTD_INCLUDE_DIR "${zstd_SOURCE_DIR}/lib"
    CACHE PATH     "" FORCE)
set(ZSTD_LIBRARY     "${zstd_BINARY_DIR}/lib/libzstd.a"
    CACHE FILEPATH "" FORCE)
set(ZSTD_FOUND TRUE CACHE BOOL "" FORCE)
# libarchive uses CHECK_FUNCTION_EXISTS (line 664-665 in its CMakeLists.txt):
#   CHECK_FUNCTION_EXISTS(ZSTD_decompressStream HAVE_LIBZSTD)
#   CHECK_FUNCTION_EXISTS(ZSTD_compressStream   HAVE_ZSTD_compressStream)
# These try_compile calls need CMAKE_REQUIRED_LIBRARIES set to our .a.
# We bypass them by pre-setting the result cache variables.
set(HAVE_LIBZSTD            1 CACHE STRING "" FORCE)
set(HAVE_ZSTD_compressStream 1 CACHE STRING "" FORCE)

# lz4 OUTPUT_NAME is "lz4" (without _static suffix) → liblz4.a
set(LZ4_INCLUDE_DIR "${lz4_SOURCE_DIR}/lib"
    CACHE PATH     "" FORCE)
set(LZ4_LIBRARY     "${lz4_BINARY_DIR}/liblz4.a"
    CACHE FILEPATH "" FORCE)
set(LZ4_FOUND TRUE CACHE BOOL "" FORCE)

set(LZO2_INCLUDE_DIR "${lzo_SOURCE_DIR}/include"
    CACHE PATH     "" FORCE)
set(LZO2_LIBRARY     "${lzo_BINARY_DIR}/liblzo2.a"
    CACHE FILEPATH "" FORCE)
set(LZO2_FOUND TRUE CACHE BOOL "" FORCE)

set(EXPAT_INCLUDE_DIR "${expat_SOURCE_DIR}/expat/lib"
    CACHE PATH     "" FORCE)
set(EXPAT_LIBRARY     "${expat_BINARY_DIR}/libexpat.a"
    CACHE FILEPATH "" FORCE)
set(EXPAT_FOUND TRUE CACHE BOOL "" FORCE)

# CRITICAL FIX: correct variable names for libarchive's FindMbedTLS.cmake
# We previously used MBEDTLS_TLS_LIBRARY which is WRONG.
set(MBEDTLS_INCLUDE_DIRS  "${mbedtls_SOURCE_DIR}/include"
    CACHE PATH     "" FORCE)
set(MBEDTLS_LIBRARY       "${mbedtls_BINARY_DIR}/library/libmbedtls.a"
    CACHE FILEPATH "" FORCE)
set(MBEDX509_LIBRARY      "${mbedtls_BINARY_DIR}/library/libmbedx509.a"
    CACHE FILEPATH "" FORCE)
set(MBEDCRYPTO_LIBRARY    "${mbedtls_BINARY_DIR}/library/libmbedcrypto.a"
    CACHE FILEPATH "" FORCE)
set(MBEDTLS_LIBRARIES
    "${mbedtls_BINARY_DIR}/library/libmbedtls.a;${mbedtls_BINARY_DIR}/library/libmbedx509.a;${mbedtls_BINARY_DIR}/library/libmbedcrypto.a"
    CACHE STRING "" FORCE)
set(MBEDTLS_FOUND TRUE CACHE BOOL "" FORCE)

# libarchive 3.7.4 has CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12) — CMake 4.x hard-errors.
# Patch the file before add_subdirectory processes it.
FetchContent_Populate(libarchive)
file(READ "${libarchive_SOURCE_DIR}/CMakeLists.txt" _la_cmake)
string(REPLACE
    "CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12"
    "cmake_minimum_required(VERSION 3.5"
    _la_cmake "${_la_cmake}")
file(WRITE "${libarchive_SOURCE_DIR}/CMakeLists.txt" "${_la_cmake}")
add_subdirectory("${libarchive_SOURCE_DIR}" "${libarchive_BINARY_DIR}" EXCLUDE_FROM_ALL)

# With BUILD_SHARED_LIBS=OFF libarchive creates "archive_static"
if(NOT TARGET archive_static)
    message(FATAL_ERROR "[deps] Expected target 'archive_static' from libarchive")
endif()

set(LIBARCHIVE_TARGET  archive_static)
set(LIBARCHIVE_INCLUDE "${libarchive_SOURCE_DIR}/libarchive")

if(NOT TARGET LibArchive::LibArchive)
    add_library(LibArchive::LibArchive ALIAS archive_static)
endif()

# =============================================================================
# Summary
# =============================================================================
message(STATUS "---------------------------------------------")
message(STATUS " libarchive static dependency tree:")
message(STATUS "   zlib-ng    2.2.2  → ${ZLIB_TARGET}")
message(STATUS "   bzip2      1.0.8  → ${BZIP2_TARGET}")
message(STATUS "   xz/liblzma 5.8.1  → liblzma")
message(STATUS "   zstd       1.5.6  → libzstd_static")
message(STATUS "   lz4        1.10.0 → lz4_static")
message(STATUS "   lzo        2.10   → lzo_static_lib")
message(STATUS "   expat      2.8.0  → expat")
message(STATUS "   mbedTLS    3.6.4  → mbedtls+mbedcrypto+mbedx509")
message(STATUS "   blake2            → built-in (libarchive)")
message(STATUS "   libarchive 3.7.4  → archive_static")
message(STATUS "---------------------------------------------")
