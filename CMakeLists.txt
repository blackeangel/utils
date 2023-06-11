cmake_minimum_required(VERSION 3.20)
project(utils)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(utils main.cpp copy.cpp cut.cpp delgaaps.cpp foffset.cpp insert.cpp sdat2img.cpp utils.cpp writekey.cpp hexpatch.cpp)
