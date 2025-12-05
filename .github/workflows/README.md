# GitHub Actions CI/CD 说明

本项目包含两个主要的 CI/CD 工作流：

## 1. android.yml - Android 构建工作流

专门用于构建 Android APK（arm64-v8a 架构）。

### 触发条件
- 推送到 `main`、`master` 或 `develop` 分支
- 向这些分支创建 Pull Request
- 手动触发（workflow_dispatch）

### 构建产物
- `raycpp-android-arm64-v8a`：包含 APK 文件
- `raycpp-android-lib-arm64-v8a`：包含 .so 共享库

### 环境要求
- Java 17 (Temurin)
- Android SDK
- Android NDK 26.1.10909125
- Android Build Tools 35.0.0
- Android Platform 35
- xmake 最新版本

## 2. build.yml - 多平台构建工作流

构建所有支持的平台（Android、Windows、Linux、macOS）。

### 触发条件
- 推送到 `main`、`master` 或 `develop` 分支
- 推送标签（以 `v` 开头，如 `v1.0.0`）
- 向主分支创建 Pull Request
- 手动触发

### 构建矩阵

#### Android
- **架构**: arm64-v8a, armeabi-v7a
- **产物**: `raycpp-android-{arch}.apk`

#### Windows
- **架构**: x64
- **产物**: Windows 可执行文件

#### Linux
- **架构**: x86_64
- **产物**: Linux 可执行文件

#### macOS
- **架构**: x86_64
- **产物**: macOS 可执行文件

### 自动发布
当推送标签时（如 `v1.0.0`），工作流会自动：
1. 构建所有平台
2. 创建 GitHub Release
3. 上传所有构建产物到 Release

## 使用方法

### 手动触发构建
1. 进入 GitHub 仓库
2. 点击 "Actions" 标签
3. 选择要运行的工作流
4. 点击 "Run workflow"

### 创建发布版本
```bash
# 创建并推送标签
git tag v1.0.0
git push origin v1.0.0
```

这将自动触发完整的多平台构建并创建 Release。

### 下载构建产物
- 在 Actions 页面点击具体的工作流运行
- 滚动到底部的 "Artifacts" 部分
- 下载需要的构建产物

## 本地构建命令参考

### Android
```bash
# 配置
xmake f -p android -a arm64-v8a --ndk=/path/to/ndk -m release

# 构建
xmake build

# 安装到 build 目录
xmake install -o build
```

### Windows
```bash
xmake f -p windows -a x64 -m release
xmake build
xmake install -o build
```

### Linux
```bash
xmake f -p linux -a x86_64 -m release
xmake build
xmake install -o build
```

### macOS
```bash
xmake f -p macosx -a x86_64 -m release
xmake build
xmake install -o build
```

## 保留时间
所有构建产物默认保留 30 天。

## 依赖说明

### Android 构建依赖
- **Java**: OpenJDK 17 (Temurin 发行版)
- **Android SDK**: 通过 android-actions/setup-android@v3 自动安装
- **Android NDK**: 26.1.10909125
- **Build Tools**: 35.0.0
- **Platform**: android-35
- **xmake**: 最新版本

### 其他平台依赖
各平台会自动安装必要的系统依赖（如 Linux 的 OpenGL 库）。

