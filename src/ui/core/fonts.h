#pragma once
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/misc/imgui_freetype.h>
#include "FontAwesome/IconsFontAwesome6.h"

// Embedded font data — Inter + JetBrains Mono + FontAwesome
#include "../embedded/files/font_poppins_regular.h"
#include "../embedded/files/font_poppins_semibold.h"
#include "../embedded/files/font_poppins_black.h"
#include "../embedded/files/font_sekuya_regular.h"
#include "../embedded/font_fa_solid.h"

namespace font {

    // ========================================================================
    // Font pointers — Inter for UI, JetBrains Mono for values
    // ========================================================================
    inline ImFont* g_display  = nullptr; // Poppins Black 16px — logo
    inline ImFont* g_label    = nullptr; // Poppins SemiBold 11px — card headers
    inline ImFont* g_body     = nullptr; // Poppins Regular 11px — toggles/options
    inline ImFont* g_mono     = nullptr; // Sekuya Regular 10px — values
    inline ImFont* g_mono_sm  = nullptr; // Sekuya Regular 8px — chips/pills

    // ========================================================================
    // Typography scale (compact, readable)
    // ========================================================================
    namespace size {
        inline constexpr float display = 16.f;   // logo / product name
        inline constexpr float label   = 11.f;   // card headers, section titles
        inline constexpr float body    = 11.f;   // toggles, options
        inline constexpr float mono    = 10.f;   // slider values, stats
        inline constexpr float mono_sm = 8.f;    // chips, pills, version
        inline constexpr float icon    = 12.f;   // FA6 icon size
        inline constexpr float tab_icon = 14.f;  // sidebar tab icons
        inline constexpr float uppercase = 7.f;  // section headers (uppercase)
    }

    // ========================================================================
    // Font loading with FreeType rendering (sharper text)
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
        fa_cfg.GlyphMinAdvanceX = 12.f;
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
        // Poppins — UI display fonts
        // ========================================================================

        // g_display: Poppins Black 18px — logo (no FA merge)
        loadFont(font_poppins_black, font_poppins_black_size,
            size::display, &g_display, false);

        // g_label: Poppins SemiBold 12px — card headers (with FA merge)
        loadFont(font_poppins_semibold, font_poppins_semibold_size,
            size::label, &g_label, true);

        // g_body: Poppins Regular 12px — body text (with FA merge)
        loadFont(font_poppins_regular, font_poppins_regular_size,
            size::body, &g_body, true);

        // ========================================================================
        // Sekuya — display/accent values
        // ========================================================================

        // g_mono: Sekuya Regular 11px — slider values, stats
        loadFont(font_sekuya_regular, font_sekuya_regular_size,
            size::mono, &g_mono, false);

        // g_mono_sm: Sekuya Regular 9px — chips, pills, version
        loadFont(font_sekuya_regular, font_sekuya_regular_size,
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
