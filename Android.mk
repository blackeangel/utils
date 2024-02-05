LOCAL_PATH := $(call my-dir)

#####################################################################################
#utils
#####################################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := bin_utils

LOCAL_C_INCLUDES :=

LOCAL_CXXFLAGS := -fexceptions -std=c++2a -O2

LOCAL_LDFLAGS := -fPIE -static -ldl

#LOCAL_SRC_FILES := main.cpp copy.cpp cut.cpp delgaaps.cpp foffset.cpp insert.cpp sdat2img.cpp utils.cpp writekey.cpp hexpatch.cpp kerver.cpp
#traverse all the directory and subdirectory
define walk
  $(wildcard $(1)) $(foreach e, $(wildcard $(1)/*), $(call walk, $(e)))
endef

#find all the file recursively under jni/
ALLFILES = $(call walk, $(LOCAL_PATH))
FILE_LIST := $(filter %.cpp, %.c, $(ALLFILES))

LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

include $(BUILD_EXECUTABLE)

