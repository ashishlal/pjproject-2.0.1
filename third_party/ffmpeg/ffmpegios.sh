#!/bin/sh
trap exit ERR
ARCH=$1
DEVPATH=$2
CC=$DEVPATH/usr/bin/gcc
IPHONESDK=$3
CPU="cortex-a8"
FFMPEG_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
X264_DIR="$FFMPEG_DIR/../x264"
cd $X264_DIR
./iosbuild.sh $ARCH $DEVPATH $IPHONESDK
cd $FFMPEG_DIR
X264_DEST="$X264_DIR/x264-$ARCH"
X264INCLUDE=${X264_DEST}/include
X264LIB=${X264_DEST}/lib
MY_FFMPEG_ARCH_FLAG=""
case "$ARCH" in 
armv6)
    CPU="arm1176jzf-s"
    MY_FFMPEG_ARCH_FLAG="--arch=arm"
    ;;
armv7)
    CPU="cortex-a8"
    MY_FFMPEG_ARCH_FLAG="--arch=arm"
    ;;
i386)
    CPU="i386"
    MY_FFMPEG_ARCH_FLAG="--arch=i686"
    ;;
default)
    ;;
esac

echo Configure FFMPEG for $ARCH build
./configure \
--cc=$CC \
--as="gas-preprocessor.pl $CC" \
--nm="$DEVPATH/usr/bin/nm" \
--sysroot=$DEVPATH/SDKs/$IPHONESDK \
--target-os=darwin \
$MY_FFMPEG_ARCH_FLAG \
--cpu=$CPU \
--enable-pic \
--extra-cflags="-arch $ARCH -I$X264INCLUDE" \
--extra-ldflags="-arch $ARCH -isysroot $DEVPATH/SDKs/$IPHONESDK -L$X264LIB" \
--prefix=compiled/$ARCH \
--enable-cross-compile \
--enable-nonfree \
--enable-gpl \
--disable-bzlib \
--disable-armv5te \
--disable-swscale-alpha \
--disable-doc \
--disable-ffmpeg \
--disable-ffplay \
--disable-ffprobe \
--disable-ffserver \
--enable-libx264 \
--enable-encoder=libx264 \
--enable-encoder=libx264rgb \
--enable-decoder=h264 \
--enable-encoder=h263 \
--enable-decoder=h263 \
--disable-asm \
--disable-debug

