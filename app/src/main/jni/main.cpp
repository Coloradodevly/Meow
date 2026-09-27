#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdio.h>
#include "hooks/egl_hook.h"

#define LOG_TAG "ModMenu"
#define LOG_FILE "/data/data/com.MA.Polyfield/modmenu_log.txt"

static FILE* logFile = nullptr;

static void writeLog(const char* level, const char* msg) {
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s: %s", level, msg);
    if (logFile) {
        fprintf(logFile, "[%s] %s\n", level, msg);
        fflush(logFile);
    }
}

#define LOG(fmt, ...) do { \
    char _buf[512]; \
    snprintf(_buf, sizeof(_buf), fmt, ##__VA_ARGS__); \
    writeLog("INFO", _buf); \
} while(0)

#define LOGERR(fmt, ...) do { \
    char _buf[512]; \
    snprintf(_buf, sizeof(_buf), fmt, ##__VA_ARGS__); \
    writeLog("ERROR", _buf); \
} while(0)

static void* hookThread(void*) {
    LOG("Hook thread started");

    void* libUnity = nullptr;
    void* libEGL = nullptr;
    int attempts = 0;

    while (!libUnity || !libEGL) {
        libUnity = dlopen("libunity.so", RTLD_NOW | RTLD_NOLOAD);
        libEGL   = dlopen("libEGL.so",   RTLD_NOW | RTLD_NOLOAD);
        LOG("Attempt %d: libunity=%p libEGL=%p", attempts++, libUnity, libEGL);
        if (!libUnity || !libEGL) {
            if (libUnity) dlclose(libUnity);
            if (libEGL)   dlclose(libEGL);
            sleep(1);
        }
    }
    dlclose(libUnity);
    dlclose(libEGL);

    LOG("Both libs loaded! Waiting 3s...");
    sleep(3);

    LOG("Calling setupEGLHook...");
    if (setupEGLHook()) {
        LOG("EGL hook installed successfully!");
    } else {
        LOGERR("EGL hook FAILED!");
    }
    return nullptr;
}

__attribute__((constructor))
void onLibraryLoad() {
    logFile = fopen(LOG_FILE, "w");
    LOG("=== Mod menu .so loaded! ===");
    pthread_t t;
    pthread_create(&t, nullptr, hookThread, nullptr);
    pthread_detach(t);
}

__attribute__((destructor))
void onLibraryUnload() {
    LOG("Mod menu .so unloaded.");
    if (logFile) fclose(logFile);
    removeEGLHook();
}
