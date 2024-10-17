add_rules("mode.debug", "mode.release")

set_policy("build.warning", true)
set_warnings("all")

if is_mode("debug") then
    set_symbols("debug")
    add_defines("DEBUG")
    set_optimize("none")
end

--if is_mode("release") then
--    set_symbols("debug")
--end

add_requires("nlohmann_json")

target("EasyAutoTracker")
    set_languages("cxx20")
    set_exceptions("cxx")
    add_rules("qt.application")
--    add_rules("qt.console")
    add_headerfiles("src/**.h")
    add_files("src/**.cpp")
    add_frameworks("QtWidgets", "QtGui")
    add_packages("nlohmann_json")
