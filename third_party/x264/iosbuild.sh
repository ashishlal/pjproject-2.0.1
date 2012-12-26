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
MYEXTRA_FLAG=""
echo "Build x264 for $ARCH"
case "$ARCH" in
armv6)
    EXTRA_FLAG="--disable-asm"
    CC=$DEVPATH/usr/bin/gcc
    ./configure --host=arm-apple-darwin --sysroot=$DEVPATH/SDKs/$IPHONESDK --prefix=$OUTPUT_DIR --extra-cflags="-arch $ARCH" --extra-ldflags="-L$DEVPATH/SDKs/$IPHONESDK/usr/lib/system -arch $ARCH" --enable-pic --enable-static $EXTRA_FLAG
    ;;
armv7)
    EXTRA_FLAG=""
    CC=$DEVPATH/usr/bin/gcc
    ./configure --host=arm-apple-darwin --sysroot=$DEVPATH/SDKs/$IPHONESDK --prefix=$OUTPUT_DIR --extra-cflags="-arch $ARCH" --extra-ldflags="-L$DEVPATH/SDKs/$IPHONESDK/usr/lib/system -arch $ARCH" --enable-pic --enable-static $EXTRA_FLAG
    ;;
i386)
    EXTRA_FLAG="--disable-asm"
    MYEXTRA_FLAG="-02 -m32"
    CC=$DEVPATH/usr/bin/gcc 
   ./configure --sysroot=$DEVPATH/SDKs/$IPHONESDK --prefix=$OUTPUT_DIR --extra-cflags="-02 -m32" --extra-ldflags="-L$DEVPATH/SDKs/$IPHONESDK/usr/lib/system -02 -m32 -Wl,-no_pie" --host="i386-apple-darwin11"
    ;;
default)
    ;;
esac


make && make install

if [ $ARCH == "i386 " ] || [ $ARCH == "i386" ]
then
    cp libx264.a x264-i386/lib
    cp x264.h x264-i386/include
    cp x264_config.h x264-i386/include
fi
