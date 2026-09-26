#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>
#include <dlfcn.h>
#include "hooks/egl_hook.h"

#define LOG_TAG "ModMenu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static void* hookThread(void*) {
    // Wait until libEGL.so is loaded by Unity
    void* libEGL = nullptr;
    while (!libEGL) {
        libEGL = dlopen("libEGL.so", RTLD_NOW | RTLD_NOLOAD);
        if (!libEGL) {
            LOGI("Waiting for libEGL.so...");
            sleep(1);
        }
    }
    dlclose(libEGL);

    // Give Unity a moment to finish GL init
    sleep(2);

    if (setupEGLHook()) {
        LOGI("EGL hook installed successfully.");
    } else {
        LOGE("Failed to install EGL hook!");
    }
    return nullptr;
}

__attribute__((constructor))
void onLibraryLoad() {
    LOGI("Mod menu .so loaded!");
    pthread_t t;
    pthread_create(&t, nullptr, hookThread, nullptr);
    pthread_detach(t);
}

__attribute__((destructor))
void onLibraryUnload() {
    LOGI("Mod menu .so unloaded.");
    removeEGLHook();
}
