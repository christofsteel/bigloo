#! /bin/bash

# TODO: make ANDROIDROOT and BS_BIGLOO parameters

if [ "$(basename $0)" == "build-android.sh" ]; then
   set -e
fi

# root for all things android
ANDROIDROOT=$HOME/src/works/inria/android
# ANDROIDROOT=/misc/virtual/android

export ANDSRC=$ANDROIDROOT/eclair-git
export ANDSDK=$ANDROIDROOT/android-sdk-linux

# droid-wrapper
# http://github.com/tmurakam/droid-wrapper/
export DROID_ROOT=$ANDSRC
# 3 for cupcake
# 5 for eclair
export DROID_TARGET=5

# bigloo/gcc
export BS_BIGLOO=$HOME/local
export CC=$ANDROIDROOT/droid-wrapper/bin/droid-gcc

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$BS_BIGLOO/lib

if [ "$1" == "configure" ]; then
   ./configure \
      --prefix=$HOME/local/soft/$(basename $(pwd))-android \
      --stack-check=no \
      --disable-srfi27 \
      --gccustomversion=gc-7.2alpha4 \
      --build-bindir=$BS_BIGLOO/bin \
      --hostsh=arch/android/android-target.sh
   shift
fi

if [ "$1" == "build" ]; then
   nice -n 19 make
   shift
fi
