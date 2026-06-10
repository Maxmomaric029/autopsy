#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <deque>
#include <string>
#include <functional>
#include <chrono>
#include "theme.h"
#include "animation.h"
#include "fonts.h"

// ========================================================================
// Toast notification system
// ========================================================================

namespace notify {

    enum class Type { Success, Error, Warning, Info };

    struct Toast {
        std::string text;
        Type type;
        double startTime;
        double duration;
        float animT{}; // 0→1 during slide-in, then back to 0 during fade-out
        bool dying{ false };
    };

    inline std::deque<Toast> g_queue;

    // ========================================================================
    // Push a new toast
    // ========================================================================
    inline void push(const std::string& text, Type type = Type::Info, double duration = 3.5) {
        if (g_queue.size() >= 5)
            g_queue.pop_front();
        g_queue.push_back({ text, type, ImGui::GetTime(), duration, 0.f, false });
    }

    inline void success(const std::string& text) { push(text, Type::Success); }
    inline void error(const std::string& text)   { push(text, Type::Error); }
    inline void warn(const std::string& text)    { push(text, Type::Warning); }
    inline void info(const std::string& text)    { push(text, Type::Info); }

    // ========================================================================
    // Render all active toasts
    // ========================================================================
    inline void render() {
        const double now = ImGui::GetTime();
        const float displayW = ImGui::GetIO().DisplaySize.x;
        constexpr float kItemW = 320.f;
        constexpr float kItemH = 50.f;
        constexpr float kGap = 8.f;
        constexpr float kSlideIn = 0.35f;
        constexpr float kFadeOut = 0.5f;

        // Remove expired
        while (!g_queue.empty()) {
            auto& t = g_queue.front();
            const double elapsed = now - t.startTime;
            if (elapsed >= t.duration + kFadeOut) {
                g_queue.pop_front();
            } else break;
        }

        if (g_queue.empty()) return;

        ImDrawList* dl = ImGui::GetForegroundDrawList();
        float y = ImGui::GetIO().DisplaySize.y - 80.f;

        // Render from back to front (newest on top, drawn last)
        for (int i = (int)g_queue.size() - 1; i >= 0; i--) {
            auto& t = g_queue[i];
            const double elapsed = now - t.startTime;

            // Stack: oldest at bottom, newest on top (F2.5 fix)
            const float offsetY = (float)i * (kItemH + kGap);

            float alpha = 1.f;
            float slideX = 0.f;

            if (elapsed < kSlideIn) {
                // Slide in
                const float p = (float)(elapsed / kSlideIn);
                slideX = (1.f - anim::ease_out_quint(p)) * (kItemW + 20.f);
                alpha = anim::ease_out_cubic(p);
            } else if (elapsed > t.duration) {
                // Fade out
                const float p = (float)((elapsed - t.duration) / kFadeOut);
                alpha = 1.f - anim::ease_in_out_cubic(p);
            }

            if (alpha <= 0.01f) continue;

            const float x = displayW - kItemW - 16.f - slideX;
            const ImVec2 mn(x, y - offsetY - kItemH);
            const ImVec2 mx(x + kItemW, y - offsetY);

            // Determine colors based on type
            ImU32 bg, border, accent;
            switch (t.type) {
            case Type::Success:
                accent = theme::col_success();
                break;
            case Type::Error:
                accent = theme::col_danger();
                break;
            case Type::Warning:
                accent = theme::col_warn();
                break;
            default:
                accent = theme::col_accent();
                break;
            }
            bg = IM_COL32(6, 11, 20, (int)(235 * alpha));
            border = IM_COL32(20, 40, 60, (int)(140 * alpha));

            // Shadow
            for (int s = 0; s < 4; s++) {
                const float sp = 4.f + s * 4.f;
                dl->AddRectFilled(mn - ImVec2(sp * .4f, sp * .2f) + ImVec2(0.f, 3.f + s),
                    mx + ImVec2(sp * .45f, sp * .65f),
                    IM_COL32(0, 0, 0, (int)((20.f - s * 3.5f) * alpha)), 10.f + sp);
            }

            // Background
            dl->AddRectFilled(mn, mx, bg, 10.f);

            // Accent bar (left edge)
            dl->AddRectFilled(mn + ImVec2(3.f, 5.f), mn + ImVec2(6.f, kItemH - 5.f), accent, 2.f);

            // Top border
            dl->AddRectFilledMultiColor(mn, ImVec2(mx.x, mn.y + 1.f),
                theme::alpha(accent, 0.3f * alpha), theme::alpha(accent, 0.05f * alpha),
                theme::alpha(accent, 0.05f * alpha), theme::alpha(accent, 0.3f * alpha));

            dl->AddRect(mn, mx, border, 10.f, 0, 1.f);

            // Text
            dl->AddText(font::medium(), font::medium()->LegacySize,
                mn + ImVec2(18.f, 8.f), theme::col_text(alpha), t.text.c_str());
        }
    }

} // namespace notify
