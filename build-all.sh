#!/bin/bash

PJSIP_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
FFMPEG_DIR=$PJSIP_DIR/third_party/ffmpeg
SDL_DIR=$PJSIP_DIR/third_party/sdl
x264_DIR=$PJSIP_DIR/third_party/x264

echo "Building for armv6"
make realclean
./configure-iphone --armv6 --build-ffmpeg --build-sdl

echo "Building for armv7"
make realclean
./configure-iphone --armv7 --build-ffmpeg --build-sdl

echo "Building for i386"
make realclean
./configure-iphone --i386 --build-ffmpeg --build-sdl

echo "Creating x264 fat library"
cd $x264_DIR
./combine-libs
cd $PJSIP_DIR
[ -d compiled-universal/x264 ] || mkdir -p compiled-universal/x264
cp $x264_DIR/compiled-universal/lib/* compiled-universal/x264

echo "Creating FFMPEG fat library"
cd $FFMPEG_DIR
./combine-libs
cd $PJSIP_DIR
[ -d $PJSIP_DIR/compiled-universal/ffmpeg ] || mkdir -p $PJSIP_DIR/compiled-universal/ffmpeg
cp $FFMPEG_DIR/compiled-universal/lib/* $PJSIP_DIR/compiled-universal/ffmpeg

echo "Creating SDL fat library"
cd $SDL_DIR
./combine-libs
cd $PJSIP_DIR
[ -d $PJSIP_DIR/compiled-universal/sdl ] || mkdir -p $PJSIP_DIR/compiled-universal/sdl
cp $SDL_DIR/compiled-universal/lib/* $PJSIP_DIR/compiled-universal/sdl

echo "Creating PJSIP fat library"
cd $PJSIP_DIR
./combine-libs

