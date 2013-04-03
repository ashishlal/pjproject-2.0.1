# build/os-auto.mak.  Generated from os-auto.mak.in by configure.

export OS_CFLAGS   := $(CC_DEF)PJ_AUTOCONF=1 -O2 -Wno-unused-label -DPJ_SDK_NAME="\"iPhoneOS6.0.sdk\"" -arch armv7 -isysroot /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS6.0.sdk -I /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS6.0.sdk/System/Library/Frameworks/Accelerate.framework/Frameworks/vImage.framework/Headers -I /Users/abc/lal/mygithub/pjproject-2.0.1/third_party/ffmpeg/compiled/armv7/include -I /Users/abc/lal/mygithub/pjproject-2.0.1/third_party/sdl/build/armv7/include -I/usr/local/include  -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1

export OS_CXXFLAGS := $(CC_DEF)PJ_AUTOCONF=1 -O2 -Wno-unused-label -DPJ_SDK_NAME="\"iPhoneOS6.0.sdk\"" -arch armv7 -isysroot /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS6.0.sdk -I /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS6.0.sdk/System/Library/Frameworks/Accelerate.framework/Frameworks/vImage.framework/Headers -I /Users/abc/lal/mygithub/pjproject-2.0.1/third_party/ffmpeg/compiled/armv7/include -I /Users/abc/lal/mygithub/pjproject-2.0.1/third_party/sdl/build/armv7/include -I/usr/local/include  

export OS_LDFLAGS  := -O2 -arch armv7 -isysroot /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS6.0.sdk -framework AudioToolbox -framework Foundation -L /Users/abc/lal/mygithub/pjproject-2.0.1/third_party/sdl/build/armv7/build/.libs -L /Users/abc/lal/mygithub/pjproject-2.0.1/third_party/ffmpeg/compiled/armv7/lib -L /Users/abc/lal/mygithub/pjproject-2.0.1/third_party/x264/x264-armv7/lib -lx264 -lSDL2 -lavformat -lavcodec -lswscale -lavutil -lavdevice -lavfilter -framework OpenGLES -framework Accelerate -lz  -lpthread  -framework CoreAudio -framework CoreFoundation -framework AudioToolbox -framework CFNetwork -framework UIKit -framework UIKit -framework AVFoundation -framework CoreGraphics -framework QuartzCore -framework CoreVideo -framework CoreMedia

export OS_SOURCES  := 


