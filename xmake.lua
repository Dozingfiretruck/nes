add_rules("mode.debug", "mode.release")

set_project("nes")
set_version("0.0.1")
set_xmakever("2.7.6")

add_requires("llvm")
set_toolchains("@llvm")

if is_mode("debug") then
    set_optimize("none")
    add_defines("__DEBUG__")
    set_symbols("debug")
else 
    set_symbols("hidden")
    set_strip("all")
    set_optimize("fastest")
end

set_warnings("all")
set_languages("c11")

-- TODO
-- if is_host("windows") then
-- elseif is_host("linux") then
-- elseif is_host("macosx") then
-- else 
-- end

-- add SDL2
add_requires("libsdl")
add_packages("libsdl")

-- add_includedirs("SDL2/include")
-- if is_arch("x64") then
-- add_linkdirs("SDL2/lib/x64")
-- elseif is_arch("x86") then
-- add_linkdirs("SDL2/lib/x86")
-- end
-- add_links("SDL2")

target("nes", function ()
    set_kind("binary")

    add_includedirs("inc")
    add_includedirs("port")
    add_files("src/**.c")
    add_files("port/*.c")
    
    add_files("main.c")
    
end)
