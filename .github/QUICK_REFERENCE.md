# CI/CD 快速参考

## 🚀 快速开始

```bash
# 1. 推送代码
git push origin main

# 2. 创建发布
git tag v1.0.0
git push origin v1.0.0

# 3. 访问 Actions 页面查看构建
```

## 📦 构建产物下载位置

**Actions 页面** → **工作流运行** → **底部 Artifacts**

## 🔧 本地构建命令

### Android (arm64-v8a)
```bash
xmake f -p android -a arm64-v8a -m release
xmake build
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

## 🎯 触发构建的方式

| 操作 | 触发的工作流 |
|------|--------------|
| `git push` 到 main/master/develop | android.yml, build.yml |
| 创建 Pull Request | android.yml, build.yml |
| `git push` 标签 (v*) | build.yml (+ 自动发布) |
| GitHub Actions 页面手动触发 | 任意工作流 |

## 📊 工作流对比

| 特性 | android.yml | build.yml |
|------|-------------|-----------|
| **Android** | ✅ arm64-v8a | ✅ arm64-v8a |
| **Windows** | ❌ | ✅ x64 |
| **Linux** | ❌ | ✅ x86_64 |
| **macOS** | ❌ | ✅ x86_64 |
| **自动发布** | ❌ | ✅ (标签推送时) |
| **构建时间** | ~10-15 分钟 | ~20-30 分钟 |

## 🏷️ 版本标签规范

```bash
v1.0.0    # 主版本
v1.1.0    # 新功能
v1.1.1    # Bug 修复
v2.0.0-beta.1  # 测试版本
```

## 🛠️ 手动触发构建

### 方法 1: GitHub 网页
1. Actions → 选择工作流 → Run workflow

### 方法 2: GitHub CLI
```bash
gh workflow run android.yml
gh workflow run build.yml
```

## 📂 构建产物说明

### Android
- `raycpp-android-arm64-v8a/` → APK 文件
- `raycpp-android-lib-arm64-v8a/` → .so 库文件

### 其他平台
- `raycpp-windows-x64/` → .exe 可执行文件
- `raycpp-linux-x64/` → Linux 可执行文件
- `raycpp-macos-x64/` → macOS 可执行文件

## 🔍 故障排查

### 构建失败？
1. 查看 Actions 日志
2. 检查最近的代码更改
3. 重新运行工作流
4. 清除缓存（Settings → Actions → Caches）

### NDK 错误？
```yaml
# 修改 NDK 版本
"ndk;26.1.10909125"  # 当前版本
"ndk;25.2.9519653"   # 备选版本
```

## ⚡ 性能优化

### 已启用的缓存
- ✅ Android NDK
- ✅ xmake 包
- ✅ 依赖库

### 预计节省时间
- 首次构建：20-30 分钟
- 缓存后：10-15 分钟

## 📝 常用命令

```bash
# 查看所有标签
git tag -l

# 删除本地标签
git tag -d v1.0.0

# 删除远程标签
git push origin :refs/tags/v1.0.0

# 清理本地构建
xmake clean

# 完全清理 xmake 缓存
xmake clean -a
```

## 🎨 添加构建徽章

```markdown
[![Android CI](https://github.com/USERNAME/REPO/actions/workflows/android.yml/badge.svg)](https://github.com/USERNAME/REPO/actions/workflows/android.yml)
```

## 💡 提示

- 🔄 首次构建需要下载依赖，会比较慢
- 💾 缓存会加速后续构建
- 📦 构建产物默认保留 30 天
- 🔒 私有仓库有构建分钟数限制
- 🌟 公开仓库构建分钟数无限制

## 📚 更多信息

- 详细设置指南：[CI_SETUP_GUIDE.md](CI_SETUP_GUIDE.md)
- 工作流说明：[workflows/README.md](workflows/README.md)
- 徽章配置：[BADGES.md](BADGES.md)

