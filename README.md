# Raylib-CPP Android Support

This project demonstrates using xmake to build raylib-cpp based projects and compile them for Android platform as APK files.


## 1 Prerequisites

1. **Android NDK**: Android NDK installation required (NDK r27d verified)
2. **Android SDK**: Full Android SDK required (for APK packaging)
   - Includes build-tools (aapt, zipalign, apksigner, etc.)
   - Includes platform (android.jar)
3. **Java Development Kit**: JDK 17 or higher required for Android tools
4. **xmake**: Ensure latest version of xmake is installed

**Automated Android Environment Setup**

This project provides an automated script to set up the Android development environment:

### 1.1 If you already have Android tools installed:

You can directly configure xmake with your existing Android tools:

```bash
# Configure xmake with your existing paths
xmake g --ndk=[YOUR_NDK_PATH]
xmake g --android_sdk=[YOUR_ANDROID_SDK_PATH]

# Then proceed with normal build process
xmake config -p android -a arm64-v8a -m release
xmake
xmake install -o build cppray
xmake run cppray
```

### 1.2 If you don't have Android tools installed:

The project includes an automated setup script that will download and install all required components:

```bash
# Run the automated setup script
xmake lua setup_android.lua

# The script will:
# 1. Check for Java environment, install if missing (using system package managers)
# 2. Download and install Android NDK r21e
# 3. Download and install Android SDK Command-line Tools
# 4. Install required SDK components (platforms;android-35, build-tools;35.0.0, platform-tools)
# 5. Configure xmake global settings automatically
```

After running the setup script, you can proceed with the normal build process as described below.

## 2 Build Steps

```bash
# Configure Android build
xmake config -p android -a arm64-v8a -m release

# Build project
xmake

# Package APK
xmake install -o build cppray

# Install and run APK  # uses adb
xmake run cppray
```

## 3 CI/CD Auto Building

This project includes GitHub Actions workflows that automatically build multi-platform versions:

- **Android**: arm64-v8a, armeabi-v7a
- **Windows**: x64
- **Linux**: x86_64
- **macOS**: x86_64

### View Build Results

Visit the repository's [Actions](../../actions) page to check build status and download build artifacts.

### Auto Release

Pushing version tags (e.g. `v1.0.0`) will automatically trigger the release process:

```bash
git tag v1.0.0
git push origin v1.0.0
```

See [CI/CD Documentation](.github/workflows/README.md) for details.