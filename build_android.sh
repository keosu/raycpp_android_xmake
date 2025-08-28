#!/bin/bash

echo "Building for Android..."

# 设置 Android NDK 路径
ANDROID_NDK="C:/Users/Administrator/progs/android-ndk-r27d"

if [ ! -d "$ANDROID_NDK" ]; then
    echo "Error: Android NDK not found at $ANDROID_NDK"
    echo "Please set ANDROID_HOME or ANDROID_NDK environment variable"
    echo "Or install Android NDK and update the path in this script"
    exit 1
fi

echo "Using Android NDK: $ANDROID_NDK"

# 清理之前的构建
echo "Cleaning previous build..."
xmake clean

# 配置 Android 构建
echo "Configuring for Android..."
xmake config -p android -a arm64-v8a -m release --ndk="$ANDROID_NDK"

if [ $? -ne 0 ]; then
    echo "Configuration failed!"
    exit 1
fi

# 构建项目
echo "Building project..."
xmake

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build completed successfully!"
echo "Output: build/android/arm64-v8a/release/libcppray.so"