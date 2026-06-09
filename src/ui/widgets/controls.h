#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <vector>
#include <string>
#include <cstdio>
#include <cmath>
#include "../core/theme.h"
#include "../core/animation.h"
#include "../core/fonts.h"

// ========================================================================
// Custom ImGui widgets — all drawn with ImDrawList for premium Aurora look
// ========================================================================

namespace w {

    // ---- Card -----------------------------------------------------------
    struct card {
        static bool begin(const char* id, ImVec2 size, const char* title = nullptr);
        static void end();
    };

    // ---- Toggle ---------------------------------------------------------
    inline bool toggle(const char* label, bool* v) {
        ImGui::PushID(label);
        const ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        constexpr float kTW = 34.f, kTH = 18.f;
        ImGui::InvisibleButton("##t", { kTW, kTH });
        const bool clicked = ImGui::IsItemClicked();
        const bool hov = ImGui::IsItemHovered();
        if (clicked) *v = !*v;

        const float t = anim::toggle(ImGui::GetItemID(), *v);
        const ImU32 trk = theme::lerp_u32(
            theme::to_u32(theme::c_surface2),
            theme::alpha(theme::col_accent(), 0.34f), t);

        dl->AddRectFilled(p, p + ImVec2(kTW, kTH), trk, kTH * .5f);
        if (t > 0.05f)
            dl->AddRect(p, p + ImVec2(kTW, kTH),
                theme::alpha(theme::col_accent(), t * 0.55f), kTH * .5f, 0, 1.f);

        const float tx = p.x + 2.f + t * (kTW - kTH);
        const float tr = (kTH - 4.f) * .5f;
        const float cy = p.y + kTH * .5f;
        dl->AddCircleFilled({ tx + tr, cy }, tr + (hov ? 0.5f : 0.f),
            hov ? theme::col_text() : IM_COL32(224, 224, 232, 255), 18);

        ImGui::SameLine(0.f, 8.f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (kTH - ImGui::GetFontSize()) * .5f);
        ImGui::TextColored(
            ImGui::ColorConvertU32ToFloat4(*v ? theme::col_text() : theme::col_muted()),
            "%s", label);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - (kTH - ImGui::GetFontSize()) * .5f);
        ImGui::PopID();
        return clicked;
    }

    // ---- Toggle with color swatch --------------------------------------
    inline bool togglecolor(const char* tLabel, bool* v,
        const char* cLabel, float col[4]) {
        const bool changed = toggle(tLabel, v);
        constexpr float kColW = 15.f, kColH = 14.f;
        ImGui::SameLine(ImGui::GetContentRegionMax().x - kColW);
        ImGui::PushID(cLabel);
        {
            const ImVec2 p = ImGui::GetCursorScreenPos();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            const ImU32 sw = IM_COL32((int)(col[0] * 255), (int)(col[1] * 255),
                (int)(col[2] * 255), (int)(col[3] * 255));
            dl->AddRectFilled(p, p + ImVec2(kColW, kColH), sw, 2.f);
            dl->AddRect(p, p + ImVec2(kColW, kColH), *v ? theme::col_accent() : theme::col_border(), 2.f);
            ImGui::InvisibleButton("##s", { kColW, kColH });
            if (ImGui::IsItemClicked()) ImGui::OpenPopup("##cp");
            ImGui::SetNextWindowPos({ p.x + kColW + 4.f, p.y - 4.f });
            ImGui::PushStyleColor(ImGuiCol_PopupBg, theme::to_u32(theme::c_card));
            ImGui::PushStyleColor(ImGuiCol_Border, theme::to_u32(theme::c_border));
            if (ImGui::BeginPopup("##cp")) {
                ImGui::ColorPicker4("##pk", col,
                    ImGuiColorEditFlags_NoSidePreview |
                    ImGuiColorEditFlags_AlphaBar |
                    ImGuiColorEditFlags_PickerHueBar);
                ImGui::EndPopup();
            }
            ImGui::PopStyleColor(2);
        }
        ImGui::PopID();
        return changed;
    }

    // ---- Color swatch (no toggle) --------------------------------------
    inline void color4(const char* label, float col[4]) {
        constexpr float kColW = 15.f, kColH = 14.f;
        ImGui::PushID(label);
        const ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImU32 sw = IM_COL32((int)(col[0] * 255), (int)(col[1] * 255),
            (int)(col[2] * 255), (int)(col[3] * 255));
        dl->AddRectFilled(p, p + ImVec2(kColW, kColH), sw, 2.f);
        dl->AddRect(p, p + ImVec2(kColW, kColH), theme::col_border(), 2.f);
        ImGui::InvisibleButton("##sw", { kColW, kColH });
        if (ImGui::IsItemClicked()) ImGui::OpenPopup("##cpe");
        ImGui::SetNextWindowPos({ p.x + kColW + 4.f, p.y - 4.f });
        ImGui::PushStyleColor(ImGuiCol_PopupBg, theme::to_u32(theme::c_card));
        ImGui::PushStyleColor(ImGuiCol_Border, theme::to_u32(theme::c_border));
        if (ImGui::BeginPopup("##cpe")) {
            ImGui::ColorPicker4("##pk", col,
                ImGuiColorEditFlags_NoSidePreview |
                ImGuiColorEditFlags_AlphaBar |
                ImGuiColorEditFlags_PickerHueBar);
            ImGui::EndPopup();
        }
        ImGui::PopStyleColor(2);
        ImGui::PopID();
    }

    inline void dualcolor(const char* la, float* ca, const char* lb, float* cb) {
        constexpr float kColW = 15.f;
        ImGui::SameLine(ImGui::GetContentRegionMax().x - kColW * 2.f - 4.f);
        color4(la, ca);
        ImGui::SameLine(0.f, 4.f);
        color4(lb, cb);
    }

    inline void tricolor(const char* la, float* ca, const char* lb, float* cb,
        const char* lc, float* cc) {
        constexpr float kColW = 15.f;
        ImGui::SameLine(ImGui::GetContentRegionMax().x - kColW * 3.f - 8.f);
        color4(la, ca); ImGui::SameLine(0.f, 4.f);
        color4(lb, cb); ImGui::SameLine(0.f, 4.f);
        color4(lc, cc);
    }

    // ---- Section label — premium with gradient accent bar ----
    inline void labelsection(const char* text) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 p  = ImGui::GetCursorScreenPos();
        const float  fh = ImGui::GetFontSize();

        // Left accent bar with vertical gradient fade
        dl->AddRectFilledMultiColor(
            p, { p.x + 3.f, p.y + fh + 2.f },
            theme::col_accent(), theme::col_accent(),
            theme::alpha(theme::col_accent(), 0.0f),
            theme::alpha(theme::col_accent(), 0.0f));

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.f);
        ImGui::PushFont(font::medium());
        ImGui::TextColored(
            ImGui::ColorConvertU32ToFloat4(theme::col_muted()),
            "%s", text);
        ImGui::PopFont();

        // Gradient separator line that fades out to the right
        const ImVec2 lp = ImGui::GetCursorScreenPos();
        const float  lw = ImGui::GetContentRegionAvail().x;
        dl->AddRectFilledMultiColor(
            { lp.x, lp.y }, { lp.x + lw * 0.7f, lp.y + 1.f },
            theme::col_border(), IM_COL32(0, 0, 0, 0),
            IM_COL32(0, 0, 0, 0), theme::col_border());
        ImGui::Dummy({ 0.f, 1.f }); // space for the line
        ImGui::Dummy({ 0.f, 4.f });
    }

    inline void gap(float px = 4.f) { ImGui::Dummy({ 0.f, px }); }

    // ---- Float slider — premium with gradient track + glow handle ----
    inline bool sliderfloat(const char* label, float* v, float mn, float mx) {
        ImGui::PushID(label);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float avail = ImGui::GetContentRegionAvail().x;

        // Label + value on the same line
        const ImVec2 labelPos = ImGui::GetCursorScreenPos();
        ImGui::PushFont(font::regular());
        ImGui::TextColored(
            ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "%s", label);
        ImGui::PopFont();

        char buf[32]; std::snprintf(buf, sizeof(buf), "%.2f", *v);
        ImGui::SameLine(avail - ImGui::CalcTextSize(buf).x - 2.f);
        ImGui::TextColored(
            ImGui::ColorConvertU32ToFloat4(theme::alpha(theme::col_accent(), 0.9f)), "%s", buf);

        // Track
        const ImVec2 p = ImGui::GetCursorScreenPos();
        const float  w = ImGui::GetContentRegionAvail().x;
        constexpr float kH    = 20.f;
        constexpr float kTrkY = 10.f;  // vertical center of track
        constexpr float kTrkH = 3.f;

        ImGui::InvisibleButton("##sl", { w, kH });
        const bool active = ImGui::IsItemActive();
        const bool hov    = ImGui::IsItemHovered();
        if (active) {
            float rel = (ImGui::GetIO().MousePos.x - p.x) / w;
            *v = mn + ImClamp(rel, 0.f, 1.f) * (mx - mn);
        }
        const float t  = mx == mn ? 0.f : ImClamp((*v - mn) / (mx - mn), 0.f, 1.f);
        const float cx = p.y + kTrkY;

        // Track background
        dl->AddRectFilled(
            { p.x,       cx - kTrkH * .5f },
            { p.x + w,   cx + kTrkH * .5f },
            IM_COL32(20, 35, 55, 200), kTrkH);

        // Track fill with gradient
        if (t > 0.001f) {
            dl->AddRectFilledMultiColor(
                { p.x,           cx - kTrkH * .5f },
                { p.x + w * t,   cx + kTrkH * .5f },
                theme::alpha(theme::col_accent(), 0.6f),
                active ? theme::col_accent() : theme::alpha(theme::col_accent(), 0.85f),
                active ? theme::col_accent() : theme::alpha(theme::col_accent(), 0.85f),
                theme::alpha(theme::col_accent(), 0.6f));
        }

        // Handle (circle with glow)
        const float hx = p.x + w * t;
        const float hr = active ? 7.f : (hov ? 6.f : 5.f);

        // Handle glow
        if (active || hov) {
            dl->AddCircleFilled({ hx, cx }, hr + 4.f,
                theme::alpha(theme::col_accent(), active ? 0.25f : 0.12f), 24);
        }

        // Main handle
        dl->AddCircleFilled({ hx, cx }, hr,
            active ? theme::col_accent2() : theme::col_accent(), 24);
        dl->AddCircleFilled({ hx, cx }, hr * 0.45f,
            IM_COL32(255, 255, 255, 200), 12);

        ImGui::PopID();
        return ImGui::IsItemDeactivatedAfterEdit();
    }

    // ---- Int slider -----------------------------------------------------
    inline bool sliderint(const char* label, int* v, int mn, int mx) {
        float fv = (float)*v;
        bool r = sliderfloat(label, &fv, (float)mn, (float)mx);
        *v = (int)roundf(fv);
        return r;
    }

    // ---- Combo ----------------------------------------------------------
    inline bool combo(const char* label, int* idx, const std::vector<const char*>& items) {
        ImGui::PushID(label);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float avail = ImGui::GetContentRegionAvail().x;
        ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "%s", label);

        const ImVec2 p = ImGui::GetCursorScreenPos();
        const float w = avail, h = 23.f;

        ImGui::InvisibleButton("##cb", { w, h });
        const bool hov = ImGui::IsItemHovered();
        const bool clicked = ImGui::IsItemClicked();

        const float ht = anim::hover(ImGui::GetItemID());
        ImU32 fill = theme::lerp_u32(theme::col_surface(), theme::to_u32(theme::c_surface2, 1.3f), ht);
        dl->AddRectFilled(p, p + ImVec2(w, h), fill, 6.f);
        dl->AddRect(p, p + ImVec2(w, h),
            hov ? theme::alpha(theme::col_accent(), 0.35f) : theme::col_border(),
            6.f, 0, 1.f);

        const char* cur = (*idx >= 0 && *idx < (int)items.size()) ? items[*idx] : "---";
        dl->AddText(font::regular(), font::regular()->LegacySize,
            { p.x + 7.f, p.y + (h - ImGui::GetFontSize()) * .5f },
            theme::col_text(), cur);

        const float cx = p.x + w - 13.f, cy = p.y + h * .5f;
        dl->AddTriangleFilled({ cx - 4.f, cy - 2.f }, { cx + 4.f, cy - 2.f },
            { cx, cy + 3.f }, theme::col_muted());

        bool changed = false;
        if (clicked) ImGui::OpenPopup("##cop");
        ImGui::SetNextWindowPos({ p.x, p.y + h + 2.f });
        ImGui::SetNextWindowSize({ w, 0.f });
        ImGui::SetNextWindowSizeConstraints({ w, 0.f }, { w, 160.f });
        ImGui::PushStyleColor(ImGuiCol_PopupBg, theme::to_u32(theme::c_card));
        ImGui::PushStyleColor(ImGuiCol_Border, theme::to_u32(theme::c_border));
        if (ImGui::BeginPopup("##cop")) {
            for (int i = 0; i < (int)items.size(); i++) {
                const bool sel = (i == *idx);
                ImGui::PushStyleColor(ImGuiCol_Header,
                    ImGui::ColorConvertU32ToFloat4(sel ? theme::alpha(theme::col_accent(), 0.34f) : IM_COL32(0, 0, 0, 0)));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                    ImGui::ColorConvertU32ToFloat4(theme::alpha(theme::col_accent(), 0.34f)));
                ImGui::PushStyleColor(ImGuiCol_Text,
                    ImGui::ColorConvertU32ToFloat4(sel ? theme::alpha(theme::col_accent(), 0.85f) : theme::col_text()));
                if (ImGui::Selectable(items[i], sel)) {
                    *idx = i;
                    changed = true;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::PopStyleColor(3);
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleColor(2);
        ImGui::PopID();
        return changed;
    }

    // ---- Key bind -------------------------------------------------------
    inline float bindwidth(ImGuiKey key, ImKeyBindMode mode) {
        const char* kn = ImGui::GetKeyName(key);
        if (!kn || !*kn) kn = "NONE";
        const char* mn = (mode == ImKeyBindMode_Hold) ? "HOLD" : "TOGG";
        const float kw = ImMax(ImGui::CalcTextSize(kn).x + 14.f, 38.f);
        const float mw = ImGui::CalcTextSize(mn).x + 10.f;
        return kw + 3.f + mw;
    }

    inline void bind(const char* id, ImGuiKey* key, ImKeyBindMode* mode) {
        static bool s_listen = false;
        static ImGuiID s_ownerId = 0;
        ImGui::PushID(id);
        const ImGuiID self = ImGui::GetID("##kb");
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float h = 18.f;
        const char* kn = ImGui::GetKeyName(*key);
        if (!kn || !*kn) kn = "NONE";
        const char* mn = (*mode == ImKeyBindMode_Hold) ? "HOLD" : "TOGG";
        const float kw = ImMax(ImGui::CalcTextSize(kn).x + 14.f, 38.f);
        const float mw = ImGui::CalcTextSize(mn).x + 10.f;
        const bool listening = s_listen && (s_ownerId == self);

        const ImVec2 kp = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##k", { kw, h });
        if (ImGui::IsItemClicked()) { s_listen = true; s_ownerId = self; }
        if (listening) {
            for (int k = ImGuiKey_Tab; k < ImGuiKey_COUNT; k++)
                if (ImGui::IsKeyPressed((ImGuiKey)k, false)) {
                    *key = (ImGuiKey)k;
                    if (*key == ImGuiKey_Escape) *key = ImGuiKey_None;
                    s_listen = false;
                    break;
                }
        }
        dl->AddRectFilled(kp, kp + ImVec2(kw, h),
            listening ? theme::alpha(theme::col_accent(), 0.34f) : theme::col_surface(), 5.f);
        dl->AddRect(kp, kp + ImVec2(kw, h),
            listening ? theme::col_accent() : theme::col_border(), 5.f, 0, 1.f);
        dl->AddText({ kp.x + (kw - ImGui::CalcTextSize(kn).x) * .5f,
                       kp.y + (h - ImGui::GetFontSize()) * .5f },
            listening ? theme::alpha(theme::col_accent(), 0.85f) : theme::col_text(), kn);

        ImGui::SameLine(0.f, 3.f);
        const ImVec2 mp = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##m", { mw, h });
        const bool mhov = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked())
            *mode = (*mode == ImKeyBindMode_Hold) ? ImKeyBindMode_Toggle : ImKeyBindMode_Hold;
        dl->AddRectFilled(mp, mp + ImVec2(mw, h),
            mhov ? theme::lerp_u32(theme::col_surface(), theme::col_accent(), 0.15f) : theme::col_surface(), 5.f);
        dl->AddRect(mp, mp + ImVec2(mw, h), theme::col_border(), 5.f, 0, 1.f);
        dl->AddText({ mp.x + (mw - ImGui::CalcTextSize(mn).x) * .5f,
                       mp.y + (h - ImGui::GetFontSize()) * .5f },
            theme::col_muted(), mn);
        ImGui::PopID();
    }

    // ---- Simple key selector --------------------------------------------
    inline void keyselect(const char* id, ImGuiKey* key) {
        static bool s_listen = false;
        static ImGuiID s_ownerId = 0;
        ImGui::PushID(id);
        const ImGuiID self = ImGui::GetID("##ks");
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float h = 18.f;
        const char* kn = ImGui::GetKeyName(*key);
        if (!kn || !*kn) kn = "NONE";
        const float kw = ImMax(ImGui::CalcTextSize(kn).x + 18.f, 48.f);
        const bool listening = s_listen && (s_ownerId == self);

        const ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##key", { kw, h });
        if (ImGui::IsItemClicked()) { s_listen = true; s_ownerId = self; }
        if (listening) {
            for (int k = ImGuiKey_Tab; k < ImGuiKey_COUNT; k++)
                if (ImGui::IsKeyPressed((ImGuiKey)k, false)) {
                    *key = (ImGuiKey)k;
                    if (*key == ImGuiKey_Escape) *key = ImGuiKey_None;
                    s_listen = false;
                    break;
                }
        }
        const char* text = listening ? "..." : kn;
        const float tw = ImGui::CalcTextSize(text).x;
        dl->AddRectFilled(p, p + ImVec2(kw, h),
            listening ? theme::alpha(theme::col_accent(), 0.34f) : theme::col_surface(), 5.f);
        dl->AddRect(p, p + ImVec2(kw, h),
            listening ? theme::col_accent() : theme::col_border(), 5.f, 0, 1.f);
        dl->AddText({ p.x + (kw - tw) * .5f, p.y + (h - ImGui::GetFontSize()) * .5f },
            listening ? theme::alpha(theme::col_accent(), 0.85f) : theme::col_text(), text);
        ImGui::PopID();
    }

    // ---- Custom button --------------------------------------------------
    inline bool button(const char* label, ImU32 bg, ImU32 brd, ImU32 txt,
        float w = -1.f, float h = 22.f) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float W = (w < 0.f) ? ImGui::GetContentRegionAvail().x : w;
        const ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton(label, { W, h });
        const bool hov = ImGui::IsItemHovered();
        const bool act = ImGui::IsItemActive();
        const ImU32 fill = act ? theme::lerp_u32(bg, theme::col_text(), 0.06f)
            : (hov ? theme::lerp_u32(bg, theme::col_text(), 0.10f) : bg);
        dl->AddRectFilled(p, p + ImVec2(W, h), fill, 5.f);
        dl->AddRect(p, p + ImVec2(W, h),
            hov ? theme::lerp_u32(brd, theme::col_accent(), 0.35f) : brd, 5.f, 0, 1.f);
        const float tw = ImGui::CalcTextSize(label).x;
        dl->AddText({ p.x + (W - tw) * .5f, p.y + (h - ImGui::GetFontSize()) * .5f }, txt, label);
        return ImGui::IsItemClicked();
    }

    // ---- Accent button shortcut -----------------------------------------
    inline bool accent_button(const char* label, float w = -1.f, float h = 22.f) {
        return button(label, theme::to_u32(theme::c_accent, 0.25f),
            theme::col_accent(), theme::col_accent(), w, h);
    }

    // ---- Danger button shortcut -----------------------------------------
    inline bool danger_button(const char* label, float w = -1.f, float h = 22.f) {
        return button(label, theme::to_u32(theme::c_danger, 0.2f),
            theme::col_danger(), theme::col_danger(), w, h);
    }

} // namespace w
