#pragma once
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/misc/imgui_freetype.h>
#include "FontAwesome/IconsFontAwesome6.h"

// Embedded font data
#include "../embedded/font_inter_regular.h"
#include "../embedded/font_inter_semibold.h"
#include "../embedded/font_inter_bold.h"
#include "../embedded/font_inter_black.h"
#include "../embedded/font_fa_solid.h"

namespace font {

    // ========================================================================
    // Font pointers — Syne for display, JetBrains Mono for values
    // ========================================================================
    inline ImFont* g_display  = nullptr; // Syne ExtraBold 20px — logo
    inline ImFont* g_label    = nullptr; // Syne Medium 13px — card headers
    inline ImFont* g_body     = nullptr; // Syne Regular 13px — toggles/options
    inline ImFont* g_mono     = nullptr; // JetBrains Mono Regular 12px — values
    inline ImFont* g_mono_sm  = nullptr; // JetBrains Mono Regular 10px — chips/pills

    // Fallback fonts (Inter) for when Syne/JetBrains aren't embedded yet
    inline ImFont* g_fallback_regular = nullptr;
    inline ImFont* g_fallback_medium  = nullptr;
    inline ImFont* g_fallback_bold    = nullptr;
    inline ImFont* g_fallback_mono    = nullptr;
    inline ImFont* g_fallback_logo    = nullptr;

    // ========================================================================
    // Typography scale
    // ========================================================================
    namespace size {
        inline constexpr float display = 20.f;   // logo / product name
        inline constexpr float label   = 13.f;   // card headers, section titles
        inline constexpr float body    = 13.f;   // toggles, options
        inline constexpr float mono    = 12.f;   // slider values, stats
        inline constexpr float mono_sm = 10.f;   // chips, pills, version
        inline constexpr float icon    = 14.f;   // FA6 icon size
        inline constexpr float tab_icon = 18.f;  // sidebar tab icons
        inline constexpr float uppercase = 9.f;  // section headers (uppercase)
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

        // Load Syne fonts (currently fallback to Inter until Syne TTFs are embedded)
        // When Syne fonts are available, use:
        //   loadFont(font_syne_extrabold, ..., &g_display, false);
        //   loadFont(font_syne_medium, ..., &g_label, true);
        //   loadFont(font_syne_regular, ..., &g_body, true);
        //
        // For now, use Inter as fallback:

        // g_display: Syne ExtraBold 20px → Inter Black 20px
        g_fallback_logo = loadFont(font_inter_black, font_inter_black_size,
            size::display, &g_display, false);

        // g_label: Syne Medium 13px → Inter SemiBold 13px
        g_fallback_medium = loadFont(font_inter_semibold, font_inter_semibold_size,
            size::body, &g_label, true);

        // g_body: Syne Regular 13px → Inter Regular 13px
        g_fallback_regular = loadFont(font_inter_regular, font_inter_regular_size,
            size::body, &g_body, true);

        // g_mono: JetBrains Mono 12px → Inter Regular 12px
        g_fallback_mono = loadFont(font_inter_regular, font_inter_regular_size,
            size::mono, &g_mono, false);

        // g_mono_sm: JetBrains Mono 10px → Inter Regular 10px
        // Reuse g_mono font at smaller size if available
        if (g_mono) {
            g_mono_sm = g_mono;
        } else {
            cfg.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_LightHinting;
            cfg.MergeMode = false;
            cfg.FontDataOwnedByAtlas = false;
            g_mono_sm = io.Fonts->AddFontFromMemoryTTF(
                (void*)font_inter_regular, (int)font_inter_regular_size,
                size::mono_sm * dpiScale, &cfg);
        }

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
