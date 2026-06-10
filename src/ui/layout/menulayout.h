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
    // Sidebar — pure black with red accents, refined tabs (F2.4)
    // ========================================================================
    static bool sidebar(ImDrawList* dl, ImVec2 wp, ImVec2 ws,
        int& section, float menuEase) {

        constexpr float kSideW = 190.f;
        constexpr float kTabH = 44.f;    // was 48
        constexpr float kTabGap = 4.f;   // was 3
        constexpr float kLogoH = 78.f;

        // ---- Sidebar background (pure black with red edge) ----
        ImVec2 sbMin = wp;
        ImVec2 sbMax = wp + ImVec2(kSideW, ws.y);

        // Shadow
        for (int i = 0; i < 2; i++) {  // was 4
            float spread = 6.f + i * 5.f;
            dl->AddRectFilled(sbMin - ImVec2(spread * 0.3f, spread * 0.1f),
                sbMax + ImVec2(spread * 0.4f, spread * 0.5f),
                IM_COL32(0, 0, 0, (int)((22.f - i * 3.5f))), theme::r_window + spread);
        }

        // Main pure black bg
        dl->AddRectFilled(sbMin, sbMax,
            IM_COL32(0, 0, 0, 240), theme::r_window, ImDrawFlags_RoundCornersLeft);

        // Red accent line on the right edge
        dl->AddRectFilled(sbMax - ImVec2(2.f, 0.f), sbMax,
            IM_COL32(220, 60, 70, 90), 0.f);

        // Bottom accent line (red)
        dl->AddRectFilled(sbMin + ImVec2(16.f, kLogoH),
            sbMin + ImVec2(kSideW - 16.f, kLogoH + 1.f),
            IM_COL32(220, 60, 70, 80));

        // ---- Logo with version inline (F2.4) ----
        if (g_sidebar_logo) {
            float logoAreaH = kLogoH - 22.f;
            float aspect = (float)g_sidebar_logoW / (float)g_sidebar_logoH;
            float logoH = ImMin(logoAreaH, 44.f);
            float logoW = logoH * aspect;
            float maxLogoW = kSideW - 40.f;
            if (logoW > maxLogoW) { logoW = maxLogoW; logoH = logoW / aspect; }
            ImVec2 logoMin = sbMin + ImVec2(18.f, (kLogoH - logoH) * 0.5f);
            dl->AddImage(g_sidebar_logo, logoMin, logoMin + ImVec2(logoW, logoH));

            // Version next to logo, baseline-aligned, muted 50%
            dl->AddText(font::mono(), font::mono()->LegacySize * 0.85f,
                logoMin + ImVec2(logoW + 6.f, logoH * 0.6f),
                IM_COL32(140, 150, 170, 80), "v1.0.0");
        } else {
            // Fallback: text logo
            ImFont* logoF = font::logo();
            float logoS = logoF->LegacySize * 0.9f;
            ImVec2 logoPos = sbMin + ImVec2(18.f, 22.f);
            dl->AddText(logoF, logoS, logoPos + ImVec2(1.f, 1.f),
                IM_COL32(255, 40, 50, 180), "MISERABLE");
            dl->AddText(logoF, logoS, logoPos,
                IM_COL32(230, 60, 70, 245), "MISERABLE");

            // Version next to logo
            ImVec2 nameSize = logoF->CalcTextSizeA(logoS, FLT_MAX, 0.f, "MISERABLE");
            dl->AddText(font::mono(), font::mono()->LegacySize * 0.85f,
                logoPos + ImVec2(nameSize.x + 8.f, logoS * 0.3f),
                IM_COL32(140, 150, 170, 80), "v1.0.0");
        }

        // ---- Animated tab indicator (F2.3) ----
        static float indicatorY = kLogoH + 14.f;
        float targetY = (kLogoH + 14.f) + section * (kTabH + kTabGap);
        indicatorY = anim::damp(indicatorY, targetY, 14.f);

        // Draw only ONE animated indicator bar (not per-tab)
        dl->AddRectFilled(
            sbMin + ImVec2(0.f, indicatorY),
            sbMin + ImVec2(3.f, indicatorY + kTabH),
            IM_COL32(230, 60, 70, 220), 2.f);

        // ---- Tab buttons ----
        float tabY = sbMin.y + kLogoH + 14.f;
        bool sectionChanged = false;

        for (int i = 0; i < icon::kTabCount; i++) {
            const bool active = (i == section);
            const char* iconChar = icon::tabIconChars[i];
            const char* label = icon::tabLabels[i];

            ImVec2 tMin(sbMin.x + 12.f, tabY);
            ImVec2 tMax(sbMax.x - 12.f, tabY + kTabH);

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

            if (active) {
                // Active state: horizontal gradient red alpha 36->0 (F2.4)
                dl->AddRectFilledMultiColor(tMin, tMax,
                    IM_COL32(230, 60, 70, (int)(24 + at * 12)),
                    IM_COL32(230, 60, 70, (int)(8 + at * 8)),
                    IM_COL32(230, 60, 70, 0),
                    IM_COL32(230, 60, 70, 0));
            } else if (hov) {
                // Hover: white translucent bg (F2.4)
                int ha = (int)(10 + ht * 8);
                dl->AddRectFilled(tMin, tMax,
                    IM_COL32(255, 255, 255, ha), 8.f);
            }

            // Border on hover/active
            if (ht > 0.05f || active) {
                int brdAlpha = active ? 100 : (int)(ht * 50);
                dl->AddRect(tMin, tMax,
                    IM_COL32(220, 60, 70, brdAlpha), 8.f, 0, 1.f);
            }

            // Icon — centered in 20x20 box at x=14 (F2.4)
            ImFont* tabF = font::bold();
            float iconS = 16.f;
            ImU32 iconCol = active
                ? IM_COL32(240, 80, 90, 230)
                : IM_COL32(140, 150, 170, (int)(120 + ht * 80));
            dl->AddText(tabF, iconS,
                tMin + ImVec2(14.f, (kTabH - iconS) * 0.5f + 1.f),
                iconCol, iconChar);

            // Label at x=46 (F2.4)
            float labelS = 13.f;
            ImU32 textCol = active
                ? IM_COL32(220, 230, 245, 240)
                : IM_COL32(160, 170, 190, (int)(160 + ht * 60));
            dl->AddText(tabF, labelS,
                tMin + ImVec2(46.f, (kTabH - labelS) * 0.5f),
                textCol, label);

            tabY += kTabH + kTabGap;
        }

        // ---- User footer (F2.4) ----
        float footerY = sbMax.y - 50.f;
        // Hairline separator
        dl->AddRectFilled(sbMin + ImVec2(16.f, footerY),
            sbMax + ImVec2(-16.f, footerY + 1.f),
            IM_COL32(255, 255, 255, 10));

        // Green status dot
        dl->AddCircleFilled(sbMin + ImVec2(24.f, footerY + 24.f),
            4.f, IM_COL32(80, 255, 120, 220), 12);
        dl->AddCircle(sbMin + ImVec2(24.f, footerY + 24.f),
            5.5f, IM_COL32(80, 255, 120, 60), 12, 1.f);

        // Username
        static const char* username = []() {
            static char buf[128];
            DWORD len = GetEnvironmentVariableA("USERNAME", buf, sizeof(buf));
            return (len && len < sizeof(buf)) ? buf : "user";
        }();
        dl->AddText(font::regular(), font::regular()->LegacySize,
            sbMin + ImVec2(38.f, footerY + 14.f),
            IM_COL32(140, 150, 170, 160), username);

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
    // Menu backdrop — dark overlay (2-ring shadow, was 5) (F1.4)
    // ========================================================================
    static void backdrop(ImDrawList* dl, ImVec2 menuMin, ImVec2 menuMax,
        float menuT, float r) {

        const ImVec2 display = ImGui::GetIO().DisplaySize;
        const float bt = anim::ease_in_out_cubic(menuT);

        // Dark overlay
        dl->AddRectFilled({ 0.f, 0.f }, display,
            IM_COL32(2, 3, 6, (int)(bt * 42.f)));

        // Shadow rings — 2 instead of 5
        for (int i = 0; i < 2; i++) {
            const float spread = 16.f + i * 10.f;
            const int alpha = (int)((14.f - i * 2.f) * bt);
            dl->AddRectFilled(menuMin - ImVec2(spread, spread * 0.5f),
                menuMax + ImVec2(spread, spread),
                IM_COL32(4, 6, 10, alpha), r + spread);
        }

        // Outer glow (blood red tint) — only 1 layer
        dl->AddRectFilledMultiColorRounded(menuMin - ImVec2(20.f, 12.f),
            menuMax + ImVec2(20.f, 20.f),
            IM_COL32(230, 60, 70, (int)(bt * 5.f)),
            IM_COL32(120, 30, 40, (int)(bt * 4.f)),
            IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), r + 20.f);
    }

} // namespace layout
