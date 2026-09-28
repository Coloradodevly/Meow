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
#define LOG_FILE "/storage/emulated/0/Documents/EGL_log.txt"

static void writeLog(const char* msg) {
    LOGI("%s", msg);
    FILE* f = fopen(LOG_FILE, "a");
    if (f) { fprintf(f, "%s\n", msg); fflush(f); fclose(f); }
}

static std::atomic<bool> g_initialized{false};
static EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay, EGLSurface) = nullptr;
static EGLBoolean (*orig_eglSwapBuffersWithDamageKHR)(EGLDisplay, EGLSurface, EGLint*, EGLint) = nullptr;

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

    char buf[64];
    snprintf(buf, sizeof(buf), "[EGL] ImGui initialized! %dx%d", width, height);
    writeLog(buf);
}

// Called when eglSwapBuffers fires
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

// Called when eglSwapBuffersWithDamageKHR fires (Unity on Android 10+)
static EGLBoolean my_eglSwapBuffersWithDamageKHR(
        EGLDisplay display, EGLSurface surface, EGLint* rects, EGLint n_rects) {
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

    return orig_eglSwapBuffersWithDamageKHR(display, surface, rects, n_rects);
}

bool setupEGLHook() {
    writeLog("[EGL] Setting up hook via xdl...");

    void* eglHandle = xdl_open("libEGL.so", XDL_DEFAULT);
    if (!eglHandle) {
        writeLog("[EGL] xdl_open libEGL.so failed!");
        return false;
    }

    // ── Hook eglSwapBuffers ───────────────────────────────────────────────
    void* sym = xdl_sym(eglHandle, "eglSwapBuffers", nullptr);
    if (!sym) {
        writeLog("[EGL] xdl_sym eglSwapBuffers failed!");
    } else {
        char buf[128];
        snprintf(buf, sizeof(buf), "[EGL] eglSwapBuffers at %p", sym);
        writeLog(buf);

        orig_eglSwapBuffers = (EGLBoolean(*)(EGLDisplay, EGLSurface))sym;
        int ret = DobbyHook(sym, (void*)my_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
        snprintf(buf, sizeof(buf), "[EGL] DobbyHook(eglSwapBuffers) = %d", ret);
        writeLog(buf);
    }

    // ── Hook eglSwapBuffersWithDamageKHR (Unity Android 10+) ─────────────
    void* sym2 = xdl_sym(eglHandle, "eglSwapBuffersWithDamageKHR", nullptr);
    if (!sym2) {
        writeLog("[EGL] eglSwapBuffersWithDamageKHR not found (ok)");
    } else {
        char buf[128];
        snprintf(buf, sizeof(buf), "[EGL] eglSwapBuffersWithDamageKHR at %p", sym2);
        writeLog(buf);

        orig_eglSwapBuffersWithDamageKHR =
            (EGLBoolean(*)(EGLDisplay, EGLSurface, EGLint*, EGLint))sym2;
        int ret = DobbyHook(sym2, (void*)my_eglSwapBuffersWithDamageKHR,
            (void**)&orig_eglSwapBuffersWithDamageKHR);
        snprintf(buf, sizeof(buf), "[EGL] DobbyHook(eglSwapBuffersWithDamageKHR) = %d", ret);
        writeLog(buf);
    }

    xdl_close(eglHandle);
    return true;
}

void removeEGLHook() {
    if (g_initialized.load()) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
        g_initialized.store(false);
    }
}
