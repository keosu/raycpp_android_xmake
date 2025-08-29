function main(target, android_sdk_version, android_manifest, android_res, android_assets, attachedjar, keystore, keystore_pass, apk_output_path)
    local outputpath = path.join("build", "android", "output")
    if os.exists(outputpath) then
        os.rmdir(outputpath)
    end
    local outputtemppath = path.join(outputpath, "temp")
    os.mkdir(outputtemppath)
    local libpath = path.join(outputpath, "lib")
    os.mkdir(libpath)
    local libarchpath = path.join(libpath, target:arch())
    os.mkdir(libarchpath)
    os.cp(path.join(target:installdir(), "lib", "*.so"), libarchpath)
    
    -- 添加libc++_shared.so到APK中
    import("core.tool.toolchain")
    local toolchain_ndk = toolchain.load("ndk", {plat = target:plat(), arch = target:arch()})
    local ndk = path.translate(assert(toolchain_ndk:config("ndk"), "cannot get NDK!"))
    -- 打印调试信息
    print("Target arch: " .. target:arch())
    -- 构造libc++_shared.so的路径
    local libcxx_shared_path = path.join(ndk, "toolchains", "llvm", "prebuilt", "windows-x86_64", "sysroot", "usr", "lib", target:arch(), "libc++_shared.so")
    print("Looking for libc++_shared.so at: " .. libcxx_shared_path)
    if os.exists(libcxx_shared_path) then
        print("Found libc++_shared.so, copying to APK")
        os.cp(libcxx_shared_path, libarchpath)
    else
        print("libc++_shared.so not found at expected location")
        -- 尝试硬编码的路径
        local hardcoded_path = "D:\\Progs\\ndk\\27.0\\toolchains\\llvm\\prebuilt\\windows-x86_64\\sysroot\\usr\\lib\\aarch64-linux-android\\libc++_shared.so"
        print("Trying hardcoded path: " .. hardcoded_path)
        if os.exists(hardcoded_path) then
            print("Found libc++_shared.so at hardcoded path, copying to APK")
            os.cp(hardcoded_path, libarchpath)
        else
            print("libc++_shared.so not found at hardcoded path either")
        end
    end
    
    -- 正确处理目标文件名，确保重命名为libmain.so
    local target_filename = target:filename()
    local source_lib = path.join(libarchpath, target_filename)
    local dest_lib = path.join(libarchpath, "libmain.so")
    if os.exists(source_lib) then
        os.mv(source_lib, dest_lib)
    else
        -- 如果目标文件不存在，尝试其他可能的文件名
        local possible_names = {"libcppray.so", "libmain.so"}
        for _, name in ipairs(possible_names) do
            local possible_lib = path.join(libarchpath, name)
            if os.exists(possible_lib) then
                os.mv(possible_lib, dest_lib)
                break
            end
        end
    end

    import("core.tool.toolchain")

    local toolchain_ndk = toolchain.load("ndk", {plat = target:plat(), arch = target:arch()})

    -- get ndk
    local ndk = path.translate(assert(toolchain_ndk:config("ndk"), "cannot get NDK!"))

    -- get android sdk directory
    local android_sdkdir = path.translate(assert(toolchain_ndk:config("android_sdk"), "please run `xmake f --android_sdk=xxx` to set the android sdk directory!"))

    -- get android build-tools version
    local android_build_toolver = assert(toolchain_ndk:config("build_toolver"), "please run `xmake f --build_toolver=xxx` to set the android build-tools version!")

    local androidjar = path.join(android_sdkdir, "platforms", string.format("android-%s", android_sdk_version), "android.jar")

    local aapt = path.join(android_sdkdir, "build-tools", android_build_toolver, "aapt" .. (is_host("windows") and ".exe" or ""))

    local resonly_apk = path.join(outputtemppath, "res_only.zip")
    local aapt_argv = {"package", "-f",
        "-M", android_manifest,
        "-I", androidjar,
        "-F", resonly_apk,
    }

    if android_res ~= nil and os.exists(android_res) then
        table.insert(aapt_argv, "-S")
        table.insert(aapt_argv, android_res)
    end

    if android_assets ~= nil and os.exists(android_assets) and os.emptydir(android_assets) == false then
        table.insert(aapt_argv, "-A")
        table.insert(aapt_argv, android_assets)
    end

    print("packing resources...")
    os.vrunv(aapt, aapt_argv)

    import("lib.detect.find_tool")
    local zip = find_tool("7z") or find_tool("zip") or find_tool("jar")
    assert(zip, "zip or jar tool not found!")
    local zip_argv = { "a", "-tzip", "-r", 
        resonly_apk,
        path.join(".", libpath)
    }

    print("archiving libs...")
    os.vrunv(zip.program or zip.name, zip_argv)

    local aligned_apk = path.join(outputtemppath, "unsigned.apk")
    local zipalign = path.join(android_sdkdir, "build-tools", android_build_toolver, "zipalign" .. (is_host("windows") and ".exe" or ""))
    local zipalign_argv = {
        "-f", "4",
        resonly_apk, aligned_apk
    }

    print("align apk...")
    os.vrunv(zipalign, zipalign_argv)

    local final_output_path = outputtemppath
    local final_apk = path.join(final_output_path, target:basename() .. ".apk")
    local apksigner = path.join(android_sdkdir, "build-tools", android_build_toolver, "apksigner" .. (is_host("windows") and ".bat" or ""))
    local apksigner_argv = {
        "sign", 
        "--ks", keystore,
        "--ks-pass", string.format("pass:%s", keystore_pass),
        "--out", final_apk, 
        "--in", aligned_apk
    }

    print("signing apk...")
    os.vrunv(apksigner, apksigner_argv)

    if apk_output_path then
        os.mv(final_apk, apk_output_path)
    end
end