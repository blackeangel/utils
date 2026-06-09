# patch_lzo.cmake
# Replaces the outdated cmake_minimum_required in LZO's CMakeLists.txt
# with a version that CMake 3.25+ accepts without errors.
# Called by FetchContent PATCH_COMMAND for the lzo dependency.

if(NOT DEFINED FILE)
    message(FATAL_ERROR "patch_lzo.cmake: FILE variable not set")
endif()

file(READ "${FILE}" content)

# Replace the old minimum version line
string(REPLACE
    "cmake_minimum_required(VERSION 2.8"
    "cmake_minimum_required(VERSION 3.5"
    content "${content}"
)

file(WRITE "${FILE}" "${content}")
message(STATUS "patch_lzo.cmake: patched ${FILE}")
