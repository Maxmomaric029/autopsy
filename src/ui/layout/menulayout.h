#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <d3d11.h>
#include "../core/theme.h"
#include "../core/animation.h"
#include "../core/fonts.h"
#include "../core/icons.h"
#include "../core/sound.h"
#include "../core/avatar.h"

#include "../modern_ui.h"

// ========================================================================
// Menu layout: 64px sidebar + 52px topbar + content area
// Per the spec:
// ┌──────────────────────────────────────────────────────┐
// │ [64px sidebar] │ [topbar 52px]                      │
// │                ├─────────────────────────────────────┤
// │   icon tabs    │ [left panels] │ [right panel 240px] │
// │                │               │                     │
// │   ──────────   │               │                     │
// │   [settings]   │               │                     │
// └──────────────────────────────────────────────────────┘
// ========================================================================

// Extern for frame content flag (defined in graphic.cpp)
extern bool g_frameHadContent;

namespace layout {

    // ========================================================================
    // Sidebar — 64px wide, pure black, lime-green accent tabs
    // ========================================================================
    static bool sidebar(ImDrawList* dl, ImVec2 wp, ImVec2 ws,
        int& section, float menuEase) {

        constexpr float kSideW = theme::kSidebarW;       // 64
        constexpr float kTabH  = theme::kSidebarTabH;    // 40
        constexpr float kTabGap = theme::kSidebarTabGap; // 6

        ImVec2 sbMin = wp;
        ImVec2 sbMax = wp + ImVec2(kSideW, ws.y);

        // ---- Sidebar background: SURFACE, border-right 1px BORDER ----
        dl->AddRectFilled(sbMin, sbMax,
            IM_COL32(17, 17, 20, 255), 0.f); // Surface

        // Right border
        dl->AddRectFilled(sbMax - ImVec2(1.f, 0.f), sbMax,
            IM_COL32(255, 255, 255, 15), 0.f);

        // ---- Top shimmer ----
        dl->AddRectFilledMultiColor(sbMin, sbMin + ImVec2(kSideW, 1.f),
            IM_COL32(255, 255, 255, 22), IM_COL32(255, 255, 255, 6),
            IM_COL32(255, 255, 255, 6), IM_COL32(255, 255, 255, 22));

        // Logo: cuervo image si está cargado, fallback skull icon
        const float logoY = sbMin.y + 20.f;
        const float logoX = sbMin.x + (kSideW - theme::kLogoSize) * 0.5f;

        if (g_sidebar_logo) {
            dl->AddRectFilled(
                { logoX, logoY },
                { logoX + theme::kLogoSize, logoY + theme::kLogoSize },
                theme::col_accent(0.10f), 8.f);
            dl->AddImageRounded(g_sidebar_logo,
                { logoX + 2.f, logoY + 2.f },
                { logoX + theme::kLogoSize - 2.f, logoY + theme::kLogoSize - 2.f },
                ImVec2(0, 0), ImVec2(1, 1),
                IM_COL32(255, 255, 255, 215), 6.f);
        } else {
            dl->AddRectFilled(
                { logoX, logoY },
                { logoX + theme::kLogoSize, logoY + theme::kLogoSize },
                theme::col_accent(), 8.f);
            ImGui::PushFont(font::label());
            const char* sk = ICON_FA_SKULL;
            ImVec2 skSz = font::label()->CalcTextSizeA(15.f, FLT_MAX, 0.f, sk);
            dl->AddText(font::label(), 15.f,
                { logoX + (theme::kLogoSize - skSz.x) * 0.5f,
                  logoY + (theme::kLogoSize - skSz.y) * 0.5f },
                IM_COL32(0, 0, 0, 220), sk);
            ImGui::PopFont();
        }

        // ---- Animated tab indicator ----
        static float indicatorY = logoY + theme::kLogoSize + 20.f;
        float targetY = (logoY + theme::kLogoSize + 20.f) + section * (kTabH + kTabGap);
        indicatorY = anim::damp(indicatorY, targetY, anim::speed::sidebar_tab);

        // Active indicator: 3px wide, glow layer behind, ACCENT
        dl->AddRectFilled(
            sbMin + ImVec2(0.f, indicatorY),
            sbMin + ImVec2(8.f, indicatorY + kTabH),
            IM_COL32(224, 48, 64, 28)); // glow spread
        dl->AddRectFilled(
            sbMin + ImVec2(0.f, indicatorY + 4.f),
            sbMin + ImVec2(3.f, indicatorY + kTabH - 4.f),
            theme::col_accent(), 2.f);

        // ---- Tab buttons ----
        float tabY = logoY + theme::kLogoSize + 20.f;
        bool sectionChanged = false;
        bool anyClicked = false;

        for (int i = 0; i < icon::kTabCount; i++) {
            const bool active = (i == section);
            const char* iconChar = icon::tabIconChars[i];

            ImVec2 tMin(sbMin.x, tabY);
            ImVec2 tMax(sbMax.x, tabY + kTabH);

            // Invisible button
            ImGui::SetCursorScreenPos(tMin);
            ImGui::PushID(i);
            ImGui::InvisibleButton("##sbtab", { kSideW, kTabH });
            const bool clicked = ImGui::IsItemClicked();
            const bool hov = ImGui::IsItemHovered();
            const ImGuiID id = ImGui::GetItemID();
            ImGui::PopID();

            if (clicked && i != section) {
                section = i;
                sectionChanged = true;
                anyClicked = true;
                sound::play(sound::id::tab_switch);
            }

            // Animation
            const float at = anim::g_anim.get_toggle(id, active);
            const float ht = anim::g_anim.get_hover(id, 10.f);

            if (active) {
                // Active: ACCENT_DIM background
                dl->AddRectFilled(tMin, tMax,
                    IM_COL32(224, 48, 64, (int)(255 * 0.12f)), 8.f);
            } else if (hov) {
                // Hover: SURFACE2 background
                int ha = (int)(10 + ht * 8);
                dl->AddRectFilled(tMin, tMax,
                    IM_COL32(23, 23, 27, ha), 8.f);
            }

            // Border on hover/active
            if (ht > 0.05f || active) {
                int brdAlpha = active ? 60 : (int)(ht * 30);
                dl->AddRect(tMin, tMax,
                    IM_COL32(224, 48, 64, brdAlpha), 8.f, 0, 1.f);
            }

            // Icon — centered in the 64px sidebar
            ImFont* tabF = font::label();
            float iconS = font::size::tab_icon;
            ImU32 iconCol = active
                ? IM_COL32(224, 48, 64, 255)
                : IM_COL32(90, 90, 96, (int)(150 + ht * 80));
            dl->AddText(tabF, iconS,
                tMin + ImVec2((kSideW - iconS) * 0.5f, (kTabH - iconS) * 0.5f + 1.f),
                iconCol, iconChar);

            tabY += kTabH + kTabGap;
        }

        // ---- Footer: separator + avatar pill ----
        float footerY = sbMax.y - 56.f;
        dl->AddRectFilled(sbMin + ImVec2(14.f, footerY),
            sbMax + ImVec2(-14.f, footerY + 1.f),
            IM_COL32(255, 255, 255, 10));

        float avatarY = footerY + 14.f;
        float avatarX = sbMin.x + (kSideW - 28.f) * 0.5f;

        ID3D11ShaderResourceView* avatarSrv = avatar::get_srv();
        if (avatarSrv && avatar::is_loaded()) {
            // Real avatar image (circular via AddImageRounded)
            dl->AddImageRounded(avatarSrv,
                { avatarX, avatarY },
                { avatarX + 28.f, avatarY + 28.f },
                ImVec2(0, 0), ImVec2(1, 1),
                IM_COL32(255, 255, 255, 255), 14.f);
            dl->AddCircle({ avatarX + 14.f, avatarY + 14.f },
            14.f, IM_COL32(255, 255, 255, 15), 24, 1.f);
                } else {
                dl->AddCircleFilled({ avatarX + 14.f, avatarY + 14.f },
                14.f, IM_COL32(23, 23, 27, 255), 24);
            dl->AddCircle({ avatarX + 14.f, avatarY + 14.f },
                14.f, IM_COL32(255, 255, 255, 15), 24, 1.f);

                static const char* username = []() {
                    static char buf[128];
                DWORD len = GetEnvironmentVariableA("USERNAME", buf, sizeof(buf));
                return (len && len < sizeof(buf)) ? buf : "U";
            }();
                char initial[2] = { username[0], '\0' };
                ImVec2 initSize = ImGui::CalcTextSize(initial);
                dl->AddText(font::body(), 12.f,
                    { avatarX + 14.f - initSize.x * 0.5f,
                  avatarY + 14.f - initSize.y * 0.5f },
              theme::col_accent(), initial);
            }

        // Status dot
        float dotY = avatarY + 20.f;
        float dotX = avatarX + 20.f;
        dl->AddCircleFilled({ dotX, dotY }, 3.5f,
            IM_COL32(80, 255, 120, 220), 12);
        dl->AddCircle({ dotX, dotY }, 5.f,
            IM_COL32(80, 255, 120, 60), 12, 1.f);

        return sectionChanged;
    }

    // ========================================================================
    // Topbar — 52px, shows section name + status
    // ========================================================================
    static void topbar(ImDrawList* dl, ImVec2 wp, ImVec2 ws, int section, float ease) {
        constexpr float kSideW = theme::kSidebarW;
        constexpr float kTopH  = theme::kTopbarH;

        ImVec2 tbMin = wp + ImVec2(kSideW, 0.f);
        ImVec2 tbMax = wp + ImVec2(ws.x, kTopH);

        // Background
        dl->AddRectFilled(tbMin, tbMax,
            IM_COL32(10, 10, 11, 255), 0.f); // slightly lighter than bg

        // Bottom border
        dl->AddRectFilled(tbMin + ImVec2(0.f, kTopH - 1.f), tbMax,
            IM_COL32(255, 255, 255, 15), 0.f);

        // Section name
        ImGui::PushFont(font::label());
        const float labelSize = 13.f;
        const char* sectionName = icon::tabLabels[section];
        dl->AddText(font::label(), labelSize,
            tbMin + ImVec2(20.f, (kTopH - labelSize) * 0.5f),
            theme::col_text(), sectionName);
        ImGui::PopFont();

        // Badge pill
        const char* badge = icon::tabBadges[section];
        float badgeW = ImGui::CalcTextSize(badge).x + 12.f;
        float badgeH = 18.f;
        float badgeX = tbMin.x + 20.f + ImGui::CalcTextSize(sectionName).x + 10.f;
        float badgeY = tbMin.y + (kTopH - badgeH) * 0.5f;
        dl->AddRectFilled({ badgeX, badgeY }, { badgeX + badgeW, badgeY + badgeH },
            IM_COL32(224, 48, 64, (int)(255 * 0.14f)), 5.f);
        dl->AddRect({ badgeX, badgeY }, { badgeX + badgeW, badgeY + badgeH },
            IM_COL32(224, 48, 64, (int)(255 * 0.20f)), 5.f, 0, 1.f);
        ImGui::PushFont(font::mono());
        dl->AddText(font::mono(), 10.f,
            { badgeX + (badgeW - ImGui::CalcTextSize(badge).x) * 0.5f,
              badgeY + (badgeH - 10.f) * 0.5f },
            theme::col_accent(), badge);
        ImGui::PopFont();

        // FPS pill (right side) — cached, no duplicate calc
        {
            static double lastT = 0.0; static int fc = 0; static float fps = 0.f;
            double now2 = ImGui::GetTime(); ++fc;
            if (now2 - lastT >= 0.5) { fps = (float)(fc / (now2 - lastT)); fc = 0; lastT = now2; }
            char fpsBuf[16]; std::snprintf(fpsBuf, sizeof(fpsBuf), "%.0f fps", fps);
            float fpsTW = ImGui::CalcTextSize(fpsBuf).x;
            float fpsPW = fpsTW + 14.f, fpsPH = 18.f;
            float fpsPX = tbMax.x - fpsPW - 16.f;
            float fpsPY = tbMin.y + (kTopH - fpsPH) * 0.5f;
            dl->AddRectFilled({ fpsPX, fpsPY }, { fpsPX + fpsPW, fpsPY + fpsPH },
                IM_COL32(20, 8, 8, 180), 5.f);
            dl->AddRect({ fpsPX, fpsPY }, { fpsPX + fpsPW, fpsPY + fpsPH },
                IM_COL32(224, 48, 64, 40), 5.f, 0, 1.f);
            ImGui::PushFont(font::mono());
            dl->AddText(font::mono(), 10.f,
                { fpsPX + (fpsPW - fpsTW) * 0.5f, fpsPY + (fpsPH - 10.f) * 0.5f },
                IM_COL32(160, 140, 140, 200), fpsBuf);
            ImGui::PopFont();
        }
    }

    // ========================================================================
    // Content area background
    // ========================================================================
    static void contentBg(ImDrawList* dl, ImVec2 wp, ImVec2 ws) {
        constexpr float kSideW = theme::kSidebarW;
        constexpr float kTopH  = theme::kTopbarH;
        const ImVec2 cMin = wp + ImVec2(kSideW, kTopH);
        const ImVec2 cMax = wp + ws;

        dl->AddRectFilled(cMin, cMax,
            IM_COL32(10, 10, 11, 255), 0.f);

        // Subtle gradient at left edge of content
        dl->AddRectFilledMultiColor(cMin, cMin + ImVec2(1.f, ws.y - kTopH),
            IM_COL32(255, 255, 255, 8), IM_COL32(255, 255, 255, 0),
            IM_COL32(255, 255, 255, 0), IM_COL32(255, 255, 255, 8));
    }

} // namespace layout
