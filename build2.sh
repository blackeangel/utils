#!/bin/sh
home_dir=$(dirname $HOME)
path_ndk="$home_dir/android-ndk"

ROOT_DIR=`pwd`
DIST_DIR=$ROOT_DIR/out
BUILD_DIR=$ROOT_DIR/build

ABIS="armeabi-v7a arm64-v8a"

export ANDROID_NDK=$path_ndk

build_static() {
  arch=$1
  rm -rf $BUILD_DIR
  mkdir $BUILD_DIR
  cd $BUILD_DIR
  cmake -DANDROID_ABI=$arch \
      -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_NATIVE_API_LEVEL=30 \
      -GNinja ..
  ninja
  if [ ! -d $DIST_DIR/$arch ]; then
    mkdir -p $DIST_DIR/$arch
  fi
cp $BUILD_DIR/utils $DIST_DIR/$arch/bin_utils
strip -s $DIST_DIR/$arch/bin_utils
}

cd $ROOT_DIR
#build_static "arm64-v8a"

for arch in $ABIS
do
  build_static $arch
done

cd ../../..
name_file="bin_utils_"$(date +%Y%m%d%H%M)
cd ./out
../../zip -9D /sdcard/$name_file.zip ./*
