#include <android/log.h>
#include <pthread.h>
#include <dlfcn.h>
#include <chrono>
#include <thread>
#include <stdio.h>
#include "../hooks/egl_hook.h"

#define LOG_TAG "ModMenu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOG_FILE "/storage/emulated/0/Android/data/com.MA.Polyfield/modmenu_log.txt"

static void writeLog(const char* msg) {
    LOGI("%s", msg);
    FILE* f = fopen(LOG_FILE, "a");
    if (f) { fprintf(f, "%s\n", msg); fflush(f); fclose(f); }
}

static void* hookThread(void*) {
    writeLog("[ModMenu] Hook thread started");

    // Small delay to let EGL context fully initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    writeLog("[ModMenu] Installing EGL hook...");
    if (setupEGLHook()) {
        writeLog("[ModMenu] EGL hook installed!");
    } else {
        writeLog("[ModMenu] EGL hook FAILED!");
    }
    return nullptr;
}

__attribute__((constructor))
void onLoad() {
    writeLog("[ModMenu] Constructor called!");
    pthread_t t;
    pthread_create(&t, nullptr, hookThread, nullptr);
    pthread_detach(t);
}

__attribute__((destructor))
void onUnload() {
    writeLog("[ModMenu] Destructor called");
    removeEGLHook();
}
