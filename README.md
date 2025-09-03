# Raylib-CPP Android Support

这个项目演示了使用xmake来构建基于raylib-cpp的项目，android平台编译成apk。 

## 前提条件

1. **Android NDK**: 需要安装 Android NDK（已验证 NDK r27d 可用）
2. **Android SDK**: 需要完整的 Android SDK（用于 APK 打包）
   - 包含 build-tools（aapt, zipalign, apksigner 等）
   - 包含 platform（android.jar）
3. **xmake**: 确保安装了最新版本的 xmake

## 构建步骤
 
```bash
# 配置 Android 构建
xmake config -p android -a arm64-v8a -m release

# 构建项目
xmake

# 打包 APK
xmake install -o build cppray

# 安装并运行 APK
xmake run cppray

# 清理构建文件
xmake clean
```
 
