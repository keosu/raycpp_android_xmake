add_rules("mode.debug", "mode.release")

-- 包含 androidcpp 规则
includes("xmake_android")

-- 添加 Android NDK 工具链支持
if is_plat("android") then
    set_toolchains("ndk", {sdkver = "21"})
end

add_requires("raylib-cpp 5.5.0")

target("cppray")
    set_kind("binary")
    set_languages("c++17")
    add_files("src/*.cpp") 
    add_packages("raylib-cpp")
    
    -- Android 特定配置
    if is_plat("android") then 
        
        set_kind("shared") 
        add_defines("PLATFORM_ANDROID") -- raylib need this for android
 
        add_rules("android.cpp", {
            android_sdk_version = "35",
            android_manifest = "android/AndroidManifest.xml",
            android_res = "android/res",
            keystore = "android/debug.jks",
            jni_interface = "android/jni_interface.cxx",
            android_assets = "assets",
            apk_output_path = "build",
            package_name = "com.game.raygame",
            activity_name = "android.app.NativeActivity"
        })
    end