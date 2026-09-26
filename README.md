# Meow — ImGui Android Mod Menu (OpenGL ES)
> by Coloradodevly | https://github.com/Coloradodevly/Meow

A starter template for an Android mod menu using Dear ImGui and an EGL hook.
Targets **ARM64** (your Poco M6 Pro 4G).

---

## Project Structure

```
jni/
├── main.cpp              ← Entry point (.so constructor/destructor)
├── CMakeLists.txt        ← Build config — add new .cpp files here
│
├── imgui/                ← Drop ImGui files here (see step 1 below)
│   ├── imgui.h / imgui.cpp
│   ├── imgui_draw.cpp
│   ├── imgui_tables.cpp
│   ├── imgui_widgets.cpp
│   ├── imgui_internal.h
│   ├── imconfig.h
│   └── backends/
│       ├── imgui_impl_opengl3.h
│       └── imgui_impl_opengl3.cpp
│
├── hooks/
│   ├── egl_hook.h        ← Hook interface
│   ├── egl_hook.cpp      ← Hooks eglSwapBuffers, inits ImGui
│   └── And64InlineHook.hpp  ← (download separately, see step 2)
│
└── menu/
    ├── menu.h            ← DrawMenu() declaration
    └── menu.cpp          ← All your ImGui widgets go here
```

---

## Setup Steps

### Step 1 — Get ImGui

Clone Dear ImGui from GitHub:
```
https://github.com/ocornut/imgui
```

Copy these files into `jni/imgui/`:
- `imgui.h`, `imgui.cpp`
- `imgui_draw.cpp`, `imgui_tables.cpp`, `imgui_widgets.cpp`
- `imgui_internal.h`, `imconfig.h`
- `imstb_rectpack.h`, `imstb_textedit.h`, `imstb_truetype.h`

Copy these into `jni/imgui/backends/`:
- `backends/imgui_impl_opengl3.h`
- `backends/imgui_impl_opengl3.cpp`
- `backends/imgui_impl_opengl3_loader.h`

### Step 2 — Get And64InlineHook

Download `And64InlineHook.hpp` from:
```
https://github.com/Rprop/And64InlineHook
```
Place it inside `jni/hooks/`.

### Step 3 — Open in Android Studio

1. Open the project root in Android Studio
2. Let it sync and index (first time takes ~1 min)
3. Make sure NDK is installed:
   - SDK Manager → SDK Tools → NDK (Side by side) ✓
4. Build → Make Project

### Step 4 — Inject the .so

After building, find your `.so` at:
```
app/build/intermediates/cmake/debug/obj/arm64-v8a/libmodmenu.so
```

Use your preferred injection method (Zygisk, custom loader, etc.) to
inject `libmodmenu.so` into the target game process.

---

## Customising the Menu

**All your menu UI lives in `menu/menu.cpp`.**

To add a new feature:
1. Add a `static bool` at the top of `menu.cpp`
2. Add a `ToggleButton(...)` call inside `DrawMenu()`
3. Hook the game function elsewhere and check the bool

Example:
```cpp
static bool s_myFeature = false;
// inside DrawMenu():
ToggleButton("My Feature", &s_myFeature);
```

---

## Touch Input

Touch is forwarded to ImGui via `handleTouch()` in `egl_hook.cpp`.
You need to hook or intercept the game's input path and call:
```cpp
extern "C" void handleTouch(float x, float y, int action);
// action: 0=DOWN  1=UP  2=MOVE
```

---

## Notes

- Built for **OpenGL ES 3.0** (`#version 300 es`)
- Targets **arm64-v8a** (your Poco M6 Pro 4G is ARM64)
- No root required for the overlay, but injection still requires a loader
- The menu is draggable — ImGui handles window movement natively
