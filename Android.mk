LOCAL_PATH := $(call my-dir)

#####################################################################################
#utils
#####################################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := bin_utils

LOCAL_C_INCLUDES :=

LOCAL_CXXFLAGS := -fexceptions -std=c++2a -O3

LOCAL_LDFLAGS := -fPIE -static -ldl

LOCAL_SRC_FILES := main.cpp copy.cpp cut.cpp delgaaps.cpp foffset.cpp insert.cpp sdat2img.cpp utils.cpp writekey.cpp hexpatch.cpp

include $(BUILD_EXECUTABLE)

