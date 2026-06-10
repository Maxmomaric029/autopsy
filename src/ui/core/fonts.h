#pragma once
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_win32.h>
#include "FontAwesome/IconsFontAwesome6.h"

// Embedded font data (generated from TTFs)
#include "../embedded/font_inter_regular.h"
#include "../embedded/font_inter_semibold.h"
#include "../embedded/font_inter_bold.h"
#include "../embedded/font_inter_black.h"
#include "../embedded/font_fa_solid.h"

namespace font {

    inline ImFont* g_regular = nullptr;
    inline ImFont* g_medium  = nullptr;
    inline ImFont* g_bold    = nullptr;
    inline ImFont* g_mono    = nullptr;
    inline ImFont* g_logo    = nullptr;

    // ==================================================================
    // Typography scale (F2.6) — single source of truth for all font sizes
    // ==================================================================
    namespace size {
        inline constexpr float title  = 11.f;  // card titles, uppercase
        inline constexpr float label  = 13.f;  // control labels
        inline constexpr float body   = 13.f;  // regular text / values
        inline constexpr float badge  = 12.f;  // badges / pills
        inline constexpr float mono   = 12.f;  // monospace (version, stats)
        inline constexpr float logo   = 22.f;  // logo / branding
        inline constexpr float icon   = 11.f;  // FA6 icon in toggles
    }

    inline bool load(float dpiScale = 1.0f) {
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();

        ImFontConfig cfg;
        cfg.PixelSnapH  = false;
        cfg.OversampleH = 1;
        cfg.OversampleV = 1;

        // FontAwesome glyph ranges (16-bit Private Use Area)
        static const ImWchar fa_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };

        ImFontConfig fa_cfg;
        fa_cfg.MergeMode        = true;
        fa_cfg.PixelSnapH       = true;
        fa_cfg.GlyphMinAdvanceX = 14.f;

        auto loadFont = [&](const unsigned char* data, unsigned int size,
                            float px, ImFont** out, bool mergeFA) -> ImFont* {
            cfg.MergeMode = false;
            cfg.FontDataOwnedByAtlas = false; // we own the memory
            ImFont* f = io.Fonts->AddFontFromMemoryTTF(
                (void*)data, (int)size, px * dpiScale, &cfg);
            if (f && mergeFA) {
                fa_cfg.FontDataOwnedByAtlas = false;
                io.Fonts->AddFontFromMemoryTTF(
                    (void*)font_fa_solid, (int)font_fa_solid_size,
                    (px - 1.f) * dpiScale, &fa_cfg, fa_ranges);
            }
            if (out) *out = f;
            return f;
        };

        loadFont(font_inter_regular,  font_inter_regular_size,  size::body,  &g_regular, true);
        loadFont(font_inter_semibold, font_inter_semibold_size, size::body,  &g_medium,  true);
        loadFont(font_inter_bold,     font_inter_bold_size,     size::label, &g_bold,    true);
        loadFont(font_inter_black,    font_inter_black_size,    size::logo,  &g_logo,    false);
        loadFont(font_inter_regular,  font_inter_regular_size,  size::mono,  &g_mono,    false);

        // Fallbacks
        if (!g_regular) g_regular = io.Fonts->AddFontDefault();
        if (!g_medium)  g_medium  = g_regular;
        if (!g_bold)    g_bold    = g_regular;
        if (!g_mono)    g_mono    = g_regular;
        if (!g_logo)    g_logo    = g_bold;

        io.Fonts->Build();
        return true;
    }

    inline ImFont* regular() { return g_regular ? g_regular : ImGui::GetFont(); }
    inline ImFont* medium()  { return g_medium  ? g_medium  : regular(); }
    inline ImFont* bold()    { return g_bold    ? g_bold    : regular(); }
    inline ImFont* mono()    { return g_mono    ? g_mono    : regular(); }
    inline ImFont* logo()    { return g_logo    ? g_logo    : bold(); }

} // namespace font
