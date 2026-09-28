#include <stdio.h>
#include "egl_hook.h"
#include "../3rdparty/dobby/dobby.h"
#include "xdl.h"

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <atomic>
#include <string>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "../menu/menu.h"

#define LOG_TAG "ModMenu/EGL"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOG_FILE "/storage/emulated/0/Android/data/com.MA.Polyfield/modmenu_log.txt"

static void writeLog(const char* msg) {
    LOGI("%s", msg);
    FILE* f = fopen(LOG_FILE, "a");
    if (f) { fprintf(f, "%s\n", msg); fflush(f); fclose(f); }
}

static std::atomic<bool> g_initialized{false};
static EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay, EGLSurface) = nullptr;

static void initImGui(EGLDisplay display, EGLSurface surface) {
    EGLint width = 0, height = 0;
    eglQuerySurface(display, surface, EGL_WIDTH,  &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding    = 8.f;
    style.FrameRounding     = 4.f;
    style.TouchExtraPadding = ImVec2(4.f, 4.f);

    ImGui_ImplOpenGL3_Init("#version 300 es");
    writeLog("[EGL] ImGui initialized!");
}

static EGLBoolean my_eglSwapBuffers(EGLDisplay display, EGLSurface surface) {
    if (!g_initialized.load()) {
        initImGui(display, surface);
        g_initialized.store(true);
    }

    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = 1.0f / 60.0f;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
    DrawMenu();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return orig_eglSwapBuffers(display, surface);
}

bool setupEGLHook() {
    writeLog("[EGL] Setting up hook via xdl...");

    void* handle = xdl_open("libEGL.so", XDL_DEFAULT);
    if (!handle) {
        writeLog("[EGL] xdl_open failed!");
        return false;
    }

    void* sym = xdl_sym(handle, "eglSwapBuffers", nullptr);
    xdl_close(handle);

    if (!sym) {
        writeLog("[EGL] xdl_sym failed!");
        return false;
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "[EGL] eglSwapBuffers at %p", sym);
    writeLog(msg);

    orig_eglSwapBuffers = (EGLBoolean(*)(EGLDisplay, EGLSurface))sym;
    int ret = DobbyHook(sym, (void*)my_eglSwapBuffers, (void**)&orig_eglSwapBuffers);

    snprintf(msg, sizeof(msg), "[EGL] DobbyHook returned %d", ret);
    writeLog(msg);

    return ret == 0;
}

void removeEGLHook() {
    if (g_initialized.load()) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
        g_initialized.store(false);
    }
}
