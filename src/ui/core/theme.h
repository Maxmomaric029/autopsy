#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <array>
#include <string>
#include <vector>
#include "animation.h"

namespace theme {

    // ========================================================================
    // Premium dark palette — VS Code dark+ meets high-end peripheral software
    // ========================================================================
    inline ImVec4 c_bg       = ImVec4(0.0157f, 0.0196f, 0.0314f, 1.f); // #040508 true near-black
    inline ImVec4 c_surface  = ImVec4(0.0392f, 0.0510f, 0.0784f, 1.f); // #0A0D14 first elevation
    inline ImVec4 c_surface2 = ImVec4(0.0627f, 0.0784f, 0.1176f, 1.f); // #10141E second elevation
    inline ImVec4 c_card     = ImVec4(0.0510f, 0.0667f, 0.1020f, 1.f); // #0D111A card surface
    inline ImVec4 c_border   = ImVec4(1.0f, 1.0f, 1.0f, 0.0706f); // white at alpha 18 (subtle border)

    inline ImVec4 c_accent   = ImVec4(0.9020f, 0.1960f, 0.2549f, 1.f); // #E63241
    inline ImVec4 c_accent2  = ImVec4(1.0000f, 0.3372f, 0.3372f, 1.f); // #FF5656 highlight

    inline ImVec4 c_text     = ImVec4(0.9412f, 0.9647f, 1.0000f, 1.f); // #F0F6FF
    inline ImVec4 c_muted    = ImVec4(0.4196f, 0.5020f, 0.6000f, 1.f); // #6B8099 slightly more blue
    inline ImVec4 c_danger   = ImVec4(1.0000f, 0.3020f, 0.4275f, 1.f); // #FF4D6D
    inline ImVec4 c_success  = ImVec4(0.3373f, 0.9412f, 0.6980f, 1.f); // #56F0B2
    inline ImVec4 c_warning  = ImVec4(1.0000f, 0.7843f, 0.3412f, 1.f); // #FFC857

    // ========================================================================
    // 8pt spacing system (F2.1)
    // ========================================================================
    namespace space {
        inline constexpr float xs = 4.f;   // between control and sub-option
        inline constexpr float sm = 8.f;   // between related controls
        inline constexpr float md = 12.f;  // between groups within a card
        inline constexpr float lg = 16.f;  // card padding / margins
        inline constexpr float xl = 24.f;  // between cards / page margins
    }

    inline float r_window = 14.f;
    inline float r_card   = 12.f;
    inline float r_frame  = 7.f;

    // Glassmorphism sidebar colors
    inline ImVec4 c_sidebar  = ImVec4(0.063f, 0.071f, 0.086f, 0.92f); // #101216 + alpha
    inline ImVec4 c_sideAccent = ImVec4(0.902f, 0.235f, 0.275f, 1.f); // #E63C46 blood red accent
    inline ImVec4 c_sideText = ImVec4(0.627f, 0.667f, 0.745f, 1.f);   // #A0AABE
    inline ImVec4 c_sideTextActive = ImVec4(0.863f, 0.902f, 0.961f, 1.f); // #DCE6F5

    // Layout constants
    inline constexpr float kSidebarW      = 190.f;
    inline constexpr float kTabH          = 44.f;
    inline constexpr float kTabGap        = 4.f;
    inline constexpr float kLogoH         = 78.f;
    inline constexpr float kPad           = 14.f;
    inline constexpr float kCardGap       = 6.f; // kept for compatibility, use space::md

    inline float kWinW = 840.f;
    inline float kWinH = 540.f;

    // ========================================================================
    // Color helpers
    // ========================================================================
    inline ImU32 to_u32(const ImVec4& c, float alphaMul = 1.f) {
        return ImGui::ColorConvertFloat4ToU32(
            ImVec4(c.x, c.y, c.z, c.w * alphaMul));
    }

    inline ImU32 alpha(ImU32 col, float a) {
        int alpha = (int)(((col >> 24) & 0xFF) * ImClamp(a, 0.f, 1.f));
        return (col & 0x00FFFFFF) | ((ImU32)alpha << 24);
    }

    inline ImU32 lerp_u32(ImU32 a, ImU32 b, float t) {
        auto ch = [](ImU32 c, int s) { return (int)((c >> s) & 0xFF); };
        auto mix = [&](int s) { return (int)(ch(a, s) * (1.f - t) + ch(b, s) * t); };
        return IM_COL32(mix(0), mix(8), mix(16), mix(24));
    }

    inline ImU32 col_accent(float a = 1.f)  { return to_u32(c_accent, a); }
    inline ImU32 col_accent2(float a = 1.f) { return to_u32(c_accent2, a); }
    inline ImU32 col_text(float a = 1.f)    { return to_u32(c_text, a); }
    inline ImU32 col_muted(float a = 1.f)   { return to_u32(c_muted, a); }
    inline ImU32 col_card(float a = 1.f)    { return to_u32(c_card, a); }
    inline ImU32 col_bg(float a = 1.f)      { return to_u32(c_bg, a); }
    inline ImU32 col_surface(float a = 1.f) { return to_u32(c_surface, a); }
    inline ImU32 col_border(float a = 1.f)  { return to_u32(c_border, a); }
    inline ImU32 col_danger(float a = 1.f)  { return to_u32(c_danger, a); }
    inline ImU32 col_success(float a = 1.f) { return to_u32(c_success, a); }
    inline ImU32 col_warn(float a = 1.f)    { return to_u32(c_warning, a); }

    // ========================================================================
    // Shadow drawing
    // ========================================================================
    inline void shadow(ImDrawList* dl, ImVec2 mn, ImVec2 mx,
                       float rounding, float strength = 1.f) {
        for (int i = 0; i < 5; i++) {
            float spread = 4.f + i * 3.5f;
            int a = (int)((32.f - i * 4.5f) * strength);
            dl->AddRectFilled(
                mn - ImVec2(spread * .45f, spread * .18f) + ImVec2(0.f, 3.f + i),
                mx + ImVec2(spread * .45f, spread * .62f),
                IM_COL32(0, 0, 0, a), rounding + spread);
        }
    }

} // namespace theme
