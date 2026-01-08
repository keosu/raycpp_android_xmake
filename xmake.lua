set_xmakever("3.0.6")

add_rules("mode.debug", "mode.release")


if is_plat("android") then
    set_toolchains("ndk", {sdkver = "21"})
end

add_requires("raylib-cpp 5.5.0")

target("cppray")
    set_kind("binary")
    set_languages("c++17")
    add_files("src/*.cpp") 
    add_packages("raylib-cpp")
    
    if is_plat("android") then
        add_rules("android.native_app", {
            android_sdk_version = "35",
            android_manifest = "android/AndroidManifest.xml",
            android_res = "android/res",
            android_assets = "assets",
            keystore = "android/debug.jks",
            keystore_pass = "123456",
            package_name = "com.game.raygame",
        })
    end
 