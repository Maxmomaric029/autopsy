#pragma once
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/misc/imgui_freetype.h>
#include <cstdio>
#include <string>
#include <array>

namespace font {

    // Exposed font pointers
    inline ImFont* g_regular = nullptr;
    inline ImFont* g_medium  = nullptr;
    inline ImFont* g_bold    = nullptr;
    inline ImFont* g_mono    = nullptr;
    inline ImFont* g_logo    = nullptr;
    inline ImFont* g_icon    = nullptr;

    // Map Icon font codepoints
    namespace icon {
        inline constexpr auto Search    = u8"\uF002";
        inline constexpr auto Gear      = u8"\uF013";
        inline constexpr auto Home      = u8"\uF015";
        inline constexpr auto Crosshair = u8"\uF05B";
        inline constexpr auto Eye       = u8"\uF06E";
        inline constexpr auto Globe     = u8"\uF0AC";
        inline constexpr auto Layers    = u8"\uF0C8";
        inline constexpr auto Diamond   = u8"\uF219";
        inline constexpr auto Save      = u8"\uF0C7";
        inline constexpr auto Folder    = u8"\uF07C";
        inline constexpr auto Trash     = u8"\uF1F8";
        inline constexpr auto Check     = u8"\uF00C";
        inline constexpr auto Close     = u8"\uF00D";
        inline constexpr auto Warning   = u8"\uF071";
        inline constexpr auto Info      = u8"\uF05A";
        inline constexpr auto ChevronR  = u8"\uF054";
        inline constexpr auto ChevronL  = u8"\uF053";
        inline constexpr auto Moon      = u8"\uF186";
        inline constexpr auto Sun       = u8"\uF185";
        inline constexpr auto User      = u8"\uF007";
    }

    // ========================================================================
    // Load all fonts with DPI-aware sizing and FreeType
    // ========================================================================
    inline bool load(float dpiScale = 1.0f) {
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();

        // Use FreeType for all fonts
        const unsigned int ftFlags =
            ImGuiFreeTypeLoaderFlags_LoadColor |
            ImGuiFreeTypeLoaderFlags_LightHinting;
        io.Fonts->SetFontLoader(ImGuiFreeType::GetFontLoader());
        io.Fonts->FontLoaderFlags = ftFlags;

        ImFontConfig cfg;
        cfg.PixelSnapH = false;
        cfg.OversampleH = 3;
        cfg.OversampleV = 3;
        cfg.RasterizerMultiply = 1.02f;
        cfg.FontLoaderFlags = 0;

        // System font paths for fallback
        const char* segoeUi   = "C:\\Windows\\Fonts\\segoeui.ttf";
        const char* segoeSemib = "C:\\Windows\\Fonts\\seguisb.ttf";
        const char* segoeBold = "C:\\Windows\\Fonts\\segoeuib.ttf";
        const char* segoeBlk  = "C:\\Windows\\Fonts\\seguibl.ttf";
        const char* consolas  = "C:\\Windows\\Fonts\\consola.ttf";
        const char* consolasB = "C:\\Windows\\Fonts\\consolab.ttf";
        const char* segoeMono = "C:\\Windows\\Fonts\\cour.ttf"; // Courier New fallback
        const char* fontAwesom = "C:\\Windows\\Fonts\\fa-regular-400.ttf";

        auto fileExists = [](const char* p) -> bool {
            return GetFileAttributesA(p) != INVALID_FILE_ATTRIBUTES;
        };

        // Regular text (14px)
        if (fileExists(segoeUi))
            g_regular = io.Fonts->AddFontFromFileTTF(segoeUi, 14.0f * dpiScale, &cfg);
        // Medium labels (13px)
        if (fileExists(segoeSemib))
            g_medium = io.Fonts->AddFontFromFileTTF(segoeSemib, 13.0f * dpiScale, &cfg);
        else if (fileExists(segoeBold))
            g_medium = io.Fonts->AddFontFromFileTTF(segoeBold, 13.0f * dpiScale, &cfg);
        // Bold / Cards (15px)
        if (fileExists(segoeSemib))
            g_bold = io.Fonts->AddFontFromFileTTF(segoeSemib, 15.0f * dpiScale, &cfg);
        else if (fileExists(segoeBold))
            g_bold = io.Fonts->AddFontFromFileTTF(segoeBold, 15.0f * dpiScale, &cfg);
        // Mono (13px)
        if (fileExists(consolas))
            g_mono = io.Fonts->AddFontFromFileTTF(consolas, 13.0f * dpiScale, &cfg);
        else if (fileExists(consolasB))
            g_mono = io.Fonts->AddFontFromFileTTF(consolasB, 13.0f * dpiScale, &cfg);
        else if (fileExists(segoeMono))
            g_mono = io.Fonts->AddFontFromFileTTF(segoeMono, 13.0f * dpiScale, &cfg);

        ImFontConfig cfg2 = cfg;
        cfg2.OversampleH = 2;
        cfg2.OversampleV = 2;

        // Logo (22px)
        if (fileExists(segoeBlk))
            g_logo = io.Fonts->AddFontFromFileTTF(segoeBlk, 22.0f * dpiScale, &cfg2);
        else if (fileExists(segoeBold))
            g_logo = io.Fonts->AddFontFromFileTTF(segoeBold, 21.0f * dpiScale, &cfg2);

        // Fallbacks if system fonts missing: use embedded fonts from existing project
        if (!g_regular) {
            extern const unsigned char font_regular[];
            extern const int font_regular_len;
            if (font_regular_len > 0)
                g_regular = io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(font_regular), font_regular_len, 14.0f * dpiScale, &cfg);
        }
        if (!g_medium) {
            extern const unsigned char font_bold[];
            extern const int font_bold_len;
            if (font_bold_len > 0)
                g_medium = io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(font_bold), font_bold_len, 13.0f * dpiScale, &cfg);
        }
        if (!g_bold) {
            extern const unsigned char font_bold[];
            extern const int font_bold_len;
            if (font_bold_len > 0)
                g_bold = io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(font_bold), font_bold_len, 15.0f * dpiScale, &cfg2);
        }

        // Ultimate fallback just in case
        if (!g_regular)  g_regular  = io.Fonts->AddFontDefault();
        if (!g_medium)   g_medium   = g_regular;
        if (!g_bold)     g_bold     = g_regular;
        if (!g_mono)     g_mono     = g_regular;
        if (!g_logo)     g_logo     = g_bold;

        // Build font atlas
        io.Fonts->Build();

        return true;
    }

    // Shorthand accessors
    inline ImFont* regular() { return g_regular ? g_regular : ImGui::GetFont(); }
    inline ImFont* medium()  { return g_medium ? g_medium : regular(); }
    inline ImFont* bold()    { return g_bold ? g_bold : regular(); }
    inline ImFont* mono()    { return g_mono ? g_mono : regular(); }
    inline ImFont* logo()    { return g_logo ? g_logo : bold(); }

} // namespace font
