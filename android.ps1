# Raylib-CPP Android Development Environment Management Script

param(
    [Parameter(Position = 0)]
    [ValidateSet("check", "build", "apk", "run", "clean", "help")]
    [string]$Action = "help"
)

# Environment Configuration
$env:ANDROID_NDK = "C:\Users\Administrator\progs\android-ndk-r27d"
$env:ANDROID_SDK = "C:/Users/Administrator/AppData/Local/Android/Sdk"
$env:JAVA_HOME = "C:/zulu-24"

function Write-Status {
    param([string]$Message, [string]$Status = "INFO")
    $color = switch ($Status) {
        "SUCCESS" { "Green" }
        "ERROR" { "Red" }
        "WARNING" { "Yellow" }
        default { "White" }
    }
    Write-Host "[$Status] $Message" -ForegroundColor $color
}

function Test-Environment {
    Write-Host "Checking Android Development Environment..." -ForegroundColor Cyan
    Write-Host "=================================================="
    
    $errorCount = 0
    
    # Check Android NDK
    Write-Host "[1/7] Checking Android NDK..."
    if (Test-Path $env:ANDROID_NDK) {
        Write-Status "NDK found: $env:ANDROID_NDK" "SUCCESS"
    } else {
        Write-Status "NDK not found: $env:ANDROID_NDK" "ERROR"
        $errorCount++
    }
    
    # Check Android SDK
    Write-Host "`n[2/7] Checking Android SDK..."
    if (Test-Path $env:ANDROID_SDK) {
        Write-Status "SDK found: $env:ANDROID_SDK" "SUCCESS"
        if (Test-Path "$env:ANDROID_SDK\platform-tools\adb.exe") {
            Write-Status "ADB tools found" "SUCCESS"
        } else {
            Write-Status "ADB tools not found" "ERROR"
            $errorCount++
        }
    } else {
        Write-Status "SDK not found: $env:ANDROID_SDK" "ERROR"
        $errorCount++
    }
    
    # Check Build Tools
    Write-Host "`n[3/7] Checking Build Tools..."
    $buildToolsPath = "$env:ANDROID_SDK\build-tools"
    if (Test-Path $buildToolsPath) {
        $versions = Get-ChildItem $buildToolsPath -Directory | Where-Object { $_.Name -match "^\d" }
        if ($versions.Count -gt 0) {
            Write-Status "Build Tools version available: $($versions[-1].Name)" "SUCCESS"
        } else {
            Write-Status "No Build Tools versions found" "ERROR"
            $errorCount++
        }
    } else {
        Write-Status "Build Tools directory not found" "ERROR"
        $errorCount++
    }
    
    # Check xmake
    Write-Host "`n[4/7] Checking xmake..."
    try {
        $xmakeVersion = xmake --version 2>$null
        if ($LASTEXITCODE -eq 0) {
            Write-Status "xmake is available" "SUCCESS"
        } else {
            throw
        }
    } catch {
        Write-Status "xmake not found or not in PATH" "ERROR"
        $errorCount++
    }
    
    # Check compression tools
    Write-Host "`n[5/7] Checking compression tools..."
    try {
        7z > $null 2>&1
        if ($LASTEXITCODE -eq 0 -or $LASTEXITCODE -eq 1) {
            Write-Status "7zip found" "SUCCESS"
        } else {
            throw
        }
    } catch {
        try {
            where.exe zip > $null 2>&1
            if ($LASTEXITCODE -eq 0) {
                Write-Status "zip found" "SUCCESS"
            } else {
                throw
            }
        } catch {
            Write-Status "Neither 7zip nor zip tools found" "ERROR"
            Write-Status "Please install 7zip for APK packaging" "WARNING"
            $errorCount++
        }
    }
    
    # Check Java Environment
    Write-Host "`n[6/7] Checking Java Environment..."
    if (Test-Path "$env:JAVA_HOME\bin\java.exe") {
        Write-Status "Java found: $env:JAVA_HOME" "SUCCESS"
        try {
            & "$env:JAVA_HOME\bin\java.exe" -version 2>$null
            if ($LASTEXITCODE -eq 0) {
                Write-Status "Java version check passed" "SUCCESS"
            } else {
                Write-Status "Java version check failed" "WARNING"
            }
        } catch {
            Write-Status "Java version check failed" "WARNING"
        }
    } else {
        Write-Status "Java not found: $env:JAVA_HOME" "ERROR"
        $errorCount++
    }
    
    # Check project files
    Write-Host "`n[7/7] Checking project files..."
    $requiredFiles = @("xmake.lua", "androidcpp\xmake.lua", "AndroidManifest.xml")
    foreach ($file in $requiredFiles) {
        if (Test-Path $file) {
            Write-Status "$file found" "SUCCESS"
        } else {
            Write-Status "$file not found" "ERROR"
            $errorCount++
        }
    }
    
    # Summary
    Write-Host "`n=================================================="
    if ($errorCount -eq 0) {
        Write-Status "Environment check passed! Ready for Android development" "SUCCESS"
        Write-Host "`nAvailable commands:"
        Write-Host "  .\android.ps1 build   - Build Android shared library"
        Write-Host "  .\android.ps1 apk     - Build complete APK"
        Write-Host "  .\android.ps1 run     - Install and run APK"
    } else {
        Write-Status "Environment check failed, please fix the issues above" "ERROR"
    }
    Write-Host "=================================================="
    
    return ($errorCount -eq 0)
}

function Build-AndroidLibrary {
    Write-Host "Building Android shared library..." -ForegroundColor Cyan
    Write-Host "=================================================="
    
    # 设置 Android 平台配置
    $buildCmd = "xmake f -p android -a arm64-v8a -m release"
    Write-Host "Configuring build: $buildCmd"
    Invoke-Expression $buildCmd
    
    if ($LASTEXITCODE -ne 0) {
        Write-Status "Failed to configure build" "ERROR"
        return $false
    }
    
    # 执行构建
    Write-Host "`nBuilding project..."
    xmake -v
    
    if ($LASTEXITCODE -eq 0) {
        Write-Status "Android shared library built successfully!" "SUCCESS"
        Write-Host "Output location: build/android/arm64-v8a/release" -ForegroundColor White
        return $true
    } else {
        Write-Status "Failed to build Android shared library" "ERROR"
        return $false
    }
}

function Build-APK {
    Write-Host "Building APK file..." -ForegroundColor Cyan
    Write-Host "=================================================="
    
    # 确保已经构建了 Android 共享库
    Write-Host "Checking if Android library is built..."
    if (-not (Test-Path "build/android/arm64-v8a/release/libcppray.so")) {
        Write-Host "Android library not found, building it first..."
        if (-not (Build-AndroidLibrary)) {
            Write-Status "Failed to build Android library, cannot proceed with APK creation" "ERROR"
            return $false
        }
    }
    
    # 清理之前的输出
    if (Test-Path "build/android/output") {
        Remove-Item -Path "build/android/output" -Recurse -Force -ErrorAction SilentlyContinue
    }
    
    if (Test-Path "build/cppray.apk") {
        Remove-Item -Path "build/cppray.apk" -Force -ErrorAction SilentlyContinue
    }
    
    # 使用 xmake 构建 APK
    Write-Host "`nBuilding APK with xmake..."
    xmake install -o build/android/output
    
    if ($LASTEXITCODE -eq 0) {
        # 查找生成的 APK 文件
        $apkPath = "build/cppray.apk"
        
        if (Test-Path $apkPath) {
            Write-Status "APK built successfully!" "SUCCESS"
            Write-Host "APK location: $apkPath" -ForegroundColor White
            
            # 显示 APK 信息
            $apkSize = [math]::Round((Get-Item $apkPath).Length / 1KB, 2)
            Write-Host "APK size: $apkSize KB" -ForegroundColor White
            return $true
        } else {
            Write-Status "APK file not found after build" "ERROR"
            return $false
        }
    } else {
        Write-Status "Failed to build APK" "ERROR"
        return $false
    }
}

function Run-APK {
    Write-Host "Installing and running APK..." -ForegroundColor Cyan
    Write-Host "=================================================="
    
    # 检查是否有连接的设备
    Write-Host "Checking for connected Android devices..."
    $adb = "$env:ANDROID_SDK\platform-tools\adb.exe"
    
    if (-not (Test-Path $adb)) {
        Write-Status "ADB not found at $adb" "ERROR"
        return $false
    }
    
    $devices = & $adb devices 2>$null | Select-String -Pattern "device$" | Measure-Object | Select-Object -ExpandProperty Count
    
    if ($devices -eq 0) {
        Write-Status "No Android devices connected. Please connect a device and enable USB debugging." "ERROR"
        Write-Host "You can check connected devices with: adb devices" -ForegroundColor Yellow
        return $false
    } else {
        Write-Status "$devices Android device(s) connected" "SUCCESS"
    }
    
    # 确保 APK 已构建
    $apkFiles = Get-ChildItem -Path "build" -Recurse -Filter "*.apk" -ErrorAction SilentlyContinue
    
    if ($apkFiles.Count -eq 0) {
        Write-Host "APK not found, building it first..."
        if (-not (Build-APK)) {
            Write-Status "Failed to build APK" "ERROR"
            return $false
        }
        # 重新查找 APK 文件
        $apkFiles = Get-ChildItem -Path "build" -Recurse -Filter "*.apk" -ErrorAction SilentlyContinue
    }
    
    if ($apkFiles.Count -gt 0) {
        $apkPath = $apkFiles[0].FullName
        Write-Host "`nInstalling APK: $apkPath"
        
        # 安装 APK
        & $adb install -r "$apkPath"
        
        if ($LASTEXITCODE -eq 0) {
            Write-Status "APK installed successfully!" "SUCCESS"
            
            # 启动应用
            Write-Host "`nLaunching application..."
            & $adb shell am start -n com.raylib.cppray/android.app.NativeActivity
            
            if ($LASTEXITCODE -eq 0) {
                Write-Status "Application launched successfully!" "SUCCESS"
                Write-Host "You can interact with the app on your Android device." -ForegroundColor White
                return $true
            } else {
                Write-Status "Failed to launch application" "ERROR"
                return $false
            }
        } else {
            Write-Status "Failed to install APK" "ERROR"
            return $false
        }
    } else {
        Write-Status "APK file not found" "ERROR"
        return $false
    }
}

function Clean-Build {
    Write-Host "Cleaning build files..." -ForegroundColor Cyan
    Write-Host "=================================================="
    
    $cleanPaths = @(
        "build",
        ".xmake"
    )
    
    $cleaned = $false
    foreach ($path in $cleanPaths) {
        if (Test-Path $path) {
            Write-Host "Removing $path..."
            Remove-Item -Path $path -Recurse -Force -ErrorAction SilentlyContinue
            $cleaned = $true
        }
    }
    
    # 运行 xmake 清理
    Write-Host "Running xmake clean..."
    xmake clean 2>$null
    
    if ($cleaned) {
        Write-Status "Build files cleaned successfully!" "SUCCESS"
    } else {
        Write-Status "No build files to clean" "INFO"
    }
    
    return $true
}

function Show-Help {
    Write-Host "Raylib-CPP Android Development Environment Management Script" -ForegroundColor Cyan
    Write-Host "=================================================="
    Write-Host "Usage: .\android.ps1 <command>"
    Write-Host ""
    Write-Host "Available commands:"
    Write-Host "  check   - Check development environment configuration"
    Write-Host "  build   - Build Android shared library (.so)"
    Write-Host "  apk     - Build complete APK file"
    Write-Host "  run     - Install and run APK on device"
    Write-Host "  clean   - Clean build files"
    Write-Host "  help    - Show this help information"
    Write-Host ""
    Write-Host "Examples:"
    Write-Host "  .\android.ps1 check      # Check environment"
    Write-Host "  .\android.ps1 apk        # Build APK"
    Write-Host "  .\android.ps1 run        # Run application"
    Write-Host ""
    Write-Host "Environment Configuration:"
    Write-Host "  NDK: $env:ANDROID_NDK"
    Write-Host "  SDK: $env:ANDROID_SDK"
    Write-Host "  Java: $env:JAVA_HOME"
    Write-Host "=================================================="
}

# Main Program Logic
switch ($Action) {
    "check" { 
        Test-Environment 
    }
    "build" { 
        # 先检查环境
        if (Test-Environment) {
            Build-AndroidLibrary 
        } else {
            Write-Status "Environment check failed, please fix issues before building" "ERROR"
        }
    }
    "apk" { 
        # 先检查环境
        if (Test-Environment) {
            Build-APK 
        } else {
            Write-Status "Environment check failed, please fix issues before building APK" "ERROR"
        }
    }
    "run" { 
        # 先检查环境
        if (Test-Environment) {
            Run-APK 
        } else {
            Write-Status "Environment check failed, please fix issues before running APK" "ERROR"
        }
    }
    "clean" { 
        Clean-Build 
    }
    "help" { 
        Show-Help 
    }
    default { 
        Show-Help 
    }
}
