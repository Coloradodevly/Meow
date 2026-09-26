cmake_minimum_required(VERSION 3.22)
project(ModMenu)

set(CMAKE_CXX_STANDARD 17)

# ─── Collect ImGui sources ────────────────────────────────────────────────────
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# ─── Build the mod menu shared library ───────────────────────────────────────
add_library(modmenu SHARED
    main.cpp
    hooks/egl_hook.cpp
    menu/menu.cpp
    ${IMGUI_SOURCES}
)

# ─── Include paths ────────────────────────────────────────────────────────────
target_include_directories(modmenu PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    hooks/
    menu/
)

# ─── Compile definitions ──────────────────────────────────────────────────────
target_compile_definitions(modmenu PRIVATE
    IMGUI_IMPL_OPENGL_ES3      # Tell imgui_impl_opengl3 to use GLES3
)

# ─── Compiler flags ───────────────────────────────────────────────────────────
target_compile_options(modmenu PRIVATE
    -O2
    -fvisibility=hidden        # Hide all symbols by default
    -ffunction-sections
    -fdata-sections
)

target_link_options(modmenu PRIVATE
    -Wl,--gc-sections          # Strip unused sections (smaller .so)
    -Wl,--strip-all
)

# ─── Link libraries ───────────────────────────────────────────────────────────
target_link_libraries(modmenu
    android    # ANativeActivity, AInputQueue, etc.
    EGL        # eglSwapBuffers
    GLESv3     # OpenGL ES 3
    log        # __android_log_print
)
