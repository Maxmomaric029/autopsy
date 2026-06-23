#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <array>
#include <string>
#include <vector>
#include <cmath>
#include "animation.h"

namespace theme {

    // ========================================================================
    // Red + Milk White palette
    // ========================================================================
    // BG:          #0D0A0A
    // SURFACE:     #140E0E
    // SURFACE2:    #1A1111
    // ACCENT:      #E03040  (red)
    // TEXT:        #F5F0EB  (milk white)
    // MUTED:       #6A5A5A
    // DANGER:      #FF2233
    // ========================================================================

    inline ImVec4 c_bg       = ImVec4(0.0510f, 0.0392f, 0.0392f, 1.f); // #0D0A0A
    inline ImVec4 c_surface  = ImVec4(0.0784f, 0.0549f, 0.0549f, 1.f); // #140E0E
    inline ImVec4 c_surface2 = ImVec4(0.1020f, 0.0667f, 0.0667f, 1.f); // #1A1111
    inline ImVec4 c_border   = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
    inline ImVec4 c_border_hv= ImVec4(1.0f, 1.0f, 1.0f, 0.12f);

    inline ImVec4 c_accent   = ImVec4(0.8784f, 0.1882f, 0.2510f, 1.f); // #E03040
    inline ImU32  c_accent_u32 = IM_COL32(224, 48, 64, 255);

    inline ImVec4 c_text     = ImVec4(0.9608f, 0.9412f, 0.9216f, 1.f); // #F5F0EB  milk white
    inline ImVec4 c_muted    = ImVec4(0.4157f, 0.3529f, 0.3529f, 1.f); // #6A5A5A
    inline ImVec4 c_muted2   = ImVec4(0.2510f, 0.2000f, 0.2000f, 1.f); // #403333
    inline ImVec4 c_danger   = ImVec4(1.0000f, 0.1333f, 0.2000f, 1.f); // #FF2233

    // ========================================================================
    // 8pt spacing system
    // ========================================================================
    namespace space {
        inline constexpr float xs = 4.f;
        inline constexpr float sm = 8.f;
        inline constexpr float md = 12.f;
        inline constexpr float lg = 16.f;
        inline constexpr float xl = 24.f;
    }

    // ========================================================================
    // Radius constants
    // ========================================================================
    inline float r_window = 12.f;
    inline float r_card   = 10.f;
    inline float r_frame  = 7.f;
    inline float r_button = 5.f;

    // ========================================================================
    // Layout constants
    // ========================================================================
    inline constexpr float kSidebarW      = 64.f;
    inline constexpr float kTopbarH       = 52.f;
    inline constexpr float kSidebarTabH   = 40.f;
    inline constexpr float kSidebarTabGap = 6.f;
    inline constexpr float kLogoSize      = 36.f;
    inline constexpr float kRightPanelW   = 240.f;

    inline float kWinW = 860.f;
    inline float kWinH = 580.f;

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

    inline ImU32 col_bg(float a = 1.f)      { return to_u32(c_bg, a); }
    inline ImU32 col_surface(float a = 1.f) { return to_u32(c_surface, a); }
    inline ImU32 col_surface2(float a = 1.f){ return to_u32(c_surface2, a); }
    inline ImU32 col_accent(float a = 1.f)  { return to_u32(c_accent, a); }
    inline ImU32 col_text(float a = 1.f)    { return to_u32(c_text, a); }
    inline ImU32 col_muted(float a = 1.f)   { return to_u32(c_muted, a); }
    inline ImU32 col_muted2(float a = 1.f)  { return to_u32(c_muted2, a); }
    inline ImU32 col_border(float a = 1.f)  { return to_u32(c_border, a); }
    inline ImU32 col_border_hv(float a = 1.f){ return to_u32(c_border_hv, a); }
    inline ImU32 col_danger(float a = 1.f)  { return to_u32(c_danger, a); }
    inline ImU32 col_success(float a = 1.f) { return IM_COL32(80, 230, 120, (int)(255 * a)); }
    inline ImU32 col_warn(float a = 1.f)    { return IM_COL32(255, 200, 50, (int)(255 * a)); }

    // ========================================================================
    // Accent helpers
    // ========================================================================
    inline ImU32 accent_dim() {
        return IM_COL32(224, 48, 64, (int)(255 * 0.12f));
    }

    inline ImU32 accent_border() {
        return IM_COL32(224, 48, 64, (int)(255 * 0.40f));
    }

    inline ImU32 accent_glow(float t = 1.f) {
        return IM_COL32(224, 48, 64, (int)(255 * 0.25f * t));
    }

    // ========================================================================
    // Shadow drawing — 3 layers, subtle (per spec)
    // ========================================================================
    inline void card_shadow(ImDrawList* dl, ImVec2 mn, ImVec2 mx, float rounding) {
        // Layer 0: offset (0,1) size +3px, black alpha 110, r=13
        dl->AddRectFilled(mn + ImVec2(0.f, 1.f) - ImVec2(1.5f, 1.5f),
            mx + ImVec2(1.5f, 1.5f),
            IM_COL32(0, 0, 0, 110), rounding + 3.f);
        // Layer 1: offset (0,3) size +8px, black alpha 65, r=16
        dl->AddRectFilled(mn + ImVec2(0.f, 3.f) - ImVec2(4.f, 4.f),
            mx + ImVec2(4.f, 4.f),
            IM_COL32(0, 0, 0, 65), rounding + 8.f);
        // Layer 2: offset (0,2) size +6px, rgba(224,48,64,14) — tint depth
        dl->AddRectFilled(mn + ImVec2(0.f, 2.f) - ImVec2(3.f, 3.f),
            mx + ImVec2(3.f, 3.f),
            IM_COL32(224, 48, 64, 14), rounding + 6.f);
    }

} // namespace theme
