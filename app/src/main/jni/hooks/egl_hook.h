#pragma once

// Sets up the hook on eglSwapBuffers.
// Returns true if the hook was installed successfully.
bool setupEGLHook();

// Removes the hook and cleans up ImGui.
void removeEGLHook();
