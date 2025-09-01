# Raylib-CPP Android Support

这个项目添加了对 Android 平台的支持，通过本地仓库扩展了 raylib-cpp 和 raylib 包。

## 前提条件

1. **Android NDK**: 需要安装 Android NDK（已验证 NDK r27d 可用）
2. **Android SDK**: 需要完整的 Android SDK（用于 APK 打包）
   - 包含 build-tools（aapt, zipalign, apksigner 等）
   - 包含 platform（android.jar）
3. **Java 环境**: 需要 Java 开发环境（用于 Android 构建）
   - 支持 Java 8+ 版本
4. **xmake**: 确保安装了最新版本的 xmake
5. **7zip**: APK 打包需要 7zip 或 zip 工具

## 构建步骤

### 方式一：使用 xmake 命令（推荐）

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

### 方式二：使用 VS Code 任务

在 VS Code 中使用 `Ctrl+Shift+P` → "Tasks: Run Task" 选择以下任务：
- "Build Android Library" - 构建共享库
- "Build APK" - 构建完整 APK
- "Install and Run APK" - 安装并运行

## 支持的架构

- `arm64-v8a` (64位 ARM，推荐)
- `armeabi-v7a` (32位 ARM)

## 本地仓库说明

项目包含一个本地仓库 `local_repo/`，扩展了以下包：

### raylib-cpp
- 添加了 Android 平台支持
- 保持与官方版本的兼容性

### raylib
- 添加了 Android 平台支持
- 包含 Android 特定的构建配置

## 使用不同架构构建

```bash
# ARM64 (推荐)
xmake config -p android -a arm64-v8a

# ARM32
xmake config -p android -a armeabi-v7a
```

## 输出文件

### 共享库文件
构建成功后，生成的共享库文件位于：
- `build/android/{架构}/release/libcppray.so`

### APK 文件（完整应用）
使用 `xmake install` 打包后，生成的 APK 文件位于：
- `build/cppray.apk`

## 集成到 Android 项目

1. 将生成的 `.so` 文件复制到 Android 项目的 `app/src/main/jniLibs/{架构}/` 目录
2. 使用提供的 `AndroidManifest.xml` 作为参考配置
3. 确保在 Android 项目中正确加载本地库

## 故障排除

### 常见问题

1. **NDK 路径错误**: 确保正确配置了 NDK 路径
2. **架构不匹配**: 确保目标设备支持所选择的架构
3. **依赖问题**: 确保本地仓库配置正确

### 调试信息

使用以下命令获取详细的构建信息：
```bash
xmake -vD
```

## 注意事项

- 这是一个实验性的 Android 支持实现
- 某些 raylib 功能可能在 Android 上有限制
- 建议在实际设备上测试，而不是仅在模拟器中测试

## APK 打包和部署

### 直接运行 APK
使用 androidcpp 规则，可以直接生成可安装的 APK 文件：

```bash
# 构建并打包 APK
xmake config -p android -a arm64-v8a
xmake
xmake install -o build cppray

# 安装并运行到设备
xmake run cppray
```

### APK 功能特性
- ✅ 自动打包共享库
- ✅ 资源文件管理
- ✅ 自动签名（开发版本）
- ✅ ADB 自动安装和启动
- ✅ 支持 NativeActivity 框架