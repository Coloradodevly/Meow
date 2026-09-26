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
    // Wait for both libunity.so and libEGL.so to be loaded
    void* libUnity = nullptr;
    void* libEGL = nullptr;

    while (!libUnity || !libEGL) {
        libUnity = dlopen("libunity.so", RTLD_NOW | RTLD_NOLOAD);
        libEGL   = dlopen("libEGL.so",   RTLD_NOW | RTLD_NOLOAD);
        if (!libUnity || !libEGL) {
            if (libUnity) dlclose(libUnity);
            if (libEGL)   dlclose(libEGL);
            LOGI("Waiting for Unity+EGL...");
            sleep(1);
        }
    }
    dlclose(libUnity);
    dlclose(libEGL);

    // Wait for Unity to finish GL init
    sleep(3);

    if (setupEGLHook()) {
        LOGI("EGL hook installed!");
    } else {
        LOGE("Hook failed!");
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
