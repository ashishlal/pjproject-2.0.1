# build/os-auto.mak.  Generated from os-auto.mak.in by configure.

export OS_CFLAGS   := $(CC_DEF)PJ_AUTOCONF=1 -O2 -Wno-unused-label -DPJ_SDK_NAME="\"iPhoneSimulator5.1.sdk\"" -arch i386 -isysroot /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator5.1.sdk -I /Users/Ashish/mygithub/pjproject-2.0.1/third_party/ffmpeg/compiled/i386/include -I /Users/Ashish/mygithub/pjproject-2.0.1/third_party/sdl/build/i386/include -I/usr/local/include -02 -m32 -miphoneos-version-min=3.0 -DPJ_CONFIG_IPHONE_SIMULATOR -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1

export OS_CXXFLAGS := $(CC_DEF)PJ_AUTOCONF=1 -O2 -Wno-unused-label -DPJ_SDK_NAME="\"iPhoneSimulator5.1.sdk\"" -arch i386 -isysroot /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator5.1.sdk -I /Users/Ashish/mygithub/pjproject-2.0.1/third_party/ffmpeg/compiled/i386/include -I /Users/Ashish/mygithub/pjproject-2.0.1/third_party/sdl/build/i386/include -I/usr/local/include -02 -m32 -miphoneos-version-min=3.0 -DPJ_CONFIG_IPHONE_SIMULATOR 

export OS_LDFLAGS  := -O2 -arch i386 -isysroot /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator5.1.sdk -framework AudioToolbox -framework Foundation -L /Users/Ashish/mygithub/pjproject-2.0.1/third_party/sdl/build/i386/build/.libs -L /Users/Ashish/mygithub/pjproject-2.0.1/third_party/ffmpeg/compiled/i386/lib -L /Users/Ashish/mygithub/pjproject-2.0.1/third_party/x264/x264-i386/lib -lx264 -lSDL2 -lavformat -lavcodec -lswscale -lavutil -lavdevice -lavfilter -framework OpenGLES -lz -02 -m32 -lpthread  -framework CoreAudio -framework CoreFoundation -framework AudioToolbox -framework CFNetwork -framework UIKit -framework UIKit -framework AVFoundation -framework CoreGraphics -framework QuartzCore -framework CoreVideo -framework CoreMedia

export OS_SOURCES  := 


