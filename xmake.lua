add_rules("mode.debug", "mode.release")

if is_mode("debug") then
    set_symbols("debug")
end

target("EasyAutoTracker")
    set_languages("cxx20")
    add_rules("qt.application")
--    add_rules("qt.console")
    add_headerfiles("src/**.h")
    add_files("src/**.cpp")
    add_frameworks("QtWidgets", "QtGui")
    set_warnings("all")

