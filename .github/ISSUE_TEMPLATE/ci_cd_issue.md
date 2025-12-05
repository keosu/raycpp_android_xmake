---
name: CI/CD 问题
about: 报告 GitHub Actions 构建相关的问题
title: '[CI] '
labels: ci, bug
assignees: ''
---

## 问题描述
<!-- 清晰描述 CI/CD 遇到的问题 -->

## 工作流信息
**工作流文件：**
- [ ] android.yml
- [ ] build.yml

**触发方式：**
- [ ] Push 到分支
- [ ] Pull Request
- [ ] 标签推送
- [ ] 手动触发

**失败的作业：**
- [ ] build-android (arm64-v8a)
- [ ] build-android (armeabi-v7a)
- [ ] build-windows
- [ ] build-linux
- [ ] build-macos
- [ ] create-release

## 工作流运行链接
<!-- 粘贴失败的工作流运行链接 -->
https://github.com/YOUR_USERNAME/YOUR_REPO/actions/runs/...

## 错误日志
<!-- 从 Actions 日志中复制相关的错误信息 -->
```
粘贴错误日志
```

## 复现步骤
1. Fork 此仓库
2. ...
3. ...

## 尝试过的解决方案
<!-- 列出你已经尝试过的解决方法 -->
- [ ] 重新运行工作流
- [ ] 清除缓存
- [ ] 检查分支是否是最新的
- [ ] 其他：

## 环境信息
**GitHub Runner：**
- [ ] ubuntu-latest
- [ ] windows-latest
- [ ] macos-latest

**相关版本：**
- xmake 版本: [从日志中查找]
- NDK 版本 (如果相关): 26.1.10909125
- Java 版本 (如果相关): 17

## 截图
<!-- 如果适用，添加 Actions 页面的截图 -->

## 本地构建是否成功
- [ ] 是，本地构建正常
- [ ] 否，本地也失败
- [ ] 未在本地测试

## 附加信息
<!-- 任何其他有助于诊断问题的信息 -->

## 建议的修复
<!-- 如果你有修复建议，请描述 -->

