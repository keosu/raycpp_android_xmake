# GitHub Actions CI/CD 设置指南

## 概述

本指南将帮助你在 GitHub 上设置自动化构建和发布流程。

## 前置条件

1. 将代码推送到 GitHub 仓库
2. 确保仓库中包含以下文件：
   - `.github/workflows/android.yml` - Android 专用构建
   - `.github/workflows/build.yml` - 多平台构建

## 设置步骤

### 1. 推送代码到 GitHub

```bash
# 如果还没有初始化 Git 仓库
git init
git add .
git commit -m "Initial commit with CI/CD workflows"

# 添加远程仓库（替换为你的仓库 URL）
git remote add origin https://github.com/YOUR_USERNAME/YOUR_REPO.git

# 推送到 GitHub
git branch -M main
git push -u origin main
```

### 2. 启用 GitHub Actions

1. 进入你的 GitHub 仓库页面
2. 点击 "Actions" 标签
3. 如果看到提示，点击 "I understand my workflows, go ahead and enable them"

### 3. 验证工作流

推送代码后，GitHub Actions 会自动开始构建：

1. 访问 "Actions" 标签
2. 查看正在运行的工作流
3. 点击工作流名称查看详细日志

### 4. 下载构建产物

构建完成后：

1. 在 Actions 页面点击具体的工作流运行
2. 滚动到底部找到 "Artifacts" 部分
3. 点击下载需要的文件（如 APK）

## 工作流说明

### android.yml - Android 专用构建

**触发条件：**
- 推送到 main/master/develop 分支
- 创建 Pull Request
- 手动触发

**构建架构：**
- arm64-v8a

**构建时间：** 约 10-15 分钟（首次构建可能更长）

**产物：**
- APK 文件
- 共享库文件 (.so)

### build.yml - 多平台构建

**触发条件：**
- 推送到 main/master/develop 分支
- 推送版本标签 (v*)
- 创建 Pull Request
- 手动触发

**构建平台：**
- Android (arm64-v8a, armeabi-v7a)
- Windows (x64)
- Linux (x86_64)
- macOS (x86_64)

**构建时间：** 约 20-30 分钟（首次构建可能更长）

## 创建发布版本

### 自动发布流程

1. 确保所有更改已提交
2. 创建版本标签：
   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```
3. GitHub Actions 会自动：
   - 构建所有平台
   - 创建 GitHub Release
   - 上传所有构建产物

### 版本号规范

建议使用语义化版本：
- `v1.0.0` - 主版本
- `v1.1.0` - 次版本（新功能）
- `v1.1.1` - 修订版本（Bug 修复）

### 查看发布

1. 访问仓库的 "Releases" 页面
2. 查看自动生成的发布说明
3. 下载对应平台的构建产物

## 手动触发构建

### 使用 GitHub 网页界面

1. 访问 "Actions" 标签
2. 选择要运行的工作流（左侧列表）
3. 点击 "Run workflow" 按钮
4. 选择分支
5. 点击绿色的 "Run workflow" 按钮

### 使用 GitHub CLI

```bash
# 安装 GitHub CLI
# Windows: winget install GitHub.cli
# macOS: brew install gh
# Linux: 见 https://cli.github.com/

# 登录
gh auth login

# 触发 Android 构建
gh workflow run android.yml

# 触发多平台构建
gh workflow run build.yml
```

## 缓存优化

工作流已配置缓存以加速构建：

### Android NDK 缓存
- 缓存 NDK 安装文件
- 节省 2-3 分钟下载时间

### xmake 包缓存
- 缓存已下载的依赖包
- 节省 5-10 分钟编译时间

### 缓存失效
缓存会在以下情况失效：
- `xmake.lua` 文件发生变化
- 手动清除缓存

## 故障排除

### 构建失败

1. **查看日志：** 点击失败的工作流，查看详细错误信息
2. **常见问题：**
   - NDK 版本不兼容：检查 `xmake.lua` 中的 SDK 版本要求
   - 依赖下载失败：重新运行工作流
   - 磁盘空间不足：简化构建或清理缓存

### NDK 相关错误

如果遇到 NDK 相关问题：
```yaml
# 在 .github/workflows/*.yml 中修改 NDK 版本
echo "y" | sdkmanager --install "ndk;25.2.9519653"
```

### xmake 配置错误

检查工作流中的配置命令：
```bash
# Android
xmake f -p android -a arm64-v8a --ndk=$ANDROID_NDK_HOME -m release -y

# Windows
xmake f -p windows -a x64 -m release -y

# Linux
xmake f -p linux -a x86_64 -m release -y

# macOS
xmake f -p macosx -a x86_64 -m release -y
```

### 清除缓存

如果缓存导致问题：

1. 访问仓库 "Settings" > "Actions" > "Caches"
2. 删除相关缓存
3. 重新运行工作流

## 配置构建徽章

在 README.md 顶部添加构建状态徽章：

```markdown
[![Android CI](https://github.com/YOUR_USERNAME/YOUR_REPO/actions/workflows/android.yml/badge.svg)](https://github.com/YOUR_USERNAME/YOUR_REPO/actions/workflows/android.yml)
[![Multi-Platform Build](https://github.com/YOUR_USERNAME/YOUR_REPO/actions/workflows/build.yml/badge.svg)](https://github.com/YOUR_USERNAME/YOUR_REPO/actions/workflows/build.yml)
```

替换 `YOUR_USERNAME` 和 `YOUR_REPO` 为实际值。

## 构建产物保留时间

- **默认保留期：** 30 天
- **修改方法：** 在工作流文件中修改 `retention-days` 参数

```yaml
- name: Upload APK
  uses: actions/upload-artifact@v4
  with:
    name: raycpp-android-arm64-v8a
    path: build/*.apk
    retention-days: 90  # 修改为 90 天
```

## 进阶配置

### 添加测试步骤

在构建后添加测试：

```yaml
- name: Run tests
  run: |
    xmake test
```

### 配置通知

接收构建失败通知：

1. 访问仓库 "Settings" > "Notifications"
2. 配置 GitHub Actions 通知
3. 或使用 Slack/Discord webhook

### 自定义构建矩阵

修改 `build.yml` 中的架构矩阵：

```yaml
strategy:
  matrix:
    arch: [arm64-v8a, armeabi-v7a, x86, x86_64]  # 添加更多架构
```

## 资源限制

GitHub Actions 免费额度：
- **公开仓库：** 无限制
- **私有仓库：** 
  - 免费账户：2000 分钟/月
  - Pro 账户：3000 分钟/月

**注意：** macOS 构建消耗倍率为 10x

## 安全建议

1. **不要在工作流中硬编码敏感信息**
2. **使用 GitHub Secrets 存储密钥**
3. **发布签名的 APK 时使用密钥库**

配置签名密钥：

```yaml
- name: Sign APK
  env:
    KEYSTORE_PASSWORD: ${{ secrets.KEYSTORE_PASSWORD }}
    KEY_ALIAS: ${{ secrets.KEY_ALIAS }}
    KEY_PASSWORD: ${{ secrets.KEY_PASSWORD }}
  run: |
    # 签名命令
```

## 获取帮助

- [GitHub Actions 文档](https://docs.github.com/actions)
- [xmake 文档](https://xmake.io/#/guide/introduction)
- [项目 Issues](../../issues)

## 总结

完成以上步骤后，你的项目将拥有：

✅ 自动化多平台构建  
✅ 自动化发布流程  
✅ 构建产物管理  
✅ 版本控制  
✅ 缓存优化  

每次推送代码或创建标签时，GitHub Actions 会自动处理构建和发布！

