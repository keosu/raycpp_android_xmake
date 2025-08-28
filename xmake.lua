add_rules("mode.debug", "mode.release")

add_requires("raylib-cpp")

target("cppray")
    set_kind("binary")
    set_languages("c++17")
    add_files("src/*.cpp")
    add_packages("raylib-cpp")
 
