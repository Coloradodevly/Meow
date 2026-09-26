#include "egl_hook.h"

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <dlfcn.h>
#include <android/log.h>

// ImGui headers
#include "imgui.h"
#include "imgui_impl_opengl3.h"

// Our menu drawing function
#include "../menu/menu.h"

// Hooking library — this is And64InlineHook (ARM64)
// Make sure to include it in your jni/ folder
#include "And64InlineHook.hpp"

#define LOG_TAG "ModMenu/EGL"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ─── State ────────────────────────────────────────────────────────────────────

static bool g_initialized  = false;
static bool g_touchDown    = false;
static float g_touchX      = 0.f;
static float g_touchY      = 0.f;

// Pointer to the original eglSwapBuffers so we can call through
static EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay, EGLSurface) = nullptr;

// ─── ImGui init ───────────────────────────────────────────────────────────────

static void initImGui(EGLDisplay display, EGLSurface surface) {
    LOGI("Initializing ImGui...");

    // Query the actual surface size so ImGui knows the screen dimensions
    EGLint width = 0, height = 0;
    eglQuerySurface(display, surface, EGL_WIDTH,  &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);
    LOGI("Surface size: %d x %d", width, height);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.IniFilename = nullptr; // Disable saving imgui.ini to disk

    // Dark theme — feel free to customise
    ImGui::StyleColorsDark();

    // Tweak some style values so it looks nice on a phone screen
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding    = 8.f;
    style.FrameRounding     = 4.f;
    style.ScrollbarRounding = 4.f;
    style.GrabRounding      = 4.f;
    style.WindowPadding     = ImVec2(12.f, 12.f);
    style.ItemSpacing       = ImVec2(8.f, 6.f);
    style.TouchExtraPadding = ImVec2(4.f, 4.f); // easier to tap on mobile

    // Init the OpenGL ES 3 backend
    // "#version 300 es" matches OpenGL ES 3.0 which your Poco M6 Pro supports
    ImGui_ImplOpenGL3_Init("#version 300 es");

    LOGI("ImGui initialized successfully.");
    g_initialized = true;
}

// ─── Touch input ──────────────────────────────────────────────────────────────

// Call this from your JNI touch handler or from a hooked input function.
// action: 0 = DOWN, 1 = UP, 2 = MOVE
extern "C" void handleTouch(float x, float y, int action) {
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(x, y);

    switch (action) {
        case 0: // ACTION_DOWN
            io.MouseDown[0] = true;
            g_touchDown = true;
            break;
        case 1: // ACTION_UP
            io.MouseDown[0] = false;
            g_touchDown = false;
            break;
        case 2: // ACTION_MOVE
            // MousePos already updated above
            break;
        default:
            break;
    }

    g_touchX = x;
    g_touchY = y;
}

// ─── Hooked eglSwapBuffers ────────────────────────────────────────────────────

static EGLBoolean hook_eglSwapBuffers(EGLDisplay display, EGLSurface surface) {
    // One-time init on the first frame
    if (!g_initialized) {
        initImGui(display, surface);
    }

    // ── Begin ImGui frame ──
    ImGui_ImplOpenGL3_NewFrame();

    // Manually tick a fake delta time (~60 fps) since we have no GLFW/SDL
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = 1.0f / 60.0f;

    ImGui::NewFrame();

    // ── Draw our menu ──
    DrawMenu();

    // ── End ImGui frame and render ──
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Call the real eglSwapBuffers to present the frame
    return orig_eglSwapBuffers(display, surface);
}

// ─── Hook install / remove ────────────────────────────────────────────────────

bool setupEGLHook() {
    // Find libEGL.so and get the address of eglSwapBuffers
    void* libEGL = dlopen("libEGL.so", RTLD_NOW);
    if (!libEGL) {
        LOGE("Failed to open libEGL.so: %s", dlerror());
        return false;
    }

    void* eglSwapBuffersAddr = dlsym(libEGL, "eglSwapBuffers");
    if (!eglSwapBuffersAddr) {
        LOGE("Failed to find eglSwapBuffers: %s", dlerror());
        return false;
    }

    LOGI("eglSwapBuffers found at: %p", eglSwapBuffersAddr);

    // Install the inline hook using And64InlineHook
    // orig_eglSwapBuffers will be set to the trampoline so we can call through
    A64HookFunction(
        eglSwapBuffersAddr,
        (void*)hook_eglSwapBuffers,
        (void**)&orig_eglSwapBuffers
    );

    if (!orig_eglSwapBuffers) {
        LOGE("Hook installation failed!");
        return false;
    }

    LOGI("eglSwapBuffers hooked successfully.");
    return true;
}

void removeEGLHook() {
    // And64InlineHook doesn't have a built-in unhook, but you can
    // restore the original bytes manually if needed.
    // For now, just clean up ImGui.
    if (g_initialized) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
        g_initialized = false;
        LOGI("ImGui shut down.");
    }
}
