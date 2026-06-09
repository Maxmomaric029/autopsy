#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <d3d11.h>
#include "../core/theme.h"
#include "../core/animation.h"
#include "../core/fonts.h"
#include "../core/icons.h"

// Logo textures (defined in modern_ui.cpp, exported via modern_ui.h)
#include "../modern_ui.h"

// ========================================================================
// Menu layout: glassmorphism gray sidebar + content area
// ========================================================================

namespace layout {

    // ========================================================================
    // Sidebar — gray glassmorphism with stacked pill tabs
    // ========================================================================
    static bool sidebar(ImDrawList* dl, ImVec2 wp, ImVec2 ws,
        int& section, float menuEase) {

        constexpr float kSideW = 190.f;
        constexpr float kTabH = 48.f;
        constexpr float kLogoH = 78.f;

        // ---- Sidebar background (pure black with red edge) ----
        ImVec2 sbMin = wp;
        ImVec2 sbMax = wp + ImVec2(kSideW, ws.y);

        // Shadow
        for (int i = 0; i < 4; i++) {
            float spread = 6.f + i * 5.f;
            dl->AddRectFilled(sbMin - ImVec2(spread * 0.3f, spread * 0.1f),
                sbMax + ImVec2(spread * 0.4f, spread * 0.5f),
                IM_COL32(0, 0, 0, (int)((22.f - i * 3.5f))), theme::r_window + spread);
        }

        // Main pure black bg (was gray)
        dl->AddRectFilled(sbMin, sbMax,
            IM_COL32(0, 0, 0, 240), theme::r_window, ImDrawFlags_RoundCornersLeft);

        // Red accent line on the right edge
        dl->AddRectFilled(sbMax - ImVec2(2.f, 0.f), sbMax,
            IM_COL32(220, 60, 70, 90), 0.f);

        // Bottom accent line (red)
        dl->AddRectFilled(sbMin + ImVec2(16.f, kLogoH),
            sbMin + ImVec2(kSideW - 16.f, kLogoH + 1.f),
            IM_COL32(220, 60, 70, 80));

        // Version text color update to match
        // (already below)

        // ---- Logo ----
        if (g_sidebar_logo) {
            float logoAreaH = kLogoH - 22.f;
            float aspect = (float)g_sidebar_logoW / (float)g_sidebar_logoH;
            float logoH = ImMin(logoAreaH, 44.f);
            float logoW = logoH * aspect;
            float maxLogoW = kSideW - 40.f;
            if (logoW > maxLogoW) { logoW = maxLogoW; logoH = logoW / aspect; }
            ImVec2 logoMin = sbMin + ImVec2(18.f, (kLogoH - logoH) * 0.5f);
            dl->AddImage(g_sidebar_logo, logoMin, logoMin + ImVec2(logoW, logoH));
        } else {
            // Fallback: text logo
            ImFont* logoF = font::logo();
            float logoS = logoF->LegacySize * 0.9f;
            ImVec2 logoPos = sbMin + ImVec2(18.f, 22.f);
            dl->AddText(logoF, logoS, logoPos + ImVec2(1.f, 1.f),
                IM_COL32(255, 40, 50, 180), "MISERABLE");
            dl->AddText(logoF, logoS, logoPos,
                IM_COL32(230, 60, 70, 245), "MISERABLE");
        }

        // Version text
        dl->AddText(font::mono(), font::mono()->LegacySize * 0.85f,
            sbMin + ImVec2(20.f, 54.f),
            IM_COL32(140, 150, 170, 120), "v1.0.0");

        // ---- Tab buttons ----
        float tabY = sbMin.y + kLogoH + 14.f;
        constexpr float kTabGap = 3.f;
        bool sectionChanged = false;

        for (int i = 0; i < icon::kTabCount; i++) {
            const bool active = (i == section);
            const char* iconChar = icon::tabIconChars[i];
            const char* label = icon::tabLabels[i];

            ImVec2 tMin(sbMin.x + 10.f, tabY);
            ImVec2 tMax(sbMax.x - 10.f, tabY + kTabH);

            // Invisible button for interaction
            ImGui::SetCursorScreenPos(tMin);
            ImGui::PushID(i);
            ImGui::InvisibleButton("##sbtab", { kSideW - 20.f, kTabH });
            const bool clicked = ImGui::IsItemClicked();
            const bool hov = ImGui::IsItemHovered();
            const ImGuiID id = ImGui::GetItemID();
            ImGui::PopID();

            if (clicked && i != section) {
                section = i;
                sectionChanged = true;
            }

            // Animation
            const float at = anim::g_anim.get_toggle(id, active);
            const float ht = anim::g_anim.get_hover(id, 10.f);

            // Tab background - glass-like
            ImU32 tabBg;
            if (active) {
                tabBg = IM_COL32(220, 60, 70, (int)(30.f + at * 30.f));
                // Active indicator (left bar)
                dl->AddRectFilled(tMin, tMin + ImVec2(3.f, kTabH),
                    IM_COL32(230, 60, 70, (int)(160 + at * 95)), 2.f);
                // Subtle background fill
                dl->AddRectFilled(tMin, tMax,
                    IM_COL32(230, 60, 70, (int)(16 + at * 18)), 8.f);
            } else {
                tabBg = IM_COL32(26, 28, 34, (int)(90 + ht * 60));
                dl->AddRectFilled(tMin, tMax, tabBg, 8.f);
            }

            // Border on hover/active
            if (ht > 0.05f || active) {
                int brdAlpha = active ? 100 : (int)(ht * 50);
                dl->AddRect(tMin, tMax,
                    IM_COL32(220, 60, 70, brdAlpha), 8.f, 0, 1.f);
            }

            // Icon
            ImFont* tabF = font::bold();
            float iconS = 16.f;
            ImU32 iconCol = active
                ? IM_COL32(240, 80, 90, 230)
                : IM_COL32(140, 150, 170, (int)(120 + ht * 80));
            dl->AddText(tabF, iconS,
                tMin + ImVec2(14.f, (kTabH - iconS) * 0.5f + 1.f),
                iconCol, iconChar);

            // Label
            float labelS = 13.f;
            ImU32 textCol = active
                ? IM_COL32(220, 230, 245, 240)
                : IM_COL32(160, 170, 190, (int)(160 + ht * 60));
            dl->AddText(tabF, labelS,
                tMin + ImVec2(44.f, (kTabH - labelS) * 0.5f),
                textCol, label);

            tabY += kTabH + kTabGap;
        }

        // ---- Glassmorphism overlay effect (subtle sheen) ----
        dl->AddRectFilledMultiColor(sbMin, sbMin + ImVec2(kSideW, 1.f),
            IM_COL32(255, 255, 255, 22), IM_COL32(255, 255, 255, 6),
            IM_COL32(255, 255, 255, 6), IM_COL32(255, 255, 255, 22));

        return sectionChanged;
    }

    // ========================================================================
    // Content area background
    // ========================================================================
    static void contentBg(ImDrawList* dl, ImVec2 wp, ImVec2 ws) {
        constexpr float kSideW = 190.f;
        const ImVec2 cMin = wp + ImVec2(kSideW, 0.f);
        const ImVec2 cMax = wp + ws;

        // Dark background — pure black, no purple tint
        dl->AddRectFilled(cMin, cMax,
            IM_COL32(1, 1, 2, 248), theme::r_window,
            ImDrawFlags_RoundCornersRight);

        // Subtle gradient at left edge
        dl->AddRectFilledMultiColor(cMin, cMin + ImVec2(1.f, ws.y),
            IM_COL32(180, 190, 210, 18), IM_COL32(180, 190, 210, 0),
            IM_COL32(180, 190, 210, 0), IM_COL32(180, 190, 210, 18));
    }

    // ========================================================================
    // Menu backdrop — dark overlay
    // ========================================================================
    static void backdrop(ImDrawList* dl, ImVec2 menuMin, ImVec2 menuMax,
        float menuT, float r) {

        const ImVec2 display = ImGui::GetIO().DisplaySize;
        const float bt = anim::ease_in_out_cubic(menuT);

        // Dark overlay
        dl->AddRectFilled({ 0.f, 0.f }, display,
            IM_COL32(2, 3, 6, (int)(bt * 42.f)));

        // Shadow rings
        for (int i = 0; i < 5; i++) {
            const float spread = 16.f + i * 10.f;
            const int alpha = (int)((14.f - i * 2.f) * bt);
            dl->AddRectFilled(menuMin - ImVec2(spread, spread * 0.5f),
                menuMax + ImVec2(spread, spread),
                IM_COL32(4, 6, 10, alpha), r + spread);
        }

        // Outer glow (blood red tint)
        dl->AddRectFilledMultiColorRounded(menuMin - ImVec2(20.f, 12.f),
            menuMax + ImVec2(20.f, 20.f),
            IM_COL32(230, 60, 70, (int)(bt * 5.f)),
            IM_COL32(120, 30, 40, (int)(bt * 4.f)),
            IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), r + 20.f);
    }

} // namespace layout
