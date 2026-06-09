#pragma once
#include <windows.h>
#include <cstdio>
#include <ctime>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include "global.h"

// ============================================================================
// AUTOPSY Console UI — Clean offsets presentation (no interactive menu)
// ============================================================================

namespace console {

    inline HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    inline bool initialized = false;
    inline float hue = 0.0f;
    inline bool redraw = true;

    // Status info (set from main thread)
    inline int playerCount = 0;
    inline bool connected = false;

    // ========================================================================
    // ANSI helpers
    // ========================================================================
    inline void rgb(int r, int g, int b) {
        printf("\033[38;2;%d;%d;%dm", r, g, b);
    }

    inline void reset() {
        printf("\033[0m");
    }

    inline void cls() {
        printf("\033[2J\033[H");
    }

    inline void home() {
        printf("\033[H");
    }

    inline void hide_cursor() {
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }

    // ========================================================================
    // HSV to RGB
    // ========================================================================
    inline void hsv_to_rgb(float h, float& r, float& g, float& b) {
        float s = 1.0f, v = 1.0f;
        int i = (int)(h * 6.0f);
        float f = h * 6.0f - i;
        float p = v * (1.0f - s);
        float q = v * (1.0f - s * f);
        float t = v * (1.0f - s * (1.0f - f));
        switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
        }
    }

    // ========================================================================
    // Gradient text
    // ========================================================================
    inline void gradient_text(const char* text, float hue_start, float hue_range) {
        int len = (int)strlen(text);
        for (int i = 0; i < len; i++) {
            float h = hue_start + (hue_range * (float)i / (float)len);
            float r, g, b;
            hsv_to_rgb(h, r, g, b);
            rgb((int)(r * 255), (int)(g * 255), (int)(b * 255));
            putchar(text[i]);
        }
        reset();
    }

    // ========================================================================
    // Box drawing helpers
    // ========================================================================
    inline void draw_hline(int len) {
        for (int i = 0; i < len; i++) putchar(0xCD);
    }

    inline void draw_box_top(int w) {
        putchar(0xC9);
        draw_hline(w - 2);
        putchar(0xBB);
    }

    inline void draw_box_bottom(int w) {
        putchar(0xC0);
        draw_hline(w - 2);
        putchar(0xD9);
    }

    inline void draw_box_sep(int w) {
        putchar(0xCC);
        draw_hline(w - 2);
        putchar(0xB9);
    }

    inline void draw_bar(const char* left, const char* right, int w) {
        printf("  %s ", left);
        int pad = w - 4 - (int)strlen(left) - (int)strlen(right);
        if (pad < 0) pad = 0;
        printf("%*s", pad, "");
        printf("%s  %s", right, left);
    }

    // ========================================================================
    // Draw "AUTOPSY" ASCII header
    // ========================================================================
    inline void draw_header(float hue_offset = 0.0f) {
        const char* header[] = {
            "    ╔═══╗╔════╗╔════╗╔════╗╔═══╗╔════╗╔═══╗╔════╗",
            "    ║╔═╗║║╔╗╔╗║║╔╗╔╗║║╔╗╔╗║║╔══╝║╔╗╔╗║║╔══╝╚═╗╔═╝",
            "    ║╚═╝║║║║║║║╚╝║║╚╝║║║║║║║╚══╗║║║║║║║╚══╗  ║║  ",
            "    ║╔╗╔╝║║║║║║  ║║  ║║║║║║║╔══╝║║║║║║║╔══╝  ║║  ",
            "    ║║║╚╗║║║║║║  ║║  ║║║║║║║╚══╗║║║║║║║╚══╗  ║║  ",
            "    ╚╝╚═╝╚╝╚╝╚╝  ╚╝  ╚╝╚╝╚╝╚═══╝╚╝╚╝╚╝╚═══╝  ╚╝  ",
        };

        for (int i = 0; i < 6; i++) {
            float h = hue_offset + ((float)i / 6.0f) * 0.15f;
            if (h > 1.0f) h -= 1.0f;
            printf("  ");
            gradient_text(header[i], h, 0.15f);
            printf("\n");
        }
    }

    // ========================================================================
    // Draw an offset line
    // ========================================================================
    inline void offset_line(const char* label, uintptr_t value, int w) {
        rgb(60, 75, 90);
        printf("  \xBA  ");
        reset();
        rgb(146, 166, 178);
        printf("%-20s", label);
        reset();
        rgb(0, 174, 255);
        if (value)
            printf("0x%08llX", (unsigned long long)value);
        else
            printf("N/A");
        reset();
        // Fill remaining space
        int used = 4 + 20 + 12;
        if (used < w) {
            rgb(60, 75, 90);
            for (int i = used; i < w - 3; i++) putchar(' ');
            printf(" \xBA");
        } else {
            rgb(60, 75, 90);
            printf("  \xBA");
        }
        reset();
        printf("\n");
    }

    // ========================================================================
    // Draw status line
    // ========================================================================
    inline void status_line(const char* label, const char* value, int w, int r, int g, int b) {
        rgb(60, 75, 90);
        printf("  \xBA  ");
        reset();
        rgb(146, 166, 178);
        printf("%-20s", label);
        reset();
        rgb(r, g, b);
        printf("%-16s", value);
        reset();
        rgb(60, 75, 90);
        printf("  \xBA");
        reset();
        printf("\n");
    }

    // ========================================================================
    // Section header
    // ========================================================================
    inline void section_header(const char* title, int w) {
        rgb(100, 117, 255);
        printf("  \xBA  ");
        rgb(146, 166, 178);
        printf(">> %s", title);
        reset();
        int used = 5 + 2 + (int)strlen(title);
        if (used < w - 3) {
            rgb(60, 75, 90);
            for (int i = used; i < w - 3; i++) putchar(' ');
            printf(" \xBA");
        } else {
            rgb(60, 75, 90);
            printf("  \xBA");
        }
        reset();
        printf("\n");
    }

    // ========================================================================
    // Full render
    // ========================================================================
    inline void render(float time) {
        cls();
        redraw = false;
        hue = fmodf(time * 0.03f, 1.0f);

        const int w = 60;

        // ===== HEADER =====
        draw_header(hue);
        printf("\n");

        // ===== STATUS BAR =====
        rgb(100, 117, 255);
        printf("  ");
        draw_box_top(w);
        reset();
        printf("\n");

        rgb(180, 140, 255);
        printf("  \xBA  AUTOPSY \xBB OFFSETS MONITOR                 \xBA\n");
        rgb(100, 117, 255);
        printf("  ");
        draw_box_sep(w);
        reset();
        printf("\n");

        // ===== GAME INFO =====
        section_header("GAME", w);

        char gameIdStr[32];
        snprintf(gameIdStr, sizeof(gameIdStr), "%llu", (unsigned long long)global::GameID);
        status_line("GameID", gameIdStr, w, 0, 220, 130);

        status_line("Status", global::camera.Address ? "Connected" : "Waiting", w,
            global::camera.Address ? 0 : 255,
            global::camera.Address ? 220 : 80,
            global::camera.Address ? 130 : 80);

        char playerStrBuf[32];
        snprintf(playerStrBuf, sizeof(playerStrBuf), "%d", console::playerCount);
        status_line("Players", playerStrBuf, w, 146, 166, 178);

        // ===== CAMERA =====
        section_header("CAMERA", w);
        offset_line("Camera Address", global::camera.Address, w);
        offset_line("Camera Subject", offset::camera::CameraSubject, w);
        offset_line("FieldOfView", offset::camera::FieldOfView, w);

        // ===== PLAYERS =====
        section_header("PLAYERS", w);
        offset_line("LocalPlayer Addr", global::LocalPlayer.Address, w);
        offset_line("Local Character", global::LocalPlayer.character.Address, w);
        offset_line("HumanoidRootPart", global::LocalPlayer.HumanoidRootPart.Address, w);
        offset_line("Model offset", offset::player::ModelInstance, w);
        offset_line("DisplayName offset", offset::player::DisplayName, w);

        // ===== LIGHTING =====
        section_header("LIGHTING", w);
        offset_line("Light Address", global::light.Address, w);
        offset_line("Brightness offset", offset::light::Brightness, w);
        offset_line("FogEnd offset", offset::light::FogEnd, w);
        offset_line("ClockTime offset", offset::light::ClockTime, w);

        // ===== CORE =====
        section_header("CORE", w);
        offset_line("Model Address", global::model.Address, w);
        offset_line("Workspace Addr", global::workspace.Address, w);
        offset_line("Render Address", global::render.Address, w);
        offset_line("View Address", global::view.Address, w);

        // ===== AIMBOT =====
        section_header("AIMBOT / SILENT", w);
        offset_line("FakeModel Ptr", offset::fakemodel::Pointer, w);
        offset_line("Render Ptr", offset::render::Pointer, w);
        offset_line("Task Ptr", offset::task::Pointer, w);

        // ===== FOOTER =====
        rgb(100, 117, 255);
        printf("  ");
        draw_box_bottom(w);
        printf("\n");
        reset();

        rgb(146, 166, 178);
        printf("  [INS] Toggle overlay  [END] Exit\n");
        reset();
    }

    // ========================================================================
    // Periodically refresh
    // ========================================================================
    inline void refresh_loop() {
        while (true) {
            float time = (float)std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count() / 1000.0f;
            render(time);
            // Wait a bit before refreshing
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }

    // ========================================================================
    // Init
    // ========================================================================
    inline void init() {
        if (initialized) return;

        DWORD mode = 0;
        if (GetConsoleMode(hConsole, &mode)) {
            mode |= 0x0004;
            SetConsoleMode(hConsole, mode);
        }

        hide_cursor();

        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        csbi.dwSize.Y = 800;
        SetConsoleScreenBufferSize(hConsole, csbi.dwSize);

        SetConsoleTitleA("autopsy - offsets monitor");

        initialized = true;
    }

    // ========================================================================
    // Start — no interactive menu, just periodic refresh
    // ========================================================================
    inline void start() {
        init();
        float time = (float)std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count() / 1000.0f;
        render(time);
        std::thread(refresh_loop).detach();
    }

} // namespace console
