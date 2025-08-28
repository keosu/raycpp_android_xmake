add_rules("mode.debug", "mode.release")

-- 添加本地仓库
add_repositories("local-repo local_repo")

-- 包含 androidcpp 规则
includes("androidcpp")

-- 添加 Android NDK 工具链支持
if is_plat("android") then
    set_toolchains("ndk", {sdkver = "21"})
end

add_requires("raylib-cpp")

target("cppray")
    set_kind("binary")
    set_languages("c++17")
    add_files("src/*.cpp")
    add_packages("raylib-cpp")
    
    -- Android 特定配置
    if is_plat("android") then
        set_kind("shared")
        add_ldflags("-shared")
        add_defines("PLATFORM_ANDROID")
        add_syslinks("log", "android", "EGL", "GLESv2", "OpenSLES", "dl", "m", "c++_shared")
        
        -- 添加 androidcpp 规则用于 APK 打包
        add_rules("androidcpp", {
            android_sdk_version = "36",
            android_manifest = "AndroidManifest.xml",
            android_res = "res",
            android_assets = "assets",
            apk_output_path = "build",
            package_name = "com.raylib.cppray",
            activity_name = "android.app.NativeActivity"
        })
    end
 
