#include "menu.h"
#include "imgui.h"

// ─── Feature toggles ──────────────────────────────────────────────────────────
// These are simple static booleans for now.
// In a real mod you'd hook game functions based on these values.

static bool s_menuOpen   = true;

// ---- Visual ----
static bool s_esp        = false;
static bool s_wallhack   = false;
static bool s_noFog      = false;

// ---- Player ----
static bool s_godMode    = false;
static bool s_infiniteAmmo = false;
static float s_speedMultiplier = 1.0f;

// ---- Misc ----
static bool s_showFPS    = false;

// ─── Small helper: a colored toggle button ────────────────────────────────────
static void ToggleButton(const char* label, bool* value) {
    if (*value) {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.2f, 0.7f, 0.3f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.4f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.1f, 0.6f, 0.2f, 1.f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.6f, 0.1f, 0.1f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.2f, 0.2f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.5f, 0.05f, 0.05f, 1.f));
    }

    if (ImGui::Button(label, ImVec2(ImGui::GetContentRegionAvail().x, 36.f))) {
        *value = !*value;
    }

    ImGui::PopStyleColor(3);
}

// ─── Main draw function ───────────────────────────────────────────────────────

void DrawMenu() {
    ImGuiIO& io = ImGui::GetIO();

    // ── FPS overlay (always visible, top-right corner) ──
    if (s_showFPS) {
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 80.f, 10.f));
        ImGui::SetNextWindowSize(ImVec2(70.f, 30.f));
        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::Begin("##fps", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoInputs     |
            ImGuiWindowFlags_NoNav);
        ImGui::Text("%.0f FPS", io.Framerate);
        ImGui::End();
    }

    // ── Toggle button (always visible, bottom-left) ──
    {
        ImGui::SetNextWindowPos(ImVec2(10.f, io.DisplaySize.y - 60.f));
        ImGui::SetNextWindowSize(ImVec2(90.f, 50.f));
        ImGui::SetNextWindowBgAlpha(0.75f);
        ImGui::Begin("##toggle", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoScrollbar);

        if (ImGui::Button(s_menuOpen ? "Close" : "Open", ImVec2(80.f, 36.f))) {
            s_menuOpen = !s_menuOpen;
        }
        ImGui::End();
    }

    // ── Main menu window ──
    if (!s_menuOpen) return;

    // Position the menu in the centre of the screen on first use,
    // but let the user drag it afterwards.
    ImGui::SetNextWindowPos(
        ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
        ImGuiCond_Once,
        ImVec2(0.5f, 0.5f)   // pivot = centre
    );
    ImGui::SetNextWindowSize(ImVec2(280.f, 360.f), ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(0.90f);

    ImGui::Begin("Mod Menu", &s_menuOpen,
        ImGuiWindowFlags_NoCollapse);

    // ─── Visual tab ───────────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("Visual", ImGuiTreeNodeFlags_DefaultOpen)) {
        ToggleButton("ESP",       &s_esp);
        ToggleButton("Wallhack",  &s_wallhack);
        ToggleButton("No Fog",    &s_noFog);
        ToggleButton("Show FPS",  &s_showFPS);
    }

    ImGui::Spacing();

    // ─── Player tab ───────────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("Player")) {
        ToggleButton("God Mode",      &s_godMode);
        ToggleButton("Infinite Ammo", &s_infiniteAmmo);

        ImGui::Spacing();
        ImGui::Text("Speed: %.1fx", s_speedMultiplier);
        ImGui::SliderFloat("##speed", &s_speedMultiplier, 1.0f, 5.0f);
    }

    ImGui::Spacing();

    // ─── Info footer ──────────────────────────────────────────────────────
    ImGui::Separator();
    ImGui::TextDisabled("Display: %.0f x %.0f", io.DisplaySize.x, io.DisplaySize.y);
    ImGui::TextDisabled("FPS: %.1f", io.Framerate);

    ImGui::End();
}
