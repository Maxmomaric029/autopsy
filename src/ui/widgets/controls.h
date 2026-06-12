#pragma once
#include <Windows.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <vector>
#include <string>
#include <cstdio>
#include <cmath>
#include "../core/theme.h"
#include "../core/animation.h"
#include "../core/fonts.h"
#include "../core/sound.h"

// ========================================================================
// Custom ImGui widgets — industrial-minimal, lime-green accent
// All controls use InvisibleButton + ImDrawList for pixel-perfect rendering.
// ========================================================================

namespace w {

    // ========================================================================
    // Forward declarations of types used by keybind widgets
    // ========================================================================
    enum class ImKeyBindMode { Hold, Toggle };

        // ---- Card -----------------------------------------------------------
    struct card {
        static bool begin(const char* id, ImVec2 size, const char* title = nullptr);
        static void end();
    };

    // ---- Gap helper -------------------------------------------------------
    inline void gap(float px = 4.f) { ImGui::Dummy({ 0.f, px }); }

    // ========================================================================
    // Toggle switch
    // Dimensions: 34 x 18px
    // Track: bg SURFACE2, border BORDER, border-radius 9px
    //         ON: bg rgba(ACCENT, 0.18), border rgba(ACCENT, 0.4)
    // Thumb: circle 12px, MUTED when OFF, ACCENT when ON
    // ========================================================================
    inline bool toggle(const char* label, bool* v) {
        ImGui::PushID(label);
        ImDrawList* dl = ImGui::GetWindowDrawList();

        constexpr float kTW = 34.f, kTH = 18.f;
        constexpr float kThumbR = 6.f; // 12px diameter
        const ImVec2 p = ImGui::GetCursorScreenPos();

        ImGui::InvisibleButton("##t", { kTW, kTH });
        const bool clicked = ImGui::IsItemClicked();
        const bool hov = ImGui::IsItemHovered();
        if (clicked) {
            *v = !*v;
            // Sound: play toggle_on / toggle_off
            sound::play(*v ? sound::id::toggle_on : sound::id::toggle_off);
        }

        const float t = anim::g_anim.get_toggle(ImGui::GetItemID(), *v);

        // Track background
        ImU32 trackBg = theme::lerp_u32(
            IM_COL32(23, 23, 27, 255),                      // SURFACE2 off
            IM_COL32(200, 241, 53, (int)(255 * 0.18f)),      // ACCENT_DIM on
            t);
        dl->AddRectFilled(p, p + ImVec2(kTW, kTH), trackBg, kTH * 0.5f);

        // Track border
        ImU32 trackBr = theme::lerp_u32(
            IM_COL32(255, 255, 255, 15),                     // BORDER off
            IM_COL32(200, 241, 53, (int)(255 * 0.40f)),      // ACCENT border on
            t);
        dl->AddRect(p, p + ImVec2(kTW, kTH), trackBr, kTH * 0.5f, 0, 1.f);

        // Thumb position: left=2px (OFF) → left=20px (ON)
        const float thumbX = p.x + 2.f + t * (kTW - kTH);
        const float thumbY = p.y + kTH * 0.5f;

        // Thumb shadow
        dl->AddCircleFilled({ thumbX + kThumbR, thumbY + 0.5f },
            kThumbR + 0.5f, IM_COL32(0, 0, 0, 60), 24);

        // Thumb
        ImU32 thumbCol = theme::lerp_u32(
            IM_COL32(90, 90, 96, 255),                       // MUTED off
            IM_COL32(200, 241, 53, 255),                     // ACCENT on
            t);
        float thumbRad = kThumbR + (hov ? 0.5f : 0.f);
        dl->AddCircleFilled({ thumbX + kThumbR, thumbY }, thumbRad, thumbCol, 24);

        // Label
        ImGui::SameLine(0.f, 10.f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (kTH - ImGui::GetFontSize()) * 0.5f);
        ImGui::TextColored(
            ImGui::ColorConvertU32ToFloat4(*v ? theme::col_text() : theme::col_muted()),
            "%s", label);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - (kTH - ImGui::GetFontSize()) * 0.5f);

        ImGui::PopID();
        return clicked;
    }

    // ========================================================================
    // Float slider
    // Track height: 3px, bg SURFACE2, border-radius 2px
    // Fill: gradient ACCENT*0.6 → ACCENT
    // Handle: circle 5px rest / 6px hover / 7px active
    // ========================================================================
    inline bool sliderfloat(const char* label, float* v, float mn, float mx) {
        ImGui::PushID(label);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float avail = ImGui::GetContentRegionAvail().x;

        // Label (left)
        ImGui::TextColored(
            ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "%s", label);

        // Value (right, monospace)
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.2f", *v);
        ImGui::SameLine(avail - ImGui::CalcTextSize(buf).x - 2.f);
        ImGui::PushFont(font::mono());
        ImGui::TextColored(
            ImGui::ColorConvertU32ToFloat4(theme::col_accent()), "%s", buf);
        ImGui::PopFont();

        // Slider track
        const ImVec2 p = ImGui::GetCursorScreenPos();
        const float  w = ImGui::GetContentRegionAvail().x;
        constexpr float kH    = 22.f;
        constexpr float kTrkY = 12.f;
        constexpr float kTrkH = 3.f;

        ImGui::InvisibleButton("##sl", { w, kH });
        const bool active = ImGui::IsItemActive();
        const bool hov    = ImGui::IsItemHovered();
        if (active) {
            float rel = (ImGui::GetIO().MousePos.x - p.x) / w;
            *v = mn + ImClamp(rel, 0.f, 1.f) * (mx - mn);
        }
        const float t  = mx == mn ? 0.f : ImClamp((*v - mn) / (mx - mn), 0.f, 1.f);
        const float trackCenterY = p.y + kTrkY;

        // Track background
        dl->AddRectFilled(
            { p.x,       trackCenterY - kTrkH * 0.5f },
            { p.x + w,   trackCenterY + kTrkH * 0.5f },
            IM_COL32(23, 23, 27, 255), 2.f);

        // Track fill with gradient
        if (t > 0.001f) {
            ImU32 fillStart = IM_COL32(200, 241, 53, (int)(255 * 0.6f));
            ImU32 fillEnd   = theme::col_accent();
            dl->AddRectFilledMultiColor(
                { p.x,           trackCenterY - kTrkH * 0.5f },
                { p.x + w * t,   trackCenterY + kTrkH * 0.5f },
                fillStart, fillEnd, fillEnd, fillStart);
        }

        // Handle
        const float hx = p.x + w * t;
        const float hr = active ? 7.f : (hov ? 6.f : 5.f);

        // Handle glow
        if (active) {
            dl->AddCircleFilled({ hx, trackCenterY }, hr + 3.f,
                IM_COL32(200, 241, 53, (int)(255 * 0.25f)), 24);
        }

        // Main handle (outer circle)
        dl->AddCircleFilled({ hx, trackCenterY }, hr, theme::col_accent(), 24);

        // Inner dot (white)
        if (hr >= 5.f) {
            dl->AddCircleFilled({ hx, trackCenterY }, hr * 0.35f,
                IM_COL32(255, 255, 255, 200), 12);
        }

        ImGui::PopID();
        return ImGui::IsItemDeactivatedAfterEdit();
    }

    // ---- Int slider -------------------------------------------------------
    inline bool sliderint(const char* label, int* v, int mn, int mx) {
        float fv = (float)*v;
        bool r = sliderfloat(label, &fv, (float)mn, (float)mx);
        *v = (int)roundf(fv);
        return r;
    }

    // ========================================================================
    // Pill toolbar — selects one of N options
    // Container: bg SURFACE2, border BORDER, border-radius 7px, padding 2px
    // Active: bg ACCENT_DIM, border rgba(ACCENT,0.2), color ACCENT
    // ========================================================================
    inline int pill_toolbar(const char* id, const std::vector<const char*>& labels, int* current) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float avail = ImGui::GetContentRegionAvail().x;
        const int count = (int)labels.size();
        constexpr float kH = 24.f;
        constexpr float kR = 7.f;
        constexpr float kPad = 2.f;
        const float itemW = (avail - kPad * 2.f - (count - 1) * 2.f) / count;

        ImGui::PushID(id);
        const ImVec2 base = ImGui::GetCursorScreenPos();

        // Container background
        dl->AddRectFilled(base, base + ImVec2(avail, kH),
            IM_COL32(23, 23, 27, 255), kR); // SURFACE2
        dl->AddRect(base, base + ImVec2(avail, kH),
            IM_COL32(255, 255, 255, 15), kR, 0, 1.f); // BORDER

        for (int i = 0; i < count; i++) {
            const ImVec2 mn = base + ImVec2(kPad + i * (itemW + 2.f), kPad);
            const ImVec2 mx = mn + ImVec2(itemW, kH - kPad * 2.f);
            ImGui::SetCursorScreenPos(mn);
            ImGui::InvisibleButton(labels[i], { itemW, kH - kPad * 2.f });
            if (ImGui::IsItemClicked()) {
                *current = i;
                sound::play(sound::id::toggle_on);
            }
            const bool active = (*current == i);
            const float t = anim::g_anim.get_toggle(ImGui::GetItemID(), active);

            if (t > 0.01f) {
                // Active pill background
                dl->AddRectFilled(mn, mx,
                    IM_COL32(200, 241, 53, (int)(255 * 0.14f)), 5.f); // ACCENT_DIM
                // Active pill border
                dl->AddRect(mn, mx,
                    IM_COL32(200, 241, 53, (int)(255 * 0.20f)), 5.f, 0, 1.f);
            }

            // Label text
            ImGui::PushFont(font::mono());
            ImVec2 ts = ImGui::CalcTextSize(labels[i]);
            ImU32 textCol = active
                ? theme::col_accent()
                : IM_COL32(90, 90, 96, 255); // MUTED
            dl->AddText(
                mn + ImVec2((itemW - ts.x) * 0.5f, ((kH - kPad * 2.f) - font::size::mono_sm) * 0.5f + 1.f),
                textCol, labels[i]);
            ImGui::PopFont();
        }

        ImGui::SetCursorScreenPos(base + ImVec2(0.f, kH + 4.f));
        ImGui::Dummy({avail, 0.f});
        ImGui::PopID();
        return *current;
    }

    // ========================================================================
    // Section header (labelsection)
    // Left bar: 3px wide, gradient ACCENT → transparent
    // Text: JetBrains Mono 9px uppercase, letter-spacing 0.14em, color MUTED
    // Separator: 1px line under text, fade horizontal
    // ========================================================================
    inline void labelsection(const char* text) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 p  = ImGui::GetCursorScreenPos();
        ImGui::PushFont(font::mono());
        const float  fh = font::size::mono;
        ImGui::PopFont();
        const float contentWidth = ImGui::GetContentRegionAvail().x;

        // Background tint (subtle accent gradient)
        dl->AddRectFilledMultiColor(
            p, p + ImVec2(contentWidth * 0.4f, fh + 4.f),
            IM_COL32(200, 241, 53, 5), IM_COL32(0, 0, 0, 0),
            IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0));

        // Left accent bar — 3px wide, vertical gradient
        const float barH = fh + 2.f;
        dl->AddRectFilledMultiColor(
            { p.x, p.y },
            { p.x + 3.f, p.y + barH },
            theme::col_accent(),
            theme::col_accent(),
            theme::alpha(theme::col_accent(), 0.f),
            theme::alpha(theme::col_accent(), 0.f));

        // Text
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.f);
        ImGui::PushFont(font::mono());
        dl->AddText(p + ImVec2(10.f, 0.f),
            theme::col_muted(), text);
        ImGui::PopFont();

        // Separator line under text — fade horizontal
        const ImVec2 lp = ImGui::GetCursorScreenPos();
        float textEndX = p.x + 10.f + ImGui::CalcTextSize(text).x * (font::size::uppercase / 13.f);
        const float lw = ImGui::GetContentRegionAvail().x;
        float sepStart = ImMax(lp.x, textEndX + 8.f);
        float sepW = lw - (sepStart - lp.x);
        dl->AddRectFilledMultiColor(
            { sepStart, lp.y + 4.f },
            { sepStart + sepW * 0.7f, lp.y + 5.f },
            IM_COL32(255, 255, 255, 11), IM_COL32(0, 0, 0, 0),
            IM_COL32(0, 0, 0, 0), IM_COL32(255, 255, 255, 11));
        ImGui::Dummy({ 0.f, 8.f });
    }

    // ========================================================================
    // Keybind chip — listening mode, hover state
    // Container: bg SURFACE2, border BORDER, border-radius 5px
    // Font: JetBrains Mono 10px
    // ========================================================================
    struct ListenState { bool listening = false; ImGuiID ownerId = 0; };
    inline ListenState& g_listen() { static ListenState s; return s; }

    inline void keyselect(const char* id, ImGuiKey* key) {
        auto& gs = g_listen();
        ImGui::PushID(id);
        const ImGuiID self = ImGui::GetID("##ks");
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float h = 18.f;
        const char* kn = ImGui::GetKeyName(*key);
        if (!kn || !*kn) kn = "NONE";
        const float kw = ImMax(ImGui::CalcTextSize(kn).x + 18.f, 48.f);
        const bool listening = gs.listening && (gs.ownerId == self);

        const ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##key", { kw, h });
        if (ImGui::IsItemClicked()) {
            gs.listening = true;
            gs.ownerId = self;
            sound::play(sound::id::bind_start);
        }
        // Hover sound
        if (ImGui::IsItemHovered() && !listening) {
            // hover sound handled by external caller with cooldown
        }
        if (listening) {
            for (int k = ImGuiKey_Tab; k < ImGuiKey_COUNT; k++) {
                if (ImGui::IsKeyPressed((ImGuiKey)k, false)) {
                    *key = (ImGuiKey)k;
                    if (*key == ImGuiKey_Escape) *key = ImGuiKey_None;
                    gs.listening = false;
                    sound::play(sound::id::bind_set);
                    break;
                }
            }
        }

        // Background
        ImU32 bg = listening
            ? IM_COL32(200, 241, 53, (int)(255 * 0.14f))
            : IM_COL32(23, 23, 27, 255);
        dl->AddRectFilled(p, p + ImVec2(kw, h), bg, 5.f);

        // Border
        ImU32 brd = listening
            ? theme::col_accent()
            : IM_COL32(255, 255, 255, 15);
        dl->AddRect(p, p + ImVec2(kw, h), brd, 5.f, 0, 1.f);

        // Text
        const char* text = listening ? "..." : kn;
        const float tw = ImGui::CalcTextSize(text).x;
        ImU32 txtCol = listening
            ? theme::col_accent()
            : theme::col_text();
        ImGui::PushFont(font::mono());
        dl->AddText({ p.x + (kw - tw) * 0.5f, p.y + (h - font::size::mono_sm) * 0.5f + 1.f },
            txtCol, text);
        ImGui::PopFont();
        ImGui::PopID();
    }

    // ========================================================================
    // Key bind (key + Hold/Toggle mode)
    // ========================================================================
    inline float bindwidth(ImGuiKey key, ImKeyBindMode mode) {
        const char* kn = ImGui::GetKeyName(key);
        if (!kn || !*kn) kn = "NONE";
        const char* mn = (mode == ImKeyBindMode::Hold) ? "HOLD" : "TOGG";
        ImGui::PushFont(font::mono());
        const float kw = ImMax(ImGui::CalcTextSize(kn).x + 14.f, 38.f);
        const float mw = ImGui::CalcTextSize(mn).x + 10.f;
        ImGui::PopFont();
        return kw + 3.f + mw;
    }

    inline void bind(const char* id, ImGuiKey* key, ImKeyBindMode* mode) {
        auto& gs = g_listen();
        ImGui::PushID(id);
        const ImGuiID self = ImGui::GetID("##kb");
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float h = 18.f;
        const char* kn = ImGui::GetKeyName(*key);
        if (!kn || !*kn) kn = "NONE";
        const char* mn = (*mode == ImKeyBindMode::Hold) ? "HOLD" : "TOGG";

        ImGui::PushFont(font::mono());
        const float kw = ImMax(ImGui::CalcTextSize(kn).x + 14.f, 38.f);
        const float mw = ImGui::CalcTextSize(mn).x + 10.f;
        ImGui::PopFont();

        const bool listening = gs.listening && (gs.ownerId == self);

        // Key chip
        const ImVec2 kp = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##k", { kw, h });
        if (ImGui::IsItemClicked()) {
            gs.listening = true;
            gs.ownerId = self;
            sound::play(sound::id::bind_start);
        }
        if (listening) {
            for (int k = ImGuiKey_Tab; k < ImGuiKey_COUNT; k++) {
                if (ImGui::IsKeyPressed((ImGuiKey)k, false)) {
                    *key = (ImGuiKey)k;
                    if (*key == ImGuiKey_Escape) *key = ImGuiKey_None;
                    gs.listening = false;
                    sound::play(sound::id::bind_set);
                    break;
                }
            }
        }

        // Key background
        ImU32 keyBg = listening
            ? IM_COL32(200, 241, 53, (int)(255 * 0.14f))
            : IM_COL32(23, 23, 27, 255);
        ImU32 keyBrd = listening
            ? theme::col_accent()
            : IM_COL32(255, 255, 255, 15);
        dl->AddRectFilled(kp, kp + ImVec2(kw, h), keyBg, 5.f);
        dl->AddRect(kp, kp + ImVec2(kw, h), keyBrd, 5.f, 0, 1.f);

        const char* keyText = listening ? "..." : kn;
        ImGui::PushFont(font::mono());
        const float ktw = ImGui::CalcTextSize(keyText).x;
        ImU32 keyTxt = listening ? theme::col_accent() : theme::col_text();
        dl->AddText({ kp.x + (kw - ktw) * 0.5f, kp.y + (h - font::size::mono_sm) * 0.5f + 1.f },
            keyTxt, keyText);

        // Mode chip
        ImGui::SameLine(0.f, 3.f);
        const ImVec2 mp = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##m", { mw, h });
        const bool mhov = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked()) {
            *mode = (*mode == ImKeyBindMode::Hold) ? ImKeyBindMode::Toggle : ImKeyBindMode::Hold;
            sound::play(sound::id::toggle_on);
        }

        ImU32 modeBg = mhov
            ? IM_COL32(200, 241, 53, (int)(255 * 0.08f))
            : IM_COL32(23, 23, 27, 255);
        dl->AddRectFilled(mp, mp + ImVec2(mw, h), modeBg, 5.f);
        dl->AddRect(mp, mp + ImVec2(mw, h), IM_COL32(255, 255, 255, 15), 5.f, 0, 1.f);

        const float mtw = ImGui::CalcTextSize(mn).x;
        dl->AddText({ mp.x + (mw - mtw) * 0.5f, mp.y + (h - font::size::mono_sm) * 0.5f + 1.f },
            IM_COL32(90, 90, 96, 255), mn);
        ImGui::PopFont();

        ImGui::PopID();
    }

    // ========================================================================
    // Color swatch
    // Size: 22 x 22px, border-radius 5px
    // ========================================================================
    inline void color4(const char* label, float col[4]) {
        constexpr float kColW = 22.f, kColH = 22.f;
        ImGui::PushID(label);
        const ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImU32 sw = IM_COL32((int)(col[0] * 255), (int)(col[1] * 255),
            (int)(col[2] * 255), (int)(col[3] * 255));

        // Checkerboard for transparency
        dl->AddRectFilled(p, p + ImVec2(kColW, kColH), IM_COL32(30, 30, 35, 255), 5.f);

        // Swatch
        dl->AddRectFilled(p, p + ImVec2(kColW, kColH), sw, 5.f);

        // Border
        const bool hov = ImGui::IsMouseHoveringRect(p, p + ImVec2(kColW, kColH));
        dl->AddRect(p, p + ImVec2(kColW, kColH),
            hov ? theme::col_accent() : IM_COL32(255, 255, 255, 8), 5.f, 0, 1.f);

        ImGui::InvisibleButton("##sw", { kColW, kColH });
        if (ImGui::IsItemClicked()) ImGui::OpenPopup("##cpe");

        // Clamp popup position
        ImVec2 cp(p.x + kColW + 4.f, p.y - 4.f);
        const ImVec2 ds = ImGui::GetIO().DisplaySize;
        cp.x = ImClamp(cp.x, 0.f, ds.x - 280.f);
        cp.y = ImClamp(cp.y, 0.f, ds.y - 320.f);
        ImGui::SetNextWindowPos(cp);

        ImGui::PushStyleColor(ImGuiCol_PopupBg, theme::to_u32(theme::c_surface));
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
        constexpr float kColW = 22.f;
        ImGui::SameLine(ImGui::GetContentRegionMax().x - kColW * 2.f - 6.f);
        color4(la, ca);
        ImGui::SameLine(0.f, 6.f);
        color4(lb, cb);
    }

    inline void tricolor(const char* la, float* ca, const char* lb, float* cb,
        const char* lc, float* cc) {
        constexpr float kColW = 22.f;
        ImGui::SameLine(ImGui::GetContentRegionMax().x - kColW * 3.f - 12.f);
        color4(la, ca); ImGui::SameLine(0.f, 6.f);
        color4(lb, cb); ImGui::SameLine(0.f, 6.f);
        color4(lc, cc);
    }

    // ========================================================================
    // Toggle with inline color swatch
    // ========================================================================
    inline bool togglecolor(const char* tLabel, bool* v,
        const char* cLabel, float col[4]) {
        const bool changed = toggle(tLabel, v);
        constexpr float kColW = 22.f, kColH = 22.f;
        ImGui::SameLine(ImGui::GetContentRegionMax().x - kColW);
        ImGui::PushID(cLabel);
        {
            const ImVec2 p = ImGui::GetCursorScreenPos();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            const ImU32 sw = IM_COL32((int)(col[0] * 255), (int)(col[1] * 255),
                (int)(col[2] * 255), (int)(col[3] * 255));
            dl->AddRectFilled(p, p + ImVec2(kColW, kColH), IM_COL32(30, 30, 35, 255), 5.f);
            dl->AddRectFilled(p, p + ImVec2(kColW, kColH), sw, 5.f);
            dl->AddRect(p, p + ImVec2(kColW, kColH),
                *v ? theme::col_accent() : IM_COL32(255, 255, 255, 8), 5.f, 0, 1.f);
            ImGui::InvisibleButton("##s", { kColW, kColH });
            if (ImGui::IsItemClicked()) ImGui::OpenPopup("##cp");
            {
                ImVec2 cp(p.x + kColW + 4.f, p.y - 4.f);
                const ImVec2 ds = ImGui::GetIO().DisplaySize;
                cp.x = ImClamp(cp.x, 0.f, ds.x - 280.f);
                cp.y = ImClamp(cp.y, 0.f, ds.y - 320.f);
                ImGui::SetNextWindowPos(cp);
            }
            ImGui::PushStyleColor(ImGuiCol_PopupBg, theme::to_u32(theme::c_surface));
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

    // ========================================================================
    // Combo box (dropdown)
    // ========================================================================
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
        ImU32 fill = theme::lerp_u32(IM_COL32(23, 23, 27, 255),
            IM_COL32(30, 30, 35, 255), ht);
        dl->AddRectFilled(p, p + ImVec2(w, h), fill, 6.f);
        dl->AddRect(p, p + ImVec2(w, h),
            hov ? IM_COL32(200, 241, 53, (int)(255 * 0.35f)) : IM_COL32(255, 255, 255, 15),
            6.f, 0, 1.f);

        const char* cur = (*idx >= 0 && *idx < (int)items.size()) ? items[*idx] : "---";
        ImGui::PushFont(font::body());
        dl->AddText({ p.x + 7.f, p.y + (h - font::size::body) * 0.5f + 1.f },
            theme::col_text(), cur);
        ImGui::PopFont();

        const float cx = p.x + w - 13.f, cy = p.y + h * 0.5f;
        dl->AddTriangleFilled({ cx - 4.f, cy - 2.f }, { cx + 4.f, cy - 2.f },
            { cx, cy + 3.f }, IM_COL32(90, 90, 96, 255));

        bool changed = false;
        if (clicked) ImGui::OpenPopup("##cop");
        ImGui::SetNextWindowPos({ p.x, p.y + h + 2.f });
        ImGui::SetNextWindowSize({ w, 0.f });
        ImGui::SetNextWindowSizeConstraints({ w, 0.f }, { w, 160.f });
        ImGui::PushStyleColor(ImGuiCol_PopupBg, theme::to_u32(theme::c_surface));
        ImGui::PushStyleColor(ImGuiCol_Border, theme::to_u32(theme::c_border));
        if (ImGui::BeginPopup("##cop")) {
            for (int i = 0; i < (int)items.size(); i++) {
                const bool sel = (i == *idx);
                ImGui::PushStyleColor(ImGuiCol_Header,
                    ImGui::ColorConvertU32ToFloat4(sel ? theme::accent_dim() : IM_COL32(0, 0, 0, 0)));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                    ImGui::ColorConvertU32ToFloat4(theme::accent_dim()));
                ImGui::PushStyleColor(ImGuiCol_Text,
                    ImGui::ColorConvertU32ToFloat4(sel ? theme::col_accent() : theme::col_text()));
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

    // ========================================================================
    // Custom button
    // ========================================================================
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
        ImGui::PushFont(font::body());
        dl->AddText({ p.x + (W - tw) * 0.5f, p.y + (h - font::size::body) * 0.5f + 1.f },
            txt, label);
        ImGui::PopFont();
        return ImGui::IsItemClicked();
    }

    inline bool accent_button(const char* label, float w = -1.f, float h = 22.f) {
        return button(label, theme::accent_dim(),
            theme::col_accent(), theme::col_accent(), w, h);
    }

    inline bool danger_button(const char* label, float w = -1.f, float h = 22.f) {
        return button(label,
            IM_COL32(255, 71, 87, (int)(255 * 0.20f)),
            theme::col_danger(), theme::col_danger(), w, h);
    }

    // ========================================================================
    // Help tooltip (hover pill)
    // ========================================================================
    inline void helptooltip(const char* text) {
        if (!ImGui::IsItemHovered() || !text || !*text) return;
        ImDrawList* dl = ImGui::GetForegroundDrawList();
        const ImVec2 mp = ImGui::GetIO().MousePos;
        const float fontSize = 11.f;
        ImVec2 ts = ImGui::CalcTextSize(text);
        const float padX = 10.f, padY = 5.f;
        const float w = ts.x + padX * 2.f;
        const float h = fontSize + padY * 2.f;
        ImVec2 mn = mp + ImVec2(12.f, 16.f);
        ImVec2 mx = mn + ImVec2(w, h);
        const ImVec2 display = ImGui::GetIO().DisplaySize;
        if (mx.x > display.x - 8.f) { mn.x -= (mx.x - display.x + 8.f); mx.x = display.x - 8.f; }
        if (mx.y > display.y - 8.f) { mn.y -= (mx.y - display.y + 8.f); mx.y = display.y - 8.f; }
        dl->AddRectFilled(mn + ImVec2(0.f, 3.f), mx + ImVec2(0.f, 3.f),
            IM_COL32(0,0,0,80), h * 0.5f);
        dl->AddRectFilled(mn, mx, IM_COL32(10, 10, 11, 240), h * 0.5f);
        dl->AddRect(mn, mx, IM_COL32(200, 241, 53, (int)(255 * 0.5f)), h * 0.5f, 0, 1.f);
        ImGui::PushFont(font::mono());
        dl->AddText(mn + ImVec2(padX, padY), IM_COL32(90, 90, 96, 255), text);
        ImGui::PopFont();
    }

} // namespace w
