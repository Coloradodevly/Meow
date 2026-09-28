#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <pthread.h>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <string>

#define LOG_TAG "Loader"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOG_FILE "/storage/emulated/0/Documents/loader_log.txt"

static void writeLog(const char* msg) {
    LOGI("%s", msg);
    FILE* f = fopen(LOG_FILE, "a");
    if (f) { fprintf(f, "%s\n", msg); fflush(f); fclose(f); }
}

static std::string getNativeLibDir() {
    char buf[512];
    FILE* maps = fopen("/proc/self/maps", "r");
    if (!maps) return "";
    while (fgets(buf, sizeof(buf), maps)) {
        if (strstr(buf, "libLoader.so")) {
            fclose(maps);
            std::string line(buf);
            size_t pos = line.rfind('/');
            if (pos != std::string::npos)
                return line.substr(line.find('/'), pos - line.find('/') + 1);
        }
    }
    fclose(maps);
    return "";
}

static void* loaderThread(void*) {
    writeLog("[Loader] Thread started");

    // Wait for libunity.so and libEGL.so
    void* libUnity = nullptr;
    void* libEGL = nullptr;
    int attempts = 0;

    while (!libUnity || !libEGL) {
        libUnity = dlopen("libunity.so", RTLD_NOW | RTLD_NOLOAD);
        libEGL   = dlopen("libEGL.so",   RTLD_NOW | RTLD_NOLOAD);
        char msg[128];
        snprintf(msg, sizeof(msg), "[Loader] Attempt %d: unity=%p egl=%p", attempts++, libUnity, libEGL);
        writeLog(msg);
        if (!libUnity || !libEGL) {
            if (libUnity) dlclose(libUnity);
            if (libEGL)   dlclose(libEGL);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    dlclose(libUnity);
    dlclose(libEGL);

    writeLog("[Loader] Both libs ready, loading ModMenu...");

    std::string dir = getNativeLibDir();
    if (dir.empty()) {
        writeLog("[Loader] ERROR: Could not find native lib dir");
        return nullptr;
    }

    std::string path = dir + "libModMenu.so";
    char msg[256];
    snprintf(msg, sizeof(msg), "[Loader] Loading: %s", path.c_str());
    writeLog(msg);

    void* handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        snprintf(msg, sizeof(msg), "[Loader] dlopen failed: %s", dlerror());
        writeLog(msg);
        return nullptr;
    }

    writeLog("[Loader] libModMenu.so loaded successfully!");
    return nullptr;
}

jint JNI_OnLoad(JavaVM* vm, void*) {
    writeLog("[Loader] JNI_OnLoad called");
    pthread_t t;
    pthread_create(&t, nullptr, loaderThread, nullptr);
    pthread_detach(t);
    return JNI_VERSION_1_6;
}
