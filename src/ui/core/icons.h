#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include "theme.h"

// ========================================================================
// Custom icon drawing functions using ImDrawList primitives
// These render without requiring FontAwesome or any icon font.
// ========================================================================

namespace icon {

    using Fn = void(*)(ImDrawList*, ImVec2, float, ImU32);

    static void crosshair(ImDrawList* dl, ImVec2 c, float sz, ImU32 col) {
        const float r = sz * .38f, gap = sz * .13f, ll = sz * .26f, lw = 1.5f;
        dl->AddCircle(c, r, col, 32, lw);
        dl->AddCircleFilled(c, 2.0f, col);
        dl->AddLine({ c.x,       c.y - r - gap }, { c.x,       c.y - r - gap - ll }, col, lw);
        dl->AddLine({ c.x,       c.y + r + gap }, { c.x,       c.y + r + gap + ll }, col, lw);
        dl->AddLine({ c.x - r - gap, c.y }, { c.x - r - gap - ll, c.y }, col, lw);
        dl->AddLine({ c.x + r + gap, c.y }, { c.x + r + gap + ll, c.y }, col, lw);
    }

    static void eye(ImDrawList* dl, ImVec2 c, float sz, ImU32 col) {
        const float rx = sz * .44f, ry = sz * .22f, lw = 1.5f;
        const ImVec2 L = { c.x - rx, c.y }, R = { c.x + rx, c.y };
        const ImVec2 T = { c.x, c.y - ry }, B = { c.x, c.y + ry };
        dl->AddBezierCubic(L, { L.x + rx * .65f, T.y - ry * .3f },
            { R.x - rx * .65f, T.y - ry * .3f }, R, col, lw);
        dl->AddBezierCubic(L, { L.x + rx * .65f, B.y + ry * .3f },
            { R.x - rx * .65f, B.y + ry * .3f }, R, col, lw);
        dl->AddCircleFilled(c, sz * .13f, col);
    }

    static void globe(ImDrawList* dl, ImVec2 c, float sz, ImU32 col) {
        const float r = sz * .40f, cx = sz * .22f, lw = 1.5f;
        dl->AddCircle(c, r, col, 32, lw);
        dl->AddLine({ c.x - r, c.y }, { c.x + r, c.y }, col, lw);
        dl->AddBezierCubic({ c.x, c.y - r }, { c.x + cx, c.y - r * .5f },
            { c.x + cx, c.y + r * .5f }, { c.x, c.y + r }, col, lw);
        dl->AddBezierCubic({ c.x, c.y - r }, { c.x - cx, c.y - r * .5f },
            { c.x - cx, c.y + r * .5f }, { c.x, c.y + r }, col, lw);
    }

    static void layer(ImDrawList* dl, ImVec2 c, float sz, ImU32 col) {
        const float hw = sz * .35f, hh = sz * .07f, gap = sz * .17f;
        for (int i = -1; i <= 1; i++)
            dl->AddRectFilled({ c.x - hw, c.y + i * gap - hh },
                { c.x + hw, c.y + i * gap + hh }, col, 1.f);
    }

    static void gear(ImDrawList* dl, ImVec2 c, float sz, ImU32 col) {
        const float ri = sz * .28f, ro = sz * .42f;
        dl->AddCircle(c, ri, col, 24, 1.5f);
        for (int i = 0; i < 6; i++) {
            const float a = (float)i / 6.f * 6.2832f;
            const float a1 = a - .24f, a2 = a + .24f;
            dl->AddQuadFilled(
                { c.x + cosf(a1) * ri, c.y + sinf(a1) * ri },
                { c.x + cosf(a2) * ri, c.y + sinf(a2) * ri },
                { c.x + cosf(a2) * ro, c.y + sinf(a2) * ro },
                { c.x + cosf(a1) * ro, c.y + sinf(a1) * ro }, col);
        }
    }

    static void diamond(ImDrawList* dl, ImVec2 c, float sz, ImU32 col) {
        const float r = sz * .42f;
        dl->AddQuad({ c.x, c.y - r }, { c.x + r, c.y },
            { c.x, c.y + r }, { c.x - r, c.y }, col, 1.6f);
        dl->AddLine({ c.x - r * .45f, c.y }, { c.x + r * .45f, c.y }, col, 1.3f);
        dl->AddCircleFilled(c, sz * .08f, col, 12);
    }

    static void home(ImDrawList* dl, ImVec2 c, float sz, ImU32 col) {
        const float s = sz * .45f;
        dl->AddTriangleFilled(
            { c.x, c.y - s },
            { c.x - s, c.y },
            { c.x + s, c.y }, col);
        dl->AddRectFilled({ c.x - s * .5f, c.y }, { c.x + s * .5f, c.y + s * .7f },
            col, 1.f);
    }

    static void monitor(ImDrawList* dl, ImVec2 c, float sz, ImU32 col) {
        const float hw = sz * .4f, hh = sz * .3f, lw = 1.3f;
        dl->AddRect({ c.x - hw, c.y - hh }, { c.x + hw, c.y + hh }, col, 2.f, 0, lw);
        dl->AddLine({ c.x - hw * .5f, c.y + hh }, { c.x + hw * .5f, c.y + hh }, col, lw);
        dl->AddLine({ c.x - hw * .3f, c.y + hh + sz * .12f },
            { c.x + hw * .3f, c.y + hh + sz * .12f }, col, lw);
    }

    // Tab icon set — indexed by section
    inline constexpr Fn tabIcons[] = {
        crosshair, // Aimbot
        eye,       // Visuals
        globe,     // World
        layer,     // Misc
        diamond,   // Blade Ball
        gear       // Settings
    };

    inline constexpr const char* tabLabels[] = {
        "Aimbot", "Visuals", "World", "Misc", "Blade Ball", "Settings"
    };

    inline constexpr int kTabCount = 6;

} // namespace icon
