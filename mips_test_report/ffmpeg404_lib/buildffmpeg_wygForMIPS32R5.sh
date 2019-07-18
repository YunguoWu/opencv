#!/bin/bash

TOOLCHAIN=/opt/imgtec/Toolchains/mips-mti-linux-gnu/2018.09-03/bin/mips-mti-linux-gnu-
SYSROOT=/opt/imgtec/Toolchains/mips-mti-linux-gnu/2018.09-03/sysroot/

#TOOLCHAIN=/home/wyg/wyg_backup/eliseBorad/EliseByDinu/buildroot-v0.4.0/buildroot/output/host/usr/bin/mips-mti-linux-gnu-
##SYSROOT=/home/wyg/wyg_backup/eliseBorad/EliseByDinu/buildroot-v0.4.0/buildroot/output/staging
#SYSROOT=/home/wyg/wyg_backup/eliseBorad/EliseByDinu/buildroot-v0.4.0/buildroot/output/host/usr/mipsel-buildroot-linux-gnu/sysroot/

INSTALL_DIR=/home/wyg/wyg_backup/ffmpeg/ffmpeg-4.0.4/wyg_build/
PKG_CONFIG_HOST_BINARY=/usr/bin/pkg-config
CPU=mips

./configure \
    --enable-cross-compile \
	--enable-avfilter \
	--disable-version3 \
	--enable-logging \
	--enable-optimizations \
	--disable-extra-warnings \
	--disable-avdevice \
	--enable-avcodec \
	--enable-avformat \
	--enable-network \
	--disable-gray \
	--enable-swscale-alpha \
	--disable-small \
	--enable-dct \
	--enable-fft \
	--enable-mdct \
	--enable-rdft \
	--disable-crystalhd \
	--disable-vdpau \
	--disable-dxva2 \
	--enable-runtime-cpudetect \
	--disable-hardcoded-tables \
	--disable-mipsdspr2 \
	--enable-msa \
	--enable-hwaccels \
	--disable-avisynth \
	--disable-frei0r \
	--disable-libopencore-amrnb \
	--disable-libopencore-amrwb \
	--disable-libopencv \
	--disable-libcdio \
	--disable-libdc1394 \
	--disable-libgsm \
	--disable-libilbc \
	--disable-libopenjpeg \
	--disable-symver \
	--disable-doc \
    --enable-ffmpeg \
    --disable-avresample \
    --enable-ffprobe \
    --disable-postproc \
    --enable-swscale \
    --enable-pthreads \
    --disable-zlib \
    --disable-bzlib \
    --enable-mipsfpu \
    --enable-pic \
    --cross-prefix=$TOOLCHAIN \
    --sysroot=$SYSROOT \
    --prefix=$INSTALL_DIR \
    --arch=mips32r5 \
    --cpu=p5600     \
    --target-os="linux" \
    --disable-stripping \
    --enable-static --enable-shared \
    --extra-cflags="-EL -mnan=2008" \
    --extra-cxxflags="-EL -mnan=2008" \
    --extra-ldflags="-EL -mnan=2008" \
    --pkg-config="$PKG_CONFIG_HOST_BINARY"
