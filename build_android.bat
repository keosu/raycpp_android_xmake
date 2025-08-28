@echo off
echo Building for Android...

REM 设置 Android NDK 路径
set ANDROID_NDK=C:\Users\Administrator\progs\android-ndk-r27d

REM 清理之前的构建
echo Cleaning previous build...
xmake clean

REM 配置 Android 构建
echo Configuring for Android...
xmake config -p android -a arm64-v8a -m release --ndk=%ANDROID_NDK%

if %ERRORLEVEL% NEQ 0 (
    echo Configuration failed!
    pause
    exit /b 1
)

REM 构建项目
echo Building project...
xmake

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo Build completed successfully!
echo Output: build\android\arm64-v8a\release\libcppray.so

pause