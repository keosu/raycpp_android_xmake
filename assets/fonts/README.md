# 字体文件说明

## 📁 需要的字体文件

为了让应用完美支持中文和 Emoji，请下载以下字体并放入此目录：

### 1. 中文字体（必需）

#### 选项 A: Noto Sans SC（推荐）
- **文件名**: `NotoSansSC-Regular.otf`
- **下载地址**: https://fonts.google.com/noto/specimen/Noto+Sans+SC
- **大小**: 约 16 MB
- **特点**: Google 开源字体，支持简体中文，清晰美观

#### 选项 B: 思源黑体
- **文件名**: `SourceHanSansSC-Regular.otf`
- **下载地址**: https://github.com/adobe-fonts/source-han-sans/releases
- **大小**: 约 16 MB
- **特点**: Adobe 开源字体，优秀的屏幕显示效果

### 2. Emoji 字体（可选）

#### 选项 A: Noto Color Emoji
- **文件名**: `NotoColorEmoji.ttf`
- **下载地址**: https://github.com/googlefonts/noto-emoji/releases
- **大小**: 约 10 MB
- **特点**: Google 开源彩色 Emoji

#### 选项 B: Twemoji（Twitter Emoji）
- **文件名**: 需要转换为字体格式
- **下载地址**: https://github.com/twitter/twemoji
- **特点**: Twitter 风格的 Emoji

### 3. 备用方案：使用开源字体

如果希望应用体积更小，可以使用：

- **文泉驿微米黑**: `wqy-microhei.ttc` (约 5 MB)
- 下载: https://wenq.org/wqy2/index.cgi?MicroHei

---

## 📥 快速下载链接

### 推荐组合（总大小约 26 MB）

```bash
# 中文字体 - Noto Sans SC Regular
wget https://github.com/googlefonts/noto-cjk/raw/main/Sans/OTF/SimplifiedChinese/NotoSansSC-Regular.otf

# Emoji 字体 - Noto Color Emoji
wget https://github.com/googlefonts/noto-emoji/raw/main/fonts/NotoColorEmoji.ttf
```

---

## 📂 文件结构

下载后，应该是这样的结构：

```
assets/
└── fonts/
    ├── README.md              (本文件)
    ├── NotoSansSC-Regular.otf (中文字体)
    └── NotoColorEmoji.ttf     (Emoji 字体)
```

---

## ⚖️ 许可证信息

### Noto 字体
- **许可证**: SIL Open Font License 1.1
- **可商用**: ✅ 是
- **可修改**: ✅ 是
- **可分发**: ✅ 是

### 思源字体
- **许可证**: SIL Open Font License 1.1
- **可商用**: ✅ 是

### 文泉驿字体
- **许可证**: GPL v2 with Font Exception
- **可商用**: ✅ 是

---

## 🚀 使用说明

1. 下载字体文件到此目录
2. 重新编译项目
3. 字体将自动打包到 APK 中
4. 应用将优先使用打包的字体

---

## 💡 提示

- 如果只需要英文和基本符号，可以不添加字体（使用系统默认）
- 中文字体必需，否则中文会显示为方块
- Emoji 字体可选，没有则显示纯文字图标
- 字体文件较大，会增加 APK 体积

---

## 🔍 验证字体

编译后，在运行时会看到日志：

```
Loaded font from assets: fonts/NotoSansSC-Regular.otf with 21000 glyphs
Loaded emoji font from assets: fonts/NotoColorEmoji.ttf
```

如果看到这些信息，说明字体加载成功！

