# Raylib-CPP Android Support

这个项目添加了对 Android 平台的支持，通过本地仓库扩展了 raylib-cpp 和 raylib 包。

## 前提条件

1. **Android NDK**: 需要安装 Android NDK（已验证 NDK r27d 可用）
   - 验证过的路径：`C:\Users\Administrator\progs\android-ndk-r27d\`
2. **环境变量**: 设置 `ANDROID_HOME` 环境变量指向 Android SDK 根目录（可选，脚本中已配置）
3. **xmake**: 确保安装了最新版本的 xmake

## 构建步骤

### 方式一：使用构建脚本（推荐）

```bash
# Windows
.\build_android.bat

# Linux/macOS
./build_android.sh
```

### 方式二：手动构建

1. **配置 Android 构建**:
```bash
xmake config -p android -a arm64-v8a -m release --ndk=/path/to/android-ndk
```

2. **构建项目**:
```bash
xmake
```

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

构建成功后，生成的共享库文件位于：
- `build/android/{架构}/release/libcppray.so`

**验证结果**：
- ✅ NDK r27d (27.3.13750724) 验证可用
- ✅ 成功构建 ARM64 版本：`libcppray.so` (932KB)
- ✅ 构建脚本正常工作

## 集成到 Android 项目

1. 将生成的 `.so` 文件复制到 Android 项目的 `app/src/main/jniLibs/{架构}/` 目录
2. 使用提供的 `AndroidManifest.xml` 作为参考配置
3. 确保在 Android 项目中正确加载本地库

## 故障排除

### 常见问题

1. **NDK 路径错误**: 确保 `ANDROID_HOME` 环境变量正确设置
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