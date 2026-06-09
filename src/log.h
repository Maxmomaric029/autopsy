#pragma once
#include <windows.h>
#include <cstdio>
#include <ctime>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

// ============================================================================
// MISERABLE Console UI - Animated Header, Credits, TOS, Box-Drawing Menu
// ============================================================================

namespace console {

    inline HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    inline bool initialized = false;
    inline bool menuVisible = true;
    inline int selectedOption = 0;
    inline float hue = 0.0f;

    // ========================================================================
    // ANSI RGB helpers
    // ========================================================================
    inline void rgb(int r, int g, int b) {
        printf("\033[38;2;%d;%d;%dm", r, g, b);
    }

    inline void rgb_bg(int r, int g, int b) {
        printf("\033[48;2;%d;%d;%dm", r, g, b);
    }

    inline void reset() {
        printf("\033[0m");
    }

    inline void cls() {
        printf("\033[2J\033[H");
    }

    // ========================================================================
    // HSV to RGB for rainbow cycling
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
    // Gradient text - writes text with a horizontal gradient
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
    // ASCII Art: MISERABLE
    // ========================================================================
    inline void draw_header(float hue_offset = 0.0f) {
        const char* art[] = {
            "  MM   MM IIIII SSSSS EEEEE RRRR   AAA  BBBB  L     EEEEE",
            "  M M M M  I   S     E     R   R A   A B   B L     E    ",
            "  M  M  M  I   SSSSS EEEE  RRRR  AAAAA BBBB  L     EEEE ",
            "  M     M  I       S E     R  R  A   A B   B L     E    ",
            "  M     M IIIII SSSSS EEEEE R   R A   A BBBB  LLLLL EEEEE",
            "                                                        ",
            "     ╔══════════════════════════════════════════════╗      ",
            "     ║       Bloodie & Xchino presents...          ║      ",
            "     ╚══════════════════════════════════════════════╝      "
        };

        int lines = sizeof(art) / sizeof(art[0]);
        for (int i = 0; i < lines; i++) {
            printf("  ");
            float h = hue_offset + ((float)i / (float)lines) * 0.3f;
            if (h > 1.0f) h -= 1.0f;
            gradient_text(art[i], h, 0.25f);
            printf("\n");
        }
        printf("\n");
    }

    // ========================================================================
    // TOS
    // ========================================================================
    inline void draw_tos() {
        const char* border_top    = "  ╔══════════════════════════════════════════════════════════════╗";
        const char* border_bottom = "  ╚══════════════════════════════════════════════════════════════╝";
        const char* sep           = "  ╠══════════════════════════════════════════════════════════════╣";

        rgb(100, 117, 255); printf("%s\n", border_top); reset();
        rgb(180, 140, 255); printf("  ║                      TERMS OF SERVICE                        ║\n"); reset();
        rgb(100, 117, 255); printf("%s\n", sep); reset();

        const char* tos_lines[] = {
            "  ║  \x95  This software is FREE and provided for EDUCATIONAL purposes  ║",
            "  ║  \x95  Use at your own risk. The creators assume NO liability.       ║",
            "  ║  \x95  You MAY be banned from games/services while using this.        ║",
            "  ║  \x95  Reselling or distributing as paid software is STRICTLY banned. ║",
            "  ║  \x95  Educational/security research purposes ONLY.                   ║",
        };

        rgb(180, 200, 220);
        for (auto& line : tos_lines) {
            printf("%s\n", line);
        }
        reset();

        rgb(100, 117, 255); printf("%s\n", border_bottom); reset();
        printf("\n");
    }

    // ========================================================================
    // Box-Drawing Menu
    // ========================================================================
    inline void draw_menu_item(const char* label, bool selected, bool enabled = true) {
        if (selected) {
            rgb(0, 174, 255);
            printf("  ║  \xFE  ");
            rgb(255, 255, 255);
            printf("%-47s", label);
            rgb(0, 174, 255);
            printf("  ║");
        }
        else if (enabled) {
            rgb(146, 166, 178);
            printf("  ║     ");
            rgb(180, 200, 220);
            printf("%-47s", label);
            rgb(146, 166, 178);
            printf("  ║");
        }
        else {
            rgb(86, 101, 114);
            printf("  ║     ");
            printf("%-47s", label);
            printf("  ║");
        }
        reset();
        printf("\n");
    }

    inline void draw_menu() {
        const char* menu_title = "  ╔══════════════════════  M E N U  ═══════════════════════╗";
        const char* menu_bot   = "  ╚════════════════════════════════════════════════════════╝";

        rgb(100, 117, 255); printf("%s\n", menu_title); reset();

        const char* items[] = {
            "Camera Settings",
            "Lights Settings",
            "Players Settings",
            "ESP Settings",
            "Aimbot Settings",
            "Silent Aim Settings",
            "World Settings",
            "Miscellaneous",
            "Config Manager"
        };
        int count = sizeof(items) / sizeof(items[0]);

        for (int i = 0; i < count; i++) {
            draw_menu_item(items[i], i == selectedOption, true);
            if (i < count - 1) {
                rgb(60, 75, 90); printf("  ║  \xFA                                            \xFA  ║\n"); reset();
            }
        }

        rgb(100, 117, 255); printf("%s\n", menu_bot); reset();
        printf("\n");
        rgb(146, 166, 178);
        printf("  [\x18\x19] Navigate  [Enter] Select  [INS] Toggle Menu\n");
        reset();
    }

    // ========================================================================
    // Init - prepare console
    // ========================================================================
    inline void init() {
        if (initialized) return;

        // Enable VT100
        DWORD mode = 0;
        if (GetConsoleMode(hConsole, &mode)) {
            mode |= 0x0004;
            SetConsoleMode(hConsole, mode);
        }

        // Hide cursor
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);

        // Set console title
        SetConsoleTitleA("miserable");

        initialized = true;
        cls();
    }

    // ========================================================================
    // Full render
    // ========================================================================
    inline void render(float time) {
        cls();
        hue = fmodf(time * 0.05f, 1.0f);
        draw_header(hue);
        draw_tos();
        draw_menu();
    }

    // ========================================================================
    // Input handling thread
    // ========================================================================
    inline void input_thread() {
        const int item_count = 9;
        while (true) {
            if (GetAsyncKeyState(VK_UP) & 0x0001) {
                selectedOption = (selectedOption - 1 + item_count) % item_count;
            }
            if (GetAsyncKeyState(VK_DOWN) & 0x0001) {
                selectedOption = (selectedOption + 1) % item_count;
            }
            if (GetAsyncKeyState(VK_RETURN) & 0x0001) {
                // Selection handled by main menu
                Beep(800, 50);
            }
            if (GetAsyncKeyState(VK_INSERT) & 0x0001) {
                menuVisible = !menuVisible;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    // ========================================================================
    // Console render thread
    // ========================================================================
    inline void render_loop() {
        while (true) {
            if (menuVisible) {
                float time = (float)std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count() / 1000.0f;
                render(time);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    // ========================================================================
    // Start console UI threads
    // ========================================================================
    inline void start() {
        init();
        std::thread(input_thread).detach();
        std::thread(render_loop).detach();
    }

} // namespace console

// ============================================================================
// Legacy output namespace - simplified, no debug (kept for compatibility)
// ============================================================================
namespace output {

    inline void info(const char*, ...) {}
    inline void warn(const char*, ...) {}
    inline void error(const char*, ...) {}
    inline void ok(const char*, ...) {}
    inline void core(const char*, const char*, ...) {}

} // namespace output
