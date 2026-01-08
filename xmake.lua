set_xmakever("3.0.6")
add_rules("mode.debug", "mode.release")


-- includes("xmake_android")
 
add_requires("raylib 5.5.0")

target("raydemo_android")
    set_kind("binary")
    set_languages("c++17")
    add_files("src/main.cpp") 
    add_packages("raylib")
    
    add_rules("android.native_app", {
        android_sdk_version = "35",
        android_manifest = "android/AndroidManifest.xml",
        android_res = "android/res",
        keystore = "android/debug.jks",
        keystore_pass = "123456",
        package_name = "com.raylib.demo"
    })

 