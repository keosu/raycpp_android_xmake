add_rules("mode.debug", "mode.release")


includes("xmake_android")


if is_plat("android") then
    set_toolchains("ndk", {sdkver = "21"})
end
 
add_requires("raylib 5.5.0")

target("raydemo_android")
    set_kind("binary")
    set_languages("c++17")
    add_files("src/main.cpp") 
    add_packages("raylib")
    
    if is_plat("android") then  
 
        add_rules("android.native_app", {
            android_sdk_version = "35",
            android_manifest = "android/AndroidManifest.xml",
            android_res = "android/res",
            keystore = "android/debug.jks",
            keystore_pass = "123456",
            jni_interface = "android/jni_interface.cxx",
            android_assets = "assets",
            package_name = "com.game.raygame"
        })
    end

 