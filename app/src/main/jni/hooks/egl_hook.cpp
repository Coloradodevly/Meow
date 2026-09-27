#include <stdio.h>
#include "egl_hook.h"
#include "dobby.h"

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <dlfcn.h>
#include <android/log.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "../menu/menu.h"

#define LOG_TAG "ModMenu/EGL"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static bool g_initialized = false;
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
    g_initialized = true;
    LOGI("ImGui initialized %dx%d", width, height);
}

extern "C" void handleTouch(float x, float y, int action) {
    if (!g_initialized) return;
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(x, y);
    io.MouseDown[0] = (action == 0 || action == 2);
}

// Our replacement function
static int frameCount = 0;

static EGLBoolean my_eglSwapBuffers(EGLDisplay display, EGLSurface surface) {
    if (frameCount == 0) {
        FILE* f = fopen("/storage/emulated/0/Android/data/com.MA.Polyfield/egl_called.txt", "w");
        if (f) { fprintf(f, "eglSwapBuffers hooked!\n"); fclose(f); }
    }
    frameCount++;
    if (frameCount == 1) {
        LOGI("my_eglSwapBuffers called for first time!");
    }
    if (frameCount % 300 == 0) {
        LOGI("Still hooking, frame %d", frameCount);
    }

    if (!g_initialized) initImGui(display, surface);

    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = 1.0f / 60.0f;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
    DrawMenu();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return orig_eglSwapBuffers(display, surface);
}

// Simple ARM64 inline hook using raw trampolines
static uint8_t g_backup[16];
static void*   g_hookTarget = nullptr;

bool setupEGLHook() {
    void* sym = DobbySymbolResolver("libEGL.so", "eglSwapBuffers");
    if (!sym) {
        LOGE("DobbySymbolResolver failed");
        return false;
    }
    LOGI("eglSwapBuffers at %p", sym);
    orig_eglSwapBuffers = (EGLBoolean(*)(EGLDisplay, EGLSurface))sym;
    DobbyHook(sym, (void*)my_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
    return true;
}

void removeEGLHook() {
    if (g_hookTarget) {
        long pageSize = sysconf(_SC_PAGESIZE);
        uintptr_t addr = (uintptr_t)g_hookTarget & ~(pageSize - 1);
        mprotect((void*)addr, pageSize * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
        memcpy(g_hookTarget, g_backup, 16);
        __builtin___clear_cache((char*)g_hookTarget, (char*)g_hookTarget + 16);
    }
    if (g_initialized) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
        g_initialized = false;
    }
}
