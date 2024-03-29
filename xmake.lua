set_project("os_class")
add_rules("mode.debug", "mode.release")
set_languages("c11")
set_targetdir("bin")

target("rb")
set_kind("static")
add_headerfiles("common/rb/include/rb/**.h")
add_includedirs("common/rb/include")
add_files("common/rb/src/*.c")
target_end()

target("class2")
set_kind("binary")
add_deps("rb")
add_includedirs("common/rb/include")
add_files("class2/class2.c")
target_end()

target("class3")
set_kind("binary")
add_deps("rb")
add_includedirs("common/rb/include")
add_files("class3/main.c")
target_end()

target("class3_note")
set_kind("binary")
add_deps("rb")
add_files("class3/notes.c")
target_end()
