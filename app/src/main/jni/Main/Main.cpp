#include <android/log.h>
#include <pthread.h>
#include <stdio.h>
#include <chrono>
#include <thread>
#include "../hooks/egl_hook.h"

#define LOG_TAG "ModMenu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOG_FILE "/storage/emulated/0/Documents/modmenu_log.txt"

static void writeLog(const char* msg) {
    LOGI("%s", msg);
    FILE* f = fopen(LOG_FILE, "a");
    if (f) { fprintf(f, "%s\n", msg); fflush(f); fclose(f); }
}

static void* hookThread(void*) {
    writeLog("[ModMenu] Hook thread started");
    writeLog("[ModMenu] Waiting 500ms for Unity GL context...");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    writeLog("[ModMenu] Installing EGL hook...");

    if (setupEGLHook()) {
        writeLog("[ModMenu] EGL hook installed successfully!");
        writeLog("[ModMenu] Waiting for first frame...");
    } else {
        writeLog("[ModMenu] ERROR: EGL hook FAILED!");
    }
    return nullptr;
}

__attribute__((constructor))
void onLoad() {
    writeLog("[ModMenu] ===== libModMenu.so loaded =====");
    writeLog("[ModMenu] Constructor called — spawning hook thread");
    pthread_t t;
    pthread_create(&t, nullptr, hookThread, nullptr);
    pthread_detach(t);
    writeLog("[ModMenu] Hook thread spawned");
}

__attribute__((destructor))
void onUnload() {
    writeLog("[ModMenu] Destructor called — cleaning up");
    removeEGLHook();
    writeLog("[ModMenu] Done.");
}
