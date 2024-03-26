cmake_minimum_required(VERSION 3.20)
project(utils)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
#set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/out)
# set(ZLIB_Dir "${CMAKE_CURRENT_SOURCE_DIR}/zlib")
# find_package(ZLIB)
# target_link_libraries(utils ZLIB::ZLIB)
#include_directories(include/zlib include/zlib)

#add_executable(utils main.cpp copy.cpp cut.cpp delgaaps.cpp foffset.cpp insert.cpp sdat2img.cpp utils.cpp writekey.cpp hexpatch.cpp kerver.cpp)
include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/zlib
        #${PROJECT_SOURCE_DIR}/src/openssl
        #${PROJECT_SOURCE_DIR}/src/e2fsdroid
        #${PROJECT_SOURCE_DIR}/src/e2fsdroid/ext2fs
)
file(GLOB SRCS
        "${PROJECT_SOURCE_DIR}/include/*"
        "${PROJECT_SOURCE_DIR}/src/*"
        "${PROJECT_SOURCE_DIR}/src/zlib/*"
        #"${PROJECT_SOURCE_DIR}/src/openssl/*"
       # "${PROJECT_SOURCE_DIR}/src/e2fsdroid/*"
       # "${PROJECT_SOURCE_DIR}/src/e2fsdroid/ext2fs/*"
)
add_executable(utils ${SRCS})
