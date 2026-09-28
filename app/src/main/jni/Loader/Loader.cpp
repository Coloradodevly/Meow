#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <pthread.h>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <string>

#define LOG_TAG "Loader"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOG_FILE "/storage/emulated/0/Documents/loader_log.txt"

static JavaVM* g_vm = nullptr;

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
            size_t slash = line.find('/');
            size_t last  = line.rfind('/');
            if (slash != std::string::npos && last != std::string::npos)
                return line.substr(slash, last - slash + 1);
        }
    }
    fclose(maps);
    return "";
}

// Safe toast using ActivityThread.currentApplication() — never crashes
static void showToastSafe(const char* msg) {
    if (!g_vm) return;
    JNIEnv* env = nullptr;
    bool attached = false;

    jint res = g_vm->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (res == JNI_EDETACHED) {
        if (g_vm->AttachCurrentThread(&env, nullptr) != JNI_OK) return;
        attached = true;
    } else if (res != JNI_OK) return;

    jclass atClass = env->FindClass("android/app/ActivityThread");
    if (!atClass) goto detach;
    {
        jmethodID currentApp = env->GetStaticMethodID(atClass,
            "currentApplication", "()Landroid/app/Application;");
        if (!currentApp) goto detach;

        jobject context = env->CallStaticObjectMethod(atClass, currentApp);
        if (!context || env->ExceptionCheck()) { env->ExceptionClear(); goto detach; }

        jclass toastClass = env->FindClass("android/widget/Toast");
        if (!toastClass) goto detach;

        jmethodID makeText = env->GetStaticMethodID(toastClass, "makeText",
            "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;");
        if (!makeText) goto detach;

        jstring jmsg = env->NewStringUTF(msg);
        jobject toast = env->CallStaticObjectMethod(toastClass, makeText, context, jmsg, (jint)0);
        if (!toast || env->ExceptionCheck()) { env->ExceptionClear(); goto detach; }

        jmethodID show = env->GetMethodID(toastClass, "show", "()V");
        if (show) env->CallVoidMethod(toast, show);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
detach:
    if (attached) g_vm->DetachCurrentThread();
}

static void* loaderThread(void*) {
    writeLog("[Loader] Thread started");

    void* libUnity = nullptr;
    void* libEGL   = nullptr;
    int   attempts = 0;

    while (!libUnity || !libEGL) {
        libUnity = dlopen("libunity.so", RTLD_NOW | RTLD_NOLOAD);
        libEGL   = dlopen("libEGL.so",   RTLD_NOW | RTLD_NOLOAD);

        char buf[128];
        snprintf(buf, sizeof(buf), "[Loader] Attempt %d: unity=%p egl=%p",
            attempts++, libUnity, libEGL);
        writeLog(buf);

        if (!libUnity || !libEGL) {
            if (libUnity) dlclose(libUnity);
            if (libEGL)   dlclose(libEGL);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        if (attempts > 300) { writeLog("[Loader] Timed out!"); return nullptr; }
    }

    dlclose(libUnity);
    dlclose(libEGL);

    writeLog("[Loader] Both libs ready, waiting for GL context...");
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    std::string dir = getNativeLibDir();
    if (dir.empty()) { writeLog("[Loader] ERROR: lib dir not found"); return nullptr; }

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

    writeLog("[Loader] libModMenu.so loaded!");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    showToastSafe("Mod Menu Loaded!");
    return nullptr;
}

jint JNI_OnLoad(JavaVM* vm, void*) {
    g_vm = vm;
    writeLog("[Loader] JNI_OnLoad called");
    pthread_t t;
    pthread_create(&t, nullptr, loaderThread, nullptr);
    pthread_detach(t);
    return JNI_VERSION_1_6;
}
