# GitHub 配置文件说明

本目录包含项目的 GitHub 相关配置文件，包括 CI/CD 工作流、Issue 模板和文档。

## 📁 目录结构

```
.github/
├── workflows/              # GitHub Actions 工作流
│   ├── android.yml        # Android 专用构建
│   ├── build.yml          # 多平台构建
│   └── README.md          # 工作流详细说明
├── ISSUE_TEMPLATE/         # Issue 模板
│   ├── bug_report.md      # Bug 报告模板
│   ├── feature_request.md # 功能请求模板
│   └── ci_cd_issue.md     # CI/CD 问题模板
├── pull_request_template.md # PR 模板
├── CI_SETUP_GUIDE.md      # CI/CD 完整设置指南
├── QUICK_REFERENCE.md     # 快速参考卡片
├── BADGES.md              # 构建徽章配置
└── README.md              # 本文件
```

## 🚀 快速开始

### 新用户
如果这是你第一次设置 CI/CD，请阅读：
1. **[CI_SETUP_GUIDE.md](CI_SETUP_GUIDE.md)** - 完整的设置指南

### 日常使用
如果你只需要快速参考命令和流程：
1. **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)** - 快速参考卡片

### 了解工作流
如果你想深入了解工作流配置：
1. **[workflows/README.md](workflows/README.md)** - 工作流详细说明

## 📝 文件说明

### 工作流文件

#### `workflows/android.yml`
- **用途：** Android 平台专用构建
- **触发：** Push, PR, 手动
- **平台：** Android arm64-v8a
- **时间：** ~10-15 分钟
- **产物：** APK + .so 库

#### `workflows/build.yml`
- **用途：** 多平台完整构建
- **触发：** Push, PR, 标签, 手动
- **平台：** Android (2 架构), Windows, Linux, macOS
- **时间：** ~20-30 分钟
- **产物：** 所有平台的可执行文件
- **特性：** 自动发布（标签推送时）

### 文档文件

#### `CI_SETUP_GUIDE.md`
**目标读者：** 项目维护者，首次设置 CI/CD 的开发者

**内容：**
- 详细的设置步骤
- 环境配置说明
- 故障排除指南
- 安全最佳实践
- 进阶配置选项

#### `QUICK_REFERENCE.md`
**目标读者：** 熟悉项目的开发者

**内容：**
- 常用命令速查
- 工作流对比表
- 快速故障排查
- 实用提示

#### `BADGES.md`
**目标读者：** 项目维护者

**内容：**
- 构建状态徽章代码
- 配置说明

#### `workflows/README.md`
**目标读者：** 需要了解工作流细节的开发者

**内容：**
- 工作流触发条件
- 构建矩阵说明
- 环境依赖列表
- 本地构建命令对照

### 模板文件

#### `pull_request_template.md`
创建 PR 时自动显示的模板，包含：
- 更改描述
- 测试平台清单
- 检查清单

#### `ISSUE_TEMPLATE/bug_report.md`
Bug 报告模板，包含：
- 问题描述
- 复现步骤
- 环境信息
- 错误日志

#### `ISSUE_TEMPLATE/feature_request.md`
功能请求模板，包含：
- 功能描述
- 使用场景
- 实现方案建议

#### `ISSUE_TEMPLATE/ci_cd_issue.md`
CI/CD 专用问题模板，包含：
- 工作流信息
- 错误日志
- 环境信息

## 🎯 使用场景导航

### 场景 1：我想第一次设置 CI/CD
1. 阅读 [CI_SETUP_GUIDE.md](CI_SETUP_GUIDE.md)
2. 推送代码到 GitHub
3. 在 Actions 页面查看构建结果

### 场景 2：我想手动触发构建
1. 访问 GitHub Actions 页面
2. 选择工作流
3. 点击 "Run workflow"

或查看 [QUICK_REFERENCE.md](QUICK_REFERENCE.md) 了解 CLI 方法

### 场景 3：我想创建发布版本
```bash
git tag v1.0.0
git push origin v1.0.0
```
详见 [CI_SETUP_GUIDE.md#创建发布版本](CI_SETUP_GUIDE.md#创建发布版本)

### 场景 4：构建失败了
1. 查看 Actions 日志
2. 参考 [QUICK_REFERENCE.md#故障排查](QUICK_REFERENCE.md#故障排查)
3. 如果无法解决，使用 [CI/CD Issue 模板](ISSUE_TEMPLATE/ci_cd_issue.md) 报告问题

### 场景 5：我想添加构建徽章
1. 查看 [BADGES.md](BADGES.md)
2. 替换用户名和仓库名
3. 添加到 README.md

### 场景 6：我想修改工作流
1. 阅读 [workflows/README.md](workflows/README.md) 了解当前配置
2. 编辑 `.github/workflows/*.yml`
3. 推送更改并在 Actions 页面验证

## 🔧 维护指南

### 更新 NDK 版本
在两个工作流文件中修改：
```yaml
"ndk;26.1.10909125"  # 改为新版本
```

### 更新 Java 版本
```yaml
- name: Setup Java
  uses: actions/setup-java@v4
  with:
    java-version: '17'  # 改为需要的版本
```

### 添加新的构建架构
在 `build.yml` 中修改 matrix：
```yaml
strategy:
  matrix:
    arch: [arm64-v8a, armeabi-v7a, x86, x86_64]  # 添加架构
```

### 修改缓存策略
调整缓存路径和 key：
```yaml
- name: Cache xmake packages
  uses: actions/cache@v4
  with:
    path: |
      ~/.xmake/packages
      .xmake
    key: xmake-${{ runner.os }}-${{ hashFiles('**/xmake.lua') }}
```

### 修改产物保留时间
```yaml
retention-days: 30  # 改为需要的天数 (1-90)
```

## 📊 工作流决策树

```
开始
│
├─ 只需要 Android APK？
│  └─ 使用 android.yml（更快）
│
├─ 需要多个平台？
│  └─ 使用 build.yml
│
├─ 需要创建发布？
│  └─ 推送标签（触发 build.yml + 自动发布）
│
└─ 需要测试 PR？
   └─ 两个工作流都会自动运行
```

## 🔍 常见问题

### Q: 为什么有两个工作流文件？
**A:** `android.yml` 专注于快速构建 Android 版本，`build.yml` 提供完整的多平台构建和发布功能。

### Q: 我应该使用哪个工作流？
**A:** 
- 日常开发和测试：`android.yml`（更快）
- 正式发布或需要多平台：`build.yml`
- 大多数情况：两者都会自动运行，选择需要的产物

### Q: 如何只构建某个平台？
**A:** 手动触发工作流时无法选择，但可以：
1. 创建新的工作流文件
2. 或临时修改工作流文件

### Q: 缓存多久会过期？
**A:** 7 天未使用或 `xmake.lua` 文件变更时会失效。

### Q: 私有仓库会消耗多少分钟数？
**A:** 
- android.yml: ~10-15 分钟
- build.yml: ~20-30 分钟（macOS 按 10x 计算）

### Q: 如何禁用某个平台的构建？
**A:** 在 `build.yml` 中注释掉对应的 job：
```yaml
# build-macos:  # 注释掉不需要的平台
#   name: Build macOS
#   ...
```

## 📚 相关资源

- [GitHub Actions 官方文档](https://docs.github.com/actions)
- [xmake 官方文档](https://xmake.io/)
- [Android NDK 文档](https://developer.android.com/ndk)
- [GitHub Actions Cache](https://github.com/actions/cache)
- [GitHub Actions Upload Artifact](https://github.com/actions/upload-artifact)

## 🤝 贡献

如果你发现文档有误或有改进建议，欢迎：
1. 提交 Issue（使用对应的模板）
2. 提交 Pull Request（使用 PR 模板）

## 📄 许可

这些配置文件遵循项目的主许可证。

---

**快速链接：**
- [完整设置指南](CI_SETUP_GUIDE.md)
- [快速参考](QUICK_REFERENCE.md)
- [工作流说明](workflows/README.md)
- [徽章配置](BADGES.md)

