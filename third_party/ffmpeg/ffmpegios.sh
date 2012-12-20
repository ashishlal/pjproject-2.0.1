ARCH=$1
DEVPATH=$2
CC=$3
IPHONESDK=$4
CPU="cortex-a8"

case "$ARCH" in 
armv6)
    CPU="arm1176jzf-s"
    ;;
armv7)
    CPU="cortex-a8"
    ;;
default)
    ;;
esac

echo Configure for $ARCH build
./configure \
--cc=$CC \
--as="gas-preprocessor.pl $CC" \
--nm="$DEVPATH/usr/bin/nm" \
--sysroot=$DEVPATH/SDKs/$IPHONESDK \
--target-os=darwin \
--arch=arm \
--cpu=$CPU \
--enable-pic \
--extra-cflags="-arch $ARCH" \
--extra-ldflags="-arch $ARCH -isysroot $DEVPATH/SDKs/$IPHONESDK" \
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
--disable-asm \
--disable-debug

