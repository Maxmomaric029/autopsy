#pragma once
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/misc/imgui_freetype.h>
#include "FontAwesome/IconsFontAwesome6.h"
#include <cstdio>
#include <string>
#include <array>

namespace font {

    // Exposed font pointers
    inline ImFont* g_regular = nullptr;  // Body 14px (Inter Regular + FA6 merged)
    inline ImFont* g_medium  = nullptr;  // Medium 14px (Inter SemiBold + FA6 merged)
    inline ImFont* g_bold    = nullptr;  // Bold 16px (Inter Bold + FA6 merged)
    inline ImFont* g_mono    = nullptr;  // Mono 13px (Inter Regular)
    inline ImFont* g_logo    = nullptr;  // Logo/Header 24px (Inter Black)

    // ========================================================================
    // Resolve font path relative to executable directory
    // Exe at: .../Build/miserable.exe → baseDir = .../
    // ========================================================================
    inline std::string resolvePath(const char* relative) {
        char exePath[MAX_PATH];
        GetModuleFileNameA(nullptr, exePath, MAX_PATH);
        std::string p(exePath);
        auto pos = p.find_last_of("\\/");
        if (pos != std::string::npos) {
            p = p.substr(0, pos); // Build/
            auto pos2 = p.find_last_of("\\/");
            if (pos2 != std::string::npos) {
                p = p.substr(0, pos2 + 1); // project root (trailing separator)
            }
        }
        return p + relative;
    }

    // ========================================================================
    // Load all fonts with DPI-aware sizing, FreeType, and FontAwesome 6 icons
    // ========================================================================
    inline bool load(float dpiScale = 1.0f) {
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();

        // Safety fallback: load default font first so we always have something
        ImFont* defaultFallback = io.Fonts->AddFontDefault();
        g_regular = g_medium = g_bold = g_mono = g_logo = defaultFallback;

        // Use FreeType for superior anti-aliasing (applies to custom fonts below)
        const unsigned int ftFlags =
            ImGuiFreeTypeLoaderFlags_LoadColor |
            ImGuiFreeTypeLoaderFlags_LightHinting;
        io.Fonts->SetFontLoader(ImGuiFreeType::GetFontLoader());
        io.Fonts->FontLoaderFlags = ftFlags;

        // Resolve absolute paths from executable location
        std::string interRegular = resolvePath("Dependencies\\Fonts\\Inter\\Inter-Regular.ttf");
        std::string interSemiBld = resolvePath("Dependencies\\Fonts\\Inter\\Inter-SemiBold.ttf");
        std::string interBold    = resolvePath("Dependencies\\Fonts\\Inter\\Inter-Bold.ttf");
        std::string interBlack   = resolvePath("Dependencies\\Fonts\\Inter\\Inter-Black.ttf");
        std::string faSolid      = resolvePath("Dependencies\\FontAwesome\\fa-solid-900.ttf");
        std::string faRegular    = resolvePath("Dependencies\\FontAwesome\\fa-regular-400.ttf");

        auto fileExists = [](const std::string& p) -> bool {
            return GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES;
        };

        // ---- Base config ----
        ImFontConfig cfg;
        cfg.PixelSnapH = false;
        cfg.OversampleH = 2;
        cfg.OversampleV = 2;
        cfg.RasterizerMultiply = 1.02f;
        cfg.FontLoaderFlags = 0;

        // ---- 1. Body: Inter Regular 14px + FontAwesome Solid merged ----
        if (fileExists(interRegular)) {
            cfg.MergeMode = false;
            g_regular = io.Fonts->AddFontFromFileTTF(interRegular.c_str(), 14.0f * dpiScale, &cfg);
            if (g_regular && fileExists(faSolid)) {
                cfg.MergeMode = true;
                cfg.GlyphMinAdvanceX = 14.0f;
                cfg.GlyphOffset = { 0.f, 1.f };
                static const ImWchar faSolidRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
                io.Fonts->AddFontFromFileTTF(faSolid.c_str(), 14.0f * dpiScale, &cfg, faSolidRanges);
                cfg.MergeMode = false;
                cfg.GlyphMinAdvanceX = 0.f;
                cfg.GlyphOffset = { 0.f, 0.f };
            }
        }

        // ---- 2. Body Medium: Inter SemiBold 14px + FontAwesome Solid merged ----
        if (fileExists(interSemiBld)) {
            cfg.MergeMode = false;
            g_medium = io.Fonts->AddFontFromFileTTF(interSemiBld.c_str(), 14.0f * dpiScale, &cfg);
            if (g_medium && fileExists(faSolid)) {
                cfg.MergeMode = true;
                cfg.GlyphMinAdvanceX = 14.0f;
                cfg.GlyphOffset = { 0.f, 1.f };
                static const ImWchar faSolidRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
                io.Fonts->AddFontFromFileTTF(faSolid.c_str(), 14.0f * dpiScale, &cfg, faSolidRanges);
                cfg.MergeMode = false;
                cfg.GlyphMinAdvanceX = 0.f;
                cfg.GlyphOffset = { 0.f, 0.f };
            }
        }

        // ---- 3. Bold/Tabs: Inter Bold 16px + FontAwesome Solid merged ----
        if (fileExists(interBold)) {
            cfg.MergeMode = false;
            cfg.OversampleH = 3;
            cfg.OversampleV = 3;
            g_bold = io.Fonts->AddFontFromFileTTF(interBold.c_str(), 16.0f * dpiScale, &cfg);
            if (g_bold && fileExists(faSolid)) {
                cfg.MergeMode = true;
                cfg.GlyphMinAdvanceX = 16.0f;
                cfg.GlyphOffset = { 0.f, 1.f };
                static const ImWchar faSolidRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
                io.Fonts->AddFontFromFileTTF(faSolid.c_str(), 16.0f * dpiScale, &cfg, faSolidRanges);
                cfg.MergeMode = false;
                cfg.GlyphMinAdvanceX = 0.f;
                cfg.GlyphOffset = { 0.f, 0.f };
            }
            cfg.OversampleH = 2;
            cfg.OversampleV = 2;
        }

        // ---- 4. Logo/Header: Inter Black 24px ----
        if (fileExists(interBlack)) {
            cfg.MergeMode = false;
            cfg.OversampleH = 3;
            cfg.OversampleV = 3;
            g_logo = io.Fonts->AddFontFromFileTTF(interBlack.c_str(), 24.0f * dpiScale, &cfg);
            cfg.OversampleH = 2;
            cfg.OversampleV = 2;
        }

        // ---- 5. Mono: Inter Regular 13px ----
        if (fileExists(interRegular)) {
            cfg.MergeMode = false;
            g_mono = io.Fonts->AddFontFromFileTTF(interRegular.c_str(), 13.0f * dpiScale, &cfg);
        }

        // ---- 6. Fallbacks ----
        if (!g_regular) g_regular = io.Fonts->AddFontDefault();
        if (!g_medium)  g_medium  = g_regular;
        if (!g_bold)    g_bold    = g_regular;
        if (!g_mono)    g_mono    = g_regular;
        if (!g_logo)    g_logo    = g_bold;

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
