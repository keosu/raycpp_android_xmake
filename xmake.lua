add_rules("mode.debug", "mode.release")

-- 添加本地仓库
add_repositories("local-repo local_repo")

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
    end
 
