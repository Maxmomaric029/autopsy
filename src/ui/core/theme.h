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
    // Aurora Dark palette
    // ========================================================================
    inline ImVec4 c_bg       = ImVec4(0.0117f, 0.0235f, 0.0392f, 1.f); // #03060A
    inline ImVec4 c_surface  = ImVec4(0.0274f, 0.0627f, 0.1020f, 1.f); // #07101A
    inline ImVec4 c_surface2 = ImVec4(0.0431f, 0.0863f, 0.1412f, 1.f); // #0B1624
    inline ImVec4 c_card     = ImVec4(0.0509f, 0.1020f, 0.1686f, 1.f); // #0D1A2B
    inline ImVec4 c_border   = ImVec4(0.1020f, 0.2000f, 0.2824f, 1.f); // #1A3348
    inline ImVec4 c_accent   = ImVec4(0.0000f, 0.6824f, 1.0000f, 1.f); // #00AEFF
    inline ImVec4 c_accent2  = ImVec4(0.4196f, 0.4863f, 1.0000f, 1.f); // #6B7CFF
    inline ImVec4 c_text     = ImVec4(0.9176f, 0.9569f, 1.0000f, 1.f); // #EAF4FF
    inline ImVec4 c_muted    = ImVec4(0.5569f, 0.6431f, 0.7098f, 1.f); // #8EA4B5
    inline ImVec4 c_danger   = ImVec4(1.0000f, 0.3020f, 0.4275f, 1.f); // #FF4D6D
    inline ImVec4 c_success  = ImVec4(0.3373f, 0.9412f, 0.6980f, 1.f); // #56F0B2
    inline ImVec4 c_warning  = ImVec4(1.0000f, 0.7843f, 0.3412f, 1.f); // #FFC857

    inline float r_window = 14.f;
    inline float r_card   = 12.f;
    inline float r_frame  = 7.f;

    // Layout constants
    inline constexpr float kSidebarW      = 178.f;
    inline constexpr float kSidebarCompactW = 58.f;
    inline constexpr float kTabH          = 42.f;
    inline constexpr float kLogoH         = 84.f;
    inline constexpr float kHeaderH       = 58.f;
    inline constexpr float kPad           = 12.f;
    inline constexpr float kCardGap       = 6.f;

    inline float kWinW = 760.f;
    inline float kWinH = 548.f;

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
