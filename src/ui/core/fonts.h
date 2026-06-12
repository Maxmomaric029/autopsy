#pragma once
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/misc/imgui_freetype.h>
#include "FontAwesome/IconsFontAwesome6.h"

// Embedded font data — Syne + JetBrains Mono + FontAwesome
#include "../embedded/font_syne_regular.h"
#include "../embedded/font_syne_medium.h"
#include "../embedded/font_syne_extrabold.h"
#include "../embedded/font_jetbrains_mono_regular.h"
#include "../embedded/font_fa_solid.h"

namespace font {

    // ========================================================================
    // Font pointers — Syne for display, JetBrains Mono for values
    // ========================================================================
    inline ImFont* g_display  = nullptr; // Syne ExtraBold 17px — logo
    inline ImFont* g_label    = nullptr; // Syne Medium 12px — card headers
    inline ImFont* g_body     = nullptr; // Syne Regular 12px — toggles/options
    inline ImFont* g_mono     = nullptr; // JetBrains Mono Regular 11px — values
    inline ImFont* g_mono_sm  = nullptr; // JetBrains Mono Regular 9px — chips/pills

    // ========================================================================
    // Typography scale (compact)
    // ========================================================================
    namespace size {
        inline constexpr float display = 17.f;   // logo / product name
        inline constexpr float label   = 12.f;   // card headers, section titles
        inline constexpr float body    = 12.f;   // toggles, options
        inline constexpr float mono    = 11.f;   // slider values, stats
        inline constexpr float mono_sm = 9.f;    // chips, pills, version
        inline constexpr float icon    = 13.f;   // FA6 icon size
        inline constexpr float tab_icon = 16.f;  // sidebar tab icons
        inline constexpr float uppercase = 8.f;  // section headers (uppercase)
    }

    // ========================================================================
    // Font loading with FreeType rendering (sharper text)
    // First tries embedded Syne/JetBrains, falls back to Inter
    // ========================================================================
    inline bool load(float dpiScale = 1.0f) {
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();

        ImFontConfig cfg;
        cfg.PixelSnapH       = true;
        cfg.OversampleH      = 2;
        cfg.OversampleV      = 2;
        cfg.FontLoaderFlags  = ImGuiFreeTypeLoaderFlags_LightHinting;
        cfg.RasterizerMultiply = 1.0f;

        // FontAwesome glyph ranges (16-bit Private Use Area)
        static const ImWchar fa_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };

        ImFontConfig fa_cfg;
        fa_cfg.MergeMode        = true;
        fa_cfg.PixelSnapH       = true;
        fa_cfg.GlyphMinAdvanceX = 14.f;
        fa_cfg.OversampleH      = 1;
        fa_cfg.OversampleV      = 1;

        auto loadFont = [&](const unsigned char* data, unsigned int size,
                            float px, ImFont** out, bool mergeFA) -> ImFont* {
            cfg.MergeMode = false;
            cfg.FontDataOwnedByAtlas = false;
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

        // ========================================================================
        // Syne — UI display fonts
        // ========================================================================

        // g_display: Syne ExtraBold 20px — logo (no FA merge)
        loadFont(font_syne_extrabold, font_syne_extrabold_size,
            size::display, &g_display, false);

        // g_label: Syne Medium 13px — card headers (with FA merge)
        loadFont(font_syne_medium, font_syne_medium_size,
            size::label, &g_label, true);

        // g_body: Syne Regular 13px — body text (with FA merge)
        loadFont(font_syne_regular, font_syne_regular_size,
            size::body, &g_body, true);

        // ========================================================================
        // JetBrains Mono — monospace values
        // ========================================================================

        // g_mono: JetBrains Mono Regular 12px — slider values, stats
        loadFont(font_jetbrains_mono_regular, font_jetbrains_mono_regular_size,
            size::mono, &g_mono, false);

        // g_mono_sm: JetBrains Mono Regular 10px — chips, pills, version
        loadFont(font_jetbrains_mono_regular, font_jetbrains_mono_regular_size,
            size::mono_sm, &g_mono_sm, false);

        // Fallbacks
        if (!g_display) g_display = g_label ? g_label : io.Fonts->AddFontDefault();
        if (!g_label)   g_label   = g_body ? g_body : io.Fonts->AddFontDefault();
        if (!g_body)    g_body    = io.Fonts->AddFontDefault();
        if (!g_mono)    g_mono    = g_body;
        if (!g_mono_sm) g_mono_sm = g_mono;

        io.Fonts->Build();
        return true;
    }

    // ========================================================================
    // Accessors (with null-safety)
    // ========================================================================
    inline ImFont* display() { return g_display ? g_display : ImGui::GetFont(); }
    inline ImFont* label()   { return g_label   ? g_label   : ImGui::GetFont(); }
    inline ImFont* body()    { return g_body    ? g_body    : ImGui::GetFont(); }
    inline ImFont* mono()    { return g_mono    ? g_mono    : ImGui::GetFont(); }
    inline ImFont* mono_sm() { return g_mono_sm ? g_mono_sm : ImGui::GetFont(); }

    // Backwards compatibility
    inline ImFont* regular() { return body(); }
    inline ImFont* medium()  { return label(); }
    inline ImFont* bold()    { return label(); }
    inline ImFont* logo()    { return display(); }

} // namespace font
