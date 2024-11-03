APP_PLATFORM = latest
#APP_ABI := armeabi-v7a
#APP_ABI := all
APP_ABI := armeabi-v7a arm64-v8a
APP_CPPFLAGS := -fexceptions -std=c++2a -pipe -O2 -s
APP_STL := c++_static
APP_OPTIM := release
APP_STRIP_MODE := --strip-all
APP_BUILD_SCRIPT := Android.mk
