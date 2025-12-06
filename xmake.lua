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
    
    -- 复制 assets 目录到安装目录
    after_install(function (target)
        import("core.project.config")
        local installdir = config.get("installdir") or "build"
        
        -- 复制 assets 目录
        if os.isdir("assets") then
            os.cp("assets", path.join(installdir, "assets"))
            print("Copied assets to " .. path.join(installdir, "assets"))
        end
    end)
    
    -- Android 特定配置
    if is_plat("android") then
        -- Android平台添加所有源文件 
        add_files("src/jni_interface.cxx") 
        
        set_kind("shared")
        add_ldflags("-shared")
        add_defines("PLATFORM_ANDROID") 
        add_syslinks("log", "android", "EGL", "GLESv2", "OpenSLES")
         
        on_load(function (target) 
            import("core.tool.toolchain")
            local toolchain_ndk = toolchain.load("ndk", {plat = target:plat(), arch = target:arch()})
            if not toolchain_ndk then
                raise("NDK toolchain not found! Please configure NDK properly.")
            end
            
            local ndk_root = toolchain_ndk:config("ndk")
            if not ndk_root then
                raise("NDK path not set! Please set NDK path properly.")
            end
            
            local native_app_glue_path = path.join(ndk_root, "sources", "android", "native_app_glue")
            
            -- 添加NDK的android_native_app_glue
            target:add("files", path.join(native_app_glue_path, "android_native_app_glue.c"))
            target:add("includedirs", native_app_glue_path)
        end)
        
        -- 添加 androidcpp 规则用于 APK 打包
        add_rules("androidcpp", {
            android_sdk_version = "35",
            android_manifest = "AndroidManifest.xml",
            android_res = "res",
            android_assets = "assets",
            apk_output_path = "build",
            package_name = "com.game.raycpp",
            activity_name = "android.app.NativeActivity"
        })
    end