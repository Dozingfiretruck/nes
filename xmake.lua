set_project("nes")
set_version("0.0.1")
set_xmakever("2.9.2")
add_rules("mode.debug", "mode.release")

if is_mode("debug") then
    set_symbols("debug")
    set_optimize("none")
    add_defines("__DEBUG__")
else
    set_strip("all")
    set_symbols("hidden")
    set_optimize("fastest")
end

set_warnings("allextra")
set_languages("c11")

-- TODO
-- if is_host("windows") then
-- elseif is_host("linux") then
-- elseif is_host("macosx") then
-- else 
-- end

-- [[ add SDL2 ]]
add_requires("libsdl")
add_packages("libsdl")

target("nes", function ()
    set_kind("binary")

    add_includedirs("inc")
    add_includedirs("port")
    add_files("src/**.c")
    add_files("port/*.c")
    
    add_files("main.c")
    
end)
