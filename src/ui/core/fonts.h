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
    // Resolve font path — checks exe directory first, then falls back to
    // project root (for development builds that still have Dependencies/)
    // Exe at: .../Build/miserable.exe
    // Fonts at: .../Build/Fonts/Inter/...  (post-build copy)
    // ========================================================================
    inline std::string resolvePath(const char* relative) {
        char exePath[MAX_PATH];
        GetModuleFileNameA(nullptr, exePath, MAX_PATH);
        std::string p(exePath);
        auto pos = p.find_last_of("\\/");
        if (pos != std::string::npos) {
            p = p.substr(0, pos); // Build/
        }
        return p + "\\" + relative;
    }

    // ========================================================================
    // Try to load a TTF and merge FontAwesome icons
    // ========================================================================
    inline bool load(float dpiScale = 1.0f) {
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();

        // Safety fallback — always have something
        g_regular = g_medium = g_bold = g_mono = g_logo = io.Fonts->AddFontDefault();

        // ---- Resolve paths — try exe dir first, then project root ----
        auto exeDir = resolvePath("");

        // Fallback: exe is at Build/, project root is one level up, Dependencies/ is there
        auto rootDir = exeDir + "..\\Dependencies\\";

        auto tryPath = [&](const char* sub) -> std::string {
            // First check: fonts copied next to exe via post-build
            std::string r1 = exeDir + sub;
            if (GetFileAttributesA(r1.c_str()) != INVALID_FILE_ATTRIBUTES)
                return r1;
            // Fallback: Dependencies/ at project root (dev builds)
            return rootDir + sub;
        };

        // On deployed builds:  Build/fonts/Inter/...   Build/fonts/fa-solid-900.ttf
        // On dev builds:        Dependencies/Fonts/Inter/...  Dependencies/FontAwesome/...
        std::string interRegular = tryPath("fonts\\Inter\\Inter-Regular.ttf");
        std::string interSemiBld = tryPath("fonts\\Inter\\Inter-SemiBold.ttf");
        std::string interBold    = tryPath("fonts\\Inter\\Inter-Bold.ttf");
        std::string interBlack   = tryPath("fonts\\Inter\\Inter-Black.ttf");
        std::string faSolid      = tryPath("fonts\\fa-solid-900.ttf");

        ImFontConfig cfg;
        cfg.PixelSnapH = false;
        cfg.OversampleH = 2;
        cfg.OversampleV = 2;

        auto loadWithIcons = [&](const std::string& path, float size, ImFont** out, bool mergeFA) -> ImFont* {
            if (GetFileAttributesA(path.c_str()) == INVALID_FILE_ATTRIBUTES) return nullptr;
            cfg.MergeMode = false;
            ImFont* fnt = io.Fonts->AddFontFromFileTTF(path.c_str(), size * dpiScale, &cfg);
            if (fnt && mergeFA && GetFileAttributesA(faSolid.c_str()) != INVALID_FILE_ATTRIBUTES) {
                cfg.MergeMode = true;
                static const ImWchar faRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
                io.Fonts->AddFontFromFileTTF(faSolid.c_str(), size * dpiScale, &cfg, faRanges);
                cfg.MergeMode = false;
            }
            if (out) *out = fnt;
            return fnt;
        };

        loadWithIcons(interRegular, 14.f, &g_regular, true);
        loadWithIcons(interSemiBld, 14.f, &g_medium, true);
        loadWithIcons(interBold, 16.f, &g_bold, true);
        loadWithIcons(interBlack, 24.f, &g_logo, false);
        loadWithIcons(interRegular, 13.f, &g_mono, false);

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
