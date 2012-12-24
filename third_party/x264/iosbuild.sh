#!/bin/sh

trap exit ERR

ARCH=$1
DEVPATH=$2
IPHONESDK=$3
BUILD_LIBS="libx264.a"
OUTPUT_DIR="x264-$ARCH"
[ -d $OUTPUT_DIR ] || mkdir -p $OUTPUT_DIR
[ -d $OUTPUT_DIR/lib ] || mkdir $OUTPUT_DIR/lib
[ -d $OUTPUT_DIR/include ] || mkdir $OUTPUT_DIR/include

case "$ARCH" in
armv6)
    EXTRA_FLAG="--disable-asm"
    ;;
armv7)
    EXTRA_FLAG=""
    ;;
default)
    ;;
esac

echo "Build x264 for $ARCH"
CC=$DEVPATH/usr/bin/gcc
./configure --host=arm-apple-darwin --sysroot=$DEVPATH/SDKs/$IPHONESDK --prefix=$OUTPUT_DIR --extra-cflags="-arch $ARCH" --extra-ldflags="-L$DEVPATH/SDKs/$IPHONESDK/usr/lib/system -arch $ARCH" --enable-pic --enable-static $EXTRA_FLAG

make && make install
