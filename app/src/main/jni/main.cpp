#include <jni.h>
#include <android/log.h>
#include "hooks/egl_hook.h"

#define LOG_TAG "ModMenu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Called automatically when the .so is loaded into the target process
__attribute__((constructor))
void onLibraryLoad() {
    LOGI("Mod menu .so loaded!");

    // Set up the EGL hook — this is what lets ImGui draw on screen
    if (setupEGLHook()) {
        LOGI("EGL hook installed successfully.");
    } else {
        LOGE("Failed to install EGL hook!");
    }
}

// Called automatically when the .so is unloaded
__attribute__((destructor))
void onLibraryUnload() {
    LOGI("Mod menu .so unloaded.");
    removeEGLHook();
}
