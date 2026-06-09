#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include "../core/theme.h"
#include "../core/animation.h"
#include "../core/fonts.h"
#include "../core/icons.h"

// ========================================================================
// Menu layout: sidebar tabs, header, and backdrop rendering
// ========================================================================

namespace layout {

    // ---- Sidebar Tab button --------------------------------------------
    static bool tab(ImDrawList* dl, ImVec2 wp, float startY, float sbW, bool compact,
        int idx, int cur, icon::Fn iconFn, const char* label) {

        const bool   active = (idx == cur);
        const float tabH = theme::kTabH - 6.f;
        const float tabPad = compact ? 4.f : 14.f;
        const float tabW = sbW - (tabPad * 2.f);
        const float tabX = wp.x + tabPad;
        const float tabY = wp.y + startY + idx * theme::kTabH;

        const ImVec2 tMin = { tabX, tabY };
        const ImVec2 tMax = tMin + ImVec2(tabW, tabH);
        const ImVec2 center = compact
            ? ImVec2(tMin.x + tabW * .5f, tMin.y + tabH * .5f)
            : ImVec2(tMin.x + 18.f, tMin.y + tabH * .5f);

        ImGui::SetCursorScreenPos(tMin);
        ImGui::PushID(idx);
        ImGui::InvisibleButton("##tab", { tabW, tabH });

        const bool clicked = ImGui::IsItemClicked();
        const bool hov = ImGui::IsItemHovered();
        const ImGuiID id = ImGui::GetItemID();
        ImGui::PopID();

        const float t = anim::ease_out_cubic(anim::g_anim.get(id, active || hov, 9.f));
        const float at = anim::ease_out_cubic(anim::g_anim.get(id ^ 0x8A13C4u, active, 10.f));

        const ImU32 hoverFill = theme::lerp_u32(IM_COL32(0, 0, 0, 0),
            IM_COL32(11, 28, 42, 126), t);
        const ImU32 activeFill = IM_COL32(0, 78, 116, 178);
        dl->AddRectFilled(tMin, tMax, theme::lerp_u32(hoverFill, activeFill, at), 9.f);

        if (at > .01f) {
            dl->AddRectFilled(tMin + ImVec2(1.f, 1.f), tMax - ImVec2(1.f, 1.f),
                IM_COL32(0, 174, 255, (int)(at * 12.f)), 8.f);
            dl->AddRect(tMin, tMax, IM_COL32(0, 174, 255, (int)(at * 96.f)), 9.f, 0, 1.f);
        }

        const ImU32 baseCol = hov ? theme::col_text() : theme::col_muted();
        ImU32 iconCol = theme::lerp_u32(baseCol, theme::alpha(theme::col_accent(), 0.85f), at);
        iconFn(dl, center + ImVec2(at * 1.5f, 0.f), 18.f + at, iconCol);
        if (!compact)
            dl->AddText({ tMin.x + 42.f, tMin.y + (tabH - ImGui::GetFontSize()) * .5f },
                iconCol, label);

        return clicked;
    }

    // ---- Sidebar --------------------------------------------------------
    static void sidebar(ImDrawList* dl, ImVec2 wp, ImVec2 ws, bool compact,
        int& section, const char* userName) {

        const float sbW = compact ? theme::kSidebarCompactW : theme::kSidebarW;

        // Sidebar background
        const ImVec2 sideMin = wp + ImVec2(1.f, 1.f);
        const ImVec2 sideMax = { wp.x + sbW, wp.y + ws.y - 1.f };
        dl->AddRectFilled(sideMin, sideMax, IM_COL32(4, 10, 17, 118), theme::r_window,
            ImDrawFlags_RoundCornersLeft);

        // Subtle gradient overlay
        dl->AddRectFilledMultiColorRounded(sideMin, sideMax,
            IM_COL32(255, 255, 255, 10), IM_COL32(255, 255, 255, 2),
            IM_COL32(0, 0, 0, 68), IM_COL32(0, 174, 255, 12),
            theme::r_window, ImDrawFlags_RoundCornersLeft);

        // Logo area gradient
        dl->AddRectFilledMultiColorRounded(sideMin + ImVec2(1.f, 1.f),
            ImVec2(sideMax.x - 1.f, sideMin.y + 92.f),
            IM_COL32(255, 255, 255, 8), IM_COL32(0, 174, 255, 5),
            IM_COL32(255, 255, 255, 1), IM_COL32(0, 0, 0, 0),
            theme::r_window - 1.f, ImDrawFlags_RoundCornersTopLeft);

        // Sidebar border
        dl->AddRect(sideMin, sideMax, IM_COL32(255, 255, 255, 18),
            theme::r_window, ImDrawFlags_RoundCornersLeft, 1.f);
        dl->AddLine({ wp.x + sbW, wp.y + 10.f },
            { wp.x + sbW, wp.y + ws.y - 10.f },
            IM_COL32(96, 210, 255, 42), 1.f);

        // Logo (non-compact)
        if (!compact) {
            const ImVec2 lp = { wp.x + 30.f, wp.y + 25.f };
            ImFont* logoF = font::logo();
            const float logoSize = logoF->FontSize;
            const ImVec2 aw = logoF->CalcTextSizeA(logoSize, FLT_MAX, 0.f, "AUTOPSY");
            dl->AddRectFilled(lp + ImVec2(-8.f, 3.f), lp + ImVec2(-5.f, 25.f),
                theme::col_accent(), 2.f);
            dl->AddText(logoF, logoSize, lp + ImVec2(1.f, 2.f),
                IM_COL32(0, 0, 0, 190), "AUTOPSY");
            dl->AddText(logoF, logoSize, lp, theme::col_text(), "AUTOPSY");
            dl->AddText(lp + ImVec2(aw.x + 7.f, 6.f),
                theme::col_accent(), "BETA");

            // Divider
            dl->AddRectFilled({ wp.x + 20.f, wp.y + theme::kLogoH - 13.f },
                { wp.x + sbW - 20.f, wp.y + theme::kLogoH - 12.f },
                theme::col_border(), 1.f);
        }

        // Section label
        if (!compact)
            dl->AddText({ wp.x + 20.f, wp.y + theme::kLogoH + 7.f },
                theme::col_muted(), "MAIN");

        // Tabs
        const float tabY = compact ? 14.f : theme::kLogoH + 28.f;
        for (int i = 0; i < icon::kTabCount; i++)
            if (tab(dl, wp, tabY, sbW, compact, i, section,
                icon::tabIcons[i], icon::tabLabels[i]))
                section = i;

        // Footer (non-compact)
        if (!compact) {
            dl->AddRectFilled({ wp.x + 20.f, wp.y + ws.y - 66.f },
                { wp.x + sbW - 20.f, wp.y + ws.y - 65.f },
                theme::col_border(), 1.f);
            dl->AddText({ wp.x + 20.f, wp.y + ws.y - 49.f },
                theme::col_muted(), "Signed in");
            dl->AddText({ wp.x + 20.f, wp.y + ws.y - 30.f },
                theme::col_text(), userName);
        }
    }

    // ---- Header ---------------------------------------------------------
    static void header(ImDrawList* dl, ImVec2 wp, ImVec2 ws, float sbW,
        int section, int prevSection, float headerEase, float headerAlpha,
        bool compact) {

        const float kHeaderH = compact ? 48.f : theme::kHeaderH;
        const float contentX = wp.x + sbW + 1.f;
        const float contentW = ws.x - sbW - 1.f;

        // Header background
        dl->AddRectFilled({ contentX, wp.y + 1.f },
            { contentX + contentW - 1.f, wp.y + kHeaderH },
            theme::col_surface(), theme::r_window, ImDrawFlags_RoundCornersTopRight);
        dl->AddRectFilled({ contentX, wp.y + 1.f },
            { contentX + contentW - 1.f, wp.y + kHeaderH },
            IM_COL32(0, 174, 255, 5), theme::r_window, ImDrawFlags_RoundCornersTopRight);
        dl->AddLine({ contentX, wp.y + kHeaderH },
            { contentX + contentW, wp.y + kHeaderH },
            theme::col_border(), 1.f);

        // Page titles
        constexpr const char* kTabTitles[] = {
            "AIMBOT", "VISUALS", "WORLD", "MISC", "BLADE BALL", "SETTINGS"
        };
        constexpr const char* kTabSubtitles[] = {
            "combat & targeting", "esp & effects", "environment",
            "movement & exploits", "game mode", "configuration"
        };

        const ImVec2 tp = { contentX + 18.f, wp.y + 12.f };
        ImFont* titleFont = font::bold();
        const float titleSize = titleFont ? titleFont->FontSize : ImGui::GetFontSize();

        auto drawTitle = [&](int idx, float a, ImVec2 off) {
            const ImVec2 p = tp + off;
            dl->AddText(titleFont, titleSize, p + ImVec2(0.f, 1.f),
                theme::alpha(IM_COL32(0, 0, 0, 180), a), kTabTitles[idx]);
            dl->AddText(titleFont, titleSize, p,
                theme::alpha(theme::col_accent(), a), kTabTitles[idx]);
            dl->AddText({ p.x, p.y + 24.f },
                theme::alpha(theme::col_muted(), a), kTabSubtitles[idx]);
        };

        if (headerAlpha < 0.995f)
            drawTitle(prevSection, 1.f - headerAlpha,
                { -headerAlpha * 14.f, -headerAlpha * 4.f });
        drawTitle(section, headerAlpha,
            { (1.f - headerAlpha) * 18.f, (1.f - headerAlpha) * 5.f });

        // Brand
        const char* brand = "autopsy.lol";
        const float bw = ImGui::CalcTextSize(brand).x;
        const ImVec2 bp = { contentX + contentW - bw - 22.f, wp.y + 20.f };
        dl->AddText(bp, theme::col_text(), brand);
        dl->AddCircleFilled({ bp.x - 12.f, bp.y + 7.f }, 3.f,
            theme::col_accent(), 16);
    }

    // ---- Menu backdrop shadow -------------------------------------------
    static void backdrop(ImDrawList* dl, ImVec2 menuMin, ImVec2 menuMax,
        float menuT, float r) {

        // Dark overlay
        const ImVec2 display = ImGui::GetIO().DisplaySize;
        const float bt = anim::ease_in_out_cubic(menuT);
        dl->AddRectFilled({ 0.f, 0.f }, display,
            IM_COL32(2, 4, 8, (int)(bt * 18.f)));

        // Shadow rings
        for (int i = 0; i < 5; i++) {
            const float spread = 18.f + i * 11.f;
            const int alpha = (int)((12.f - i * 1.85f) * bt);
            dl->AddRectFilled(menuMin - ImVec2(spread, spread * .55f),
                menuMax + ImVec2(spread, spread),
                IM_COL32(7, 13, 21, alpha), r + spread);
        }

        // Outer glow
        dl->AddRectFilledMultiColorRounded(menuMin - ImVec2(28.f, 20.f),
            menuMax + ImVec2(28.f, 32.f),
            IM_COL32(0, 174, 255, (int)(bt * 5.f)),
            IM_COL32(100, 117, 255, (int)(bt * 5.f)),
            IM_COL32(0, 0, 0, 0),
            IM_COL32(0, 0, 0, 0), r + 26.f);
    }

    // ---- Dissolve particles effect --------------------------------------
    static void dissolve(ImDrawList* dl, ImVec2 mn, ImVec2 mx, float t, float time) {
        const float a = sinf(anim::saturate(t) * 3.14159265f);
        if (a <= .01f) return;

        const float w = mx.x - mn.x;
        const float h = mx.y - mn.y;
        for (int i = 0; i < 30; i++) {
            const float seed = (float)i * 12.9898f;
            const float u = fmodf(fabsf(sinf(seed) * 43758.5453f), 1.f);
            const float v = fmodf(fabsf(sinf(seed + 41.371f) * 24634.6345f), 1.f);
            const float drift = sinf(time * 2.1f + seed) * 5.f * (1.f - fabsf(t - .5f) * 1.55f);
            const float size = 1.0f + v * 1.8f;
            ImVec2 p;
            if ((i & 3) == 0)
                p = { mn.x + u * w, mn.y - 11.f + drift };
            else if ((i & 3) == 1)
                p = { mn.x + u * w, mx.y + 8.f + drift };
            else if ((i & 3) == 2)
                p = { mn.x - 10.f + drift, mn.y + u * h };
            else
                p = { mx.x + 8.f + drift, mn.y + u * h };

            const int alpha = (int)(a * (14.f + v * 28.f));
            const ImU32 col = (i % 5) == 0
                ? IM_COL32(100, 117, 255, alpha)
                : IM_COL32(0, 174, 255, alpha);
            dl->AddRectFilled(p, p + ImVec2(size, size), col, 1.5f);
        }
    }

} // namespace layout
