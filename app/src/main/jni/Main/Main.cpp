#include <android/log.h>
#include <stdio.h>

#define LOG_TAG "ModMenu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

__attribute__((constructor))
void onLoad() {
    LOGI("=== libModMenu.so loaded! ===");
    FILE* f = fopen("/storage/emulated/0/Documents/modmenu_log.txt", "w");
    if (f) { fprintf(f, "ModMenu loaded!\n"); fclose(f); }
}
