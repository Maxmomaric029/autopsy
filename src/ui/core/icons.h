#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include "theme.h"
#include "fonts.h"
#include "FontAwesome/IconsFontAwesome6.h"

// ========================================================================
// FontAwesome 6 icon definitions for the new 5-tab navigation
// ========================================================================

namespace icon {

    // Tab 0 — Aimbot:   ICON_FA_CROSSHAIRS
    // Tab 1 — Visuals:  ICON_FA_BINOCULARS
    // Tab 2 — World:    ICON_FA_SKULL
    // Tab 3 — Misc:     ICON_FA_BOLT
    // Tab 4 — Settings: ICON_FA_GEAR

    inline constexpr const char* tabIconChars[] = {
        ICON_FA_CROSSHAIRS,   // Aimbot
        ICON_FA_BINOCULARS,   // Visuals
        ICON_FA_SKULL,        // World
        ICON_FA_BOLT,         // Misc
        ICON_FA_GEAR          // Settings
    };

    inline constexpr const char* tabLabels[] = {
        "Aimbot", "Visuals", "World", "Misc", "Settings"
    };

    // Badge tag for topbar context
    inline constexpr const char* tabBadges[] = {
        "Aim", "Esp", "Wrld", "Misc", "Cfg"
    };

    inline constexpr int kTabCount = 5;

    // ========================================================================
    // Draw a FontAwesome icon at a position
    // ========================================================================
    inline void draw(ImDrawList* dl, ImVec2 pos, const char* iconChar,
        ImU32 color, float fontSize = 16.f) {
        ImFont* f = font::bold();
        dl->AddText(f, fontSize, pos, color, iconChar);
    }

    // ========================================================================
    // Measure the width of a FontAwesome icon
    // ========================================================================
    inline ImVec2 measure(const char* iconChar, float fontSize = 16.f) {
        ImFont* f = font::bold();
        return f->CalcTextSizeA(fontSize, FLT_MAX, 0.f, iconChar);
    }

    // ========================================================================
    // Convenience: Draw a tab icon centered in a rect
    // ========================================================================
    inline void tabIcon(ImDrawList* dl, ImVec2 center, const char* iconChar,
        ImU32 color, float size = 18.f) {
        ImVec2 ts = measure(iconChar, size);
        ImVec2 pos(center.x - ts.x * 0.5f, center.y - ts.y * 0.5f);
        draw(dl, pos, iconChar, color, size);
    }

} // namespace icon
