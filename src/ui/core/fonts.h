#pragma once
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_win32.h>
#include "FontAwesome/IconsFontAwesome6.h"
#include <cstdio>
#include <string>

namespace font {

    inline ImFont* g_regular = nullptr;
    inline ImFont* g_medium  = nullptr;
    inline ImFont* g_bold    = nullptr;
    inline ImFont* g_mono    = nullptr;
    inline ImFont* g_logo    = nullptr;

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
            p = p.substr(0, pos);
            auto pos2 = p.find_last_of("\\/");
            if (pos2 != std::string::npos) {
                p = p.substr(0, pos2);
            }
        }
        return p + "\\" + relative;
    }

    // ========================================================================
    // Load all fonts with FontAwesome 6 icons merged
    // ========================================================================
    inline bool load(float dpiScale = 1.0f) {
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();

        // Safety fallback: we always have something
        g_regular = g_medium = g_bold = g_mono = g_logo = io.Fonts->AddFontDefault();

        // Resolve absolute paths
        std::string baseDir = resolvePath("Dependencies");
        std::string interRegular = baseDir + "\\Fonts\\Inter\\Inter-Regular.ttf";
        std::string interSemiBld = baseDir + "\\Fonts\\Inter\\Inter-SemiBold.ttf";
        std::string interBold    = baseDir + "\\Fonts\\Inter\\Inter-Bold.ttf";
        std::string interBlack   = baseDir + "\\Fonts\\Inter\\Inter-Black.ttf";
        std::string faSolid      = baseDir + "\\FontAwesome\\fa-solid-900.ttf";

        auto fileExists = [](const std::string& p) -> bool {
            return GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES;
        };

        ImFontConfig cfg;
        cfg.PixelSnapH = false;
        cfg.OversampleH = 2;
        cfg.OversampleV = 2;

        // Helper to load a font and merge FA6 icons
        auto loadWithIcons = [&](const std::string& path, float size, ImFont** out, bool mergeFA) -> ImFont* {
            if (!fileExists(path)) return nullptr;
            cfg.MergeMode = false;
            ImFont* fnt = io.Fonts->AddFontFromFileTTF(path.c_str(), size * dpiScale, &cfg);
            if (fnt && mergeFA && fileExists(faSolid)) {
                cfg.MergeMode = true;
                cfg.GlyphMinAdvanceX = size;
                cfg.GlyphOffset = { 0.f, 1.f };
                static const ImWchar faRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
                io.Fonts->AddFontFromFileTTF(faSolid.c_str(), size * dpiScale, &cfg, faRanges);
                cfg.MergeMode = false;
                cfg.GlyphMinAdvanceX = 0.f;
                cfg.GlyphOffset = { 0.f, 0.f };
            }
            if (out) *out = fnt;
            return fnt;
        };

        loadWithIcons(interRegular, 14.f, &g_regular, true);
        loadWithIcons(interSemiBld, 14.f, &g_medium, true);
        loadWithIcons(interBold, 16.f, &g_bold, true);
        loadWithIcons(interBlack, 24.f, &g_logo, false);
        loadWithIcons(interRegular, 13.f, &g_mono, false);

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
    inline ImFont* medium()  { return g_medium ? g_medium : regular(); }
    inline ImFont* bold()    { return g_bold ? g_bold : regular(); }
    inline ImFont* mono()    { return g_mono ? g_mono : regular(); }
    inline ImFont* logo()    { return g_logo ? g_logo : bold(); }

} // namespace font
