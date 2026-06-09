#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include "../core/theme.h"
#include "../core/animation.h"
#include "../core/fonts.h"
#include "../core/icons.h"

// ========================================================================
// Menu layout: horizontal pill tabs + clean content area
// ========================================================================

namespace layout {

    // ========================================================================
    // Header bar — logo + pill tabs + branding
    // ========================================================================
    static bool header(ImDrawList* dl, ImVec2 wp, ImVec2 ws,
        int& section, float menuEase) {

        const float kHeaderH = 56.f;
        constexpr float padX = 16.f;

        // Header background
        dl->AddRectFilled(wp, wp + ImVec2(ws.x, kHeaderH),
            IM_COL32(4, 10, 18, 220), theme::r_window,
            ImDrawFlags_RoundCornersTop);

        // Subtle accent line at bottom of header
        dl->AddRectFilled(wp + ImVec2(20.f, kHeaderH - 1.f),
            wp + ImVec2(ws.x - 20.f, kHeaderH),
            IM_COL32(0, 174, 255, 30), 0.f);

        // Logo text
        ImFont* logoF = font::logo();
        const float logoSize = logoF->LegacySize;
        const ImVec2 logoPos = wp + ImVec2(padX, 14.f);
        dl->AddText(logoF, logoSize, logoPos + ImVec2(1.f, 1.f),
            IM_COL32(15, 25, 40, 180), "AUTOPSY");
        dl->AddText(logoF, logoSize, logoPos,
            theme::col_text(), "AUTOPSY");

        // Beta badge
        ImVec2 logoSize2 = logoF->CalcTextSizeA(logoSize, FLT_MAX, 0.f, "AUTOPSY");
        dl->AddRectFilled(logoPos + ImVec2(logoSize2.x + 6.f, 2.f),
            logoPos + ImVec2(logoSize2.x + 34.f, logoSize - 2.f),
            IM_COL32(0, 78, 116, 200), 4.f);
        dl->AddText(logoPos + ImVec2(logoSize2.x + 9.f, 3.f),
            IM_COL32(0, 174, 255, 235), "BETA");

        // Pill tabs
        const float tabStartX = 210.f;
        const float tabGap = 4.f;
        const float tabH = 32.f;
        float cx = wp.x + tabStartX;

        bool sectionChanged = false;

        for (int i = 0; i < icon::kTabCount; i++) {
            const bool active = (i == section);

            // Measure tab size
            const char* iconChar = icon::tabIconChars[i];
            const char* label = icon::tabLabels[i];
            char tabText[64];
            snprintf(tabText, sizeof(tabText), "%s  %s", iconChar, label);

            ImFont* tabF = font::bold();
            ImVec2 textSize = tabF->CalcTextSizeA(16.f * 0.85f, FLT_MAX, 0.f, label);
            ImVec2 iconSize = tabF->CalcTextSizeA(16.f * 0.85f, FLT_MAX, 0.f, iconChar);
            float tabW = iconSize.x + textSize.x + 18.f;
            if (tabW < 72.f) tabW = 72.f;

            ImVec2 tMin(cx, wp.y + (kHeaderH - tabH) * 0.5f);
            ImVec2 tMax(cx + tabW, tMin.y + tabH);

            ImGui::SetCursorScreenPos(tMin);
            ImGui::PushID(i);
            ImGui::InvisibleButton("##tab", { tabW, tabH });

            const bool clicked = ImGui::IsItemClicked();
            const bool hov = ImGui::IsItemHovered();
            const ImGuiID id = ImGui::GetItemID();
            ImGui::PopID();

            if (clicked && i != section) {
                section = i;
                sectionChanged = true;
            }

            const float t = anim::ease_out_cubic(anim::g_anim.get(id, active || hov, 10.f));
            const float at = anim::ease_out_cubic(anim::g_anim.get(id ^ 0x8A13C4u, active, 11.f));

            // Tab background
            const ImU32 bg = theme::lerp_u32(
                IM_COL32(6, 14, 26, (int)(80.f * (hov ? 1.5f : 1.f))),
                IM_COL32(0, 60, 90, 200), at);
            dl->AddRectFilled(tMin, tMax, bg, tabH * 0.5f);

            // Active glow
            if (at > 0.05f) {
                dl->AddRectFilled(tMin, tMax,
                    IM_COL32(0, 174, 255, (int)(at * 8.f)), tabH * 0.5f);
                dl->AddRect(tMin, tMax,
                    IM_COL32(0, 174, 255, (int)(at * 80.f)), tabH * 0.5f, 0, 1.2f);
            }

            // Vertical divider (non-active only)
            if (!active && i > 0 && at < 0.05f) {
                dl->AddLine(
                    { tMin.x - tabGap * 0.5f, tMin.y + 8.f },
                    { tMin.x - tabGap * 0.5f, tMax.y - 8.f },
                    IM_COL32(60, 75, 90, 60), 1.f);
            }

            // Icon + label text
            const float iconColorFade = active ? 0.85f : (hov ? 0.6f : 0.35f);
            const float textColorFade = active ? 1.0f : (hov ? 0.9f : 0.6f);
            const float textOff = at * 1.5f;

            dl->AddText(tabF, 16.f * 0.85f,
                tMin + ImVec2(7.f + textOff, (tabH - 16.f * 0.85f) * 0.5f),
                theme::alpha(active ? theme::col_accent() : theme::col_muted(), iconColorFade),
                iconChar);

            dl->AddText(tabF, 16.f * 0.85f,
                tMin + ImVec2(7.f + iconSize.x + 4.f + textOff,
                    (tabH - 16.f * 0.85f) * 0.5f),
                theme::alpha(theme::col_text(), textColorFade),
                label);

            cx += tabW + tabGap;
        }

        return sectionChanged;
    }

    // ========================================================================
    // Content area background
    // ========================================================================
    static void contentBg(ImDrawList* dl, ImVec2 wp, ImVec2 ws) {
        const float kHeaderH = 56.f;
        const ImVec2 cMin = wp + ImVec2(0.f, kHeaderH);
        const ImVec2 cMax = wp + ws;

        dl->AddRectFilled(cMin, cMax,
            IM_COL32(5, 9, 16, 240), theme::r_window,
            ImDrawFlags_RoundCornersBottom);

        // Subtle inner gradient
        dl->AddRectFilledMultiColor(cMin, ImVec2(cMin.x + 1.f, cMax.y),
            IM_COL32(0, 174, 255, 16), IM_COL32(0, 0, 0, 0),
            IM_COL32(0, 0, 0, 0), IM_COL32(0, 174, 255, 16));
    }

    // ========================================================================
    // Menu backdrop
    // ========================================================================
    static void backdrop(ImDrawList* dl, ImVec2 menuMin, ImVec2 menuMax,
        float menuT, float r) {

        const ImVec2 display = ImGui::GetIO().DisplaySize;
        const float bt = anim::ease_in_out_cubic(menuT);

        // Dark overlay
        dl->AddRectFilled({ 0.f, 0.f }, display,
            IM_COL32(2, 4, 8, (int)(bt * 28.f)));

        // Shadow rings
        for (int i = 0; i < 5; i++) {
            const float spread = 20.f + i * 12.f;
            const int alpha = (int)((10.f - i * 1.5f) * bt);
            dl->AddRectFilled(menuMin - ImVec2(spread, spread * 0.5f),
                menuMax + ImVec2(spread, spread),
                IM_COL32(6, 12, 20, alpha), r + spread);
        }

        // Outer glow
        dl->AddRectFilledMultiColorRounded(menuMin - ImVec2(30.f, 20.f),
            menuMax + ImVec2(30.f, 30.f),
            IM_COL32(0, 174, 255, (int)(bt * 6.f)),
            IM_COL32(100, 117, 255, (int)(bt * 5.f)),
            IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), r + 28.f);
    }

    // ========================================================================
    // Particles effect
    // ========================================================================
    static void dissolve(ImDrawList* dl, ImVec2 mn, ImVec2 mx, float t, float time) {
        const float a = sinf(anim::saturate(t) * 3.14159265f);
        if (a <= .01f) return;

        const float w = mx.x - mn.x;
        const float h = mx.y - mn.y;
        for (int i = 0; i < 25; i++) {
            const float seed = (float)i * 13.37f;
            const float u = fmodf(fabsf(sinf(seed) * 43758.5453f), 1.f);
            const float v = fmodf(fabsf(sinf(seed + 42.0f) * 24634.6345f), 1.f);
            const float drift = sinf(time * 1.8f + seed) * 6.f * (1.f - fabsf(t - .5f) * 1.5f);
            const float size = 1.2f + v * 1.5f;

            ImVec2 p;
            if ((i & 3) == 0)
                p = { mn.x + u * w, mn.y - 10.f + drift };
            else if ((i & 3) == 1)
                p = { mn.x + u * w, mx.y + 8.f + drift };
            else if ((i & 3) == 2)
                p = { mn.x - 10.f + drift, mn.y + u * h };
            else
                p = { mx.x + 8.f + drift, mn.y + u * h };

            const int alpha = (int)(a * (12.f + v * 24.f));
            const ImU32 col = (i % 5) == 0
                ? IM_COL32(100, 117, 255, alpha)
                : IM_COL32(0, 174, 255, alpha);
            dl->AddRectFilled(p, p + ImVec2(size, size), col, 1.f);
        }
    }

} // namespace layout
