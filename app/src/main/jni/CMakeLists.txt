cmake_minimum_required(VERSION 3.22.1)
project("meow")

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui)

# ── Dobby static lib ──────────────────────────────────────────────────────────
add_library(dobby STATIC IMPORTED)
set_target_properties(dobby PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/3rdparty/dobby/arm64-v8a/libdobby.a)

# ── libLoader (lean) ──────────────────────────────────────────────────────────
add_library(Loader SHARED
    Loader/Loader.cpp
)
target_include_directories(Loader PRIVATE
    ${CMAKE_SOURCE_DIR}/hooks
)
target_link_libraries(Loader
    log
    dl
)

# ── libModMenu (full stack) ───────────────────────────────────────────────────
add_library(ModMenu SHARED
    Main/Main.cpp
    hooks/egl_hook.cpp
    hooks/xdl.c
    hooks/xdl_iterate.c
    hooks/xdl_linker.c
    hooks/xdl_util.c
    hooks/xdl_lzma.c
    menu/menu.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(ModMenu PRIVATE
    ${CMAKE_SOURCE_DIR}/hooks
    ${CMAKE_SOURCE_DIR}/menu
    ${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/imgui/backends
    ${CMAKE_SOURCE_DIR}/3rdparty/dobby
)
target_compile_definitions(ModMenu PRIVATE
    IMGUI_IMPL_OPENGL_ES3
    modmenu_EXPORTS
)
target_link_libraries(ModMenu
    dobby
    android
    EGL
    GLESv3
    log
    dl
)
