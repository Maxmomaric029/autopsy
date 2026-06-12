#pragma once
#include <windows.h>
#include <cstdio>
#include <string>
#include "global.h"

// ============================================================================
// MISERABLE Console — Static offsets presentation (black + blood red)
// ============================================================================

namespace console {

    inline HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    inline bool initialized = false;

    // Status info
    inline int playerCount = 0;
    inline bool connected = false;
    inline uintptr_t cameraAddress = 0;

    // ========================================================================
    // Console helpers
    // ========================================================================
    inline void red() {
        printf("\033[91m");
    }

    inline void darkred() {
        printf("\033[31m");
    }

    inline void brightred() {
        printf("\033[1;31m");
    }

    inline void gray() {
        printf("\033[90m");
    }

    inline void reset() {
        printf("\033[0m");
    }

    inline void cls() {
        printf("\033[2J\033[H");
    }

    inline void hide_cursor() {
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }

    inline void set_bg_black() {
        printf("\033[40m");
    }

    // ========================================================================
    // Draw an offset line
    // ========================================================================
    inline void offset_line(const char* label, uintptr_t value, int w) {
        gray();
        printf("  |  ");
        reset();
        darkred();
        printf("%-20s", label);
        reset();
        brightred();
        if (value)
            printf("0x%08llX", (unsigned long long)value);
        else
            printf("%-10s", "N/A");
        reset();
        // Fill remaining
        int used = 4 + 20 + 12;
        if (used < w) {
            gray();
            for (int i = used; i < w - 3; i++) putchar(' ');
            printf(" |");
        } else {
            gray();
            printf("  |");
        }
        reset();
        printf("\n");
    }

    // ========================================================================
    // Status line with blood red coloring
    // ========================================================================
    inline void status_line(const char* label, const char* value, int w, bool highlight = false) {
        gray();
        printf("  |  ");
        reset();
        darkred();
        printf("%-20s", label);
        reset();
        if (highlight) brightred();
        else red();
        printf("%-16s", value);
        reset();
        gray();
        printf("  |");
        reset();
        printf("\n");
    }

    // ========================================================================
    // Section header
    // ========================================================================
    inline void section_header(const char* title, int w) {
        brightred();
        printf("  |  ");
        red();
        printf(">> %s", title);
        reset();
        int used = 5 + 2 + (int)strlen(title);
        if (used < w - 3) {
            gray();
            for (int i = used; i < w - 3; i++) putchar(' ');
            printf(" |");
        } else {
            gray();
            printf("  |");
        }
        reset();
        printf("\n");
    }

    // ========================================================================
    // Draw box characters
    // ========================================================================
    inline void draw_hline(int len) {
        for (int i = 0; i < len; i++) putchar('-');
    }

    inline void draw_box_top(int w) {
        putchar('+');
        draw_hline(w - 2);
        putchar('+');
    }

    inline void draw_box_bottom(int w) {
        putchar('+');
        draw_hline(w - 2);
        putchar('+');
    }

    inline void draw_box_sep(int w) {
        putchar('+');
        draw_hline(w - 2);
        putchar('+');
    }

    // ========================================================================
    // Draw "MISERABLE" ASCII header
    // ========================================================================
    inline void draw_header() {
        // Simple ASCII header — no Unicode, always renders
        brightred();
        printf("\n");
        printf("    M M M   III   SSS   EEEEE  RRRR    AAA   BBBB   L      EEEEE\n");
        printf("    M   M    I   S      E      R   R  A   A  B   B  L      E    \n");
        printf("    M M M    I    SSS   EEEE   RRRR   AAAAA  BBBB   L      EEEE \n");
        printf("    M   M    I       S  E      R  R   A   A  B   B  L      E    \n");
        printf("    M   M   III   SSS   EEEEE  R   R  A   A  BBBB   LLLLL  EEEEE\n");
        printf("\n");
        reset();
    }

    // ========================================================================
    // Full render
    // ========================================================================
    inline void render(bool clearFirst = true) {
        if (clearFirst) cls();
        const int w = 60;

        draw_header();
        printf("\n");

        brightred();
        printf("  ");
        draw_box_top(w);
        reset();
        printf("\n");

        brightred();
        printf("  |  MISERABLE - OFFSETS MONITOR              |\n");
        printf("  ");
        draw_box_sep(w);
        reset();
        printf("\n");

        section_header("GAME", w);
        char gameIdStr[32];
        snprintf(gameIdStr, sizeof(gameIdStr), "%llu", (unsigned long long)global::GameID);
        status_line("GameID", gameIdStr, w);

        status_line("Status", global::camera.Address ? "Connected" : "Waiting", w,
            global::camera.Address ? true : false);

        char playerStrBuf[32];
        snprintf(playerStrBuf, sizeof(playerStrBuf), "%d", console::playerCount);
        status_line("Players", playerStrBuf, w);

        section_header("CAMERA", w);
        offset_line("Camera Address", global::camera.Address, w);
        offset_line("Camera Subject", offset::camera::CameraSubject, w);
        offset_line("FieldOfView", offset::camera::FieldOfView, w);

        section_header("PLAYERS", w);
        offset_line("LocalPlayer Addr", global::LocalPlayer.player.Address, w);
        offset_line("Local Character", global::LocalPlayer.character.Address, w);
        offset_line("HumanoidRootPart", global::LocalPlayer.HumanoidRootPart.Address, w);
        offset_line("Model offset", offset::player::ModelInstance, w);
        offset_line("DisplayName offset", offset::player::DisplayName, w);

        section_header("LIGHTING", w);
        offset_line("Light Address", global::light.Address, w);
        offset_line("Brightness offset", offset::light::Brightness, w);
        offset_line("FogEnd offset", offset::light::FogEnd, w);
        offset_line("ClockTime offset", offset::light::ClockTime, w);

        section_header("CORE", w);
        offset_line("Model Address", global::model.Address, w);
        offset_line("Workspace Addr", global::workspace.Address, w);
        offset_line("Render Address", global::render.Address, w);
        offset_line("View Address", global::view.Address, w);

        section_header("AIMBOT / SILENT", w);
        offset_line("FakeModel Ptr", offset::fakemodel::Pointer, w);
        offset_line("Render Ptr", offset::render::Pointer, w);
        offset_line("Task Ptr", offset::task::Pointer, w);

        brightred();
        printf("  ");
        draw_box_bottom(w);
        printf("\n");
        reset();

        // Print real-time Console Logs below the monitor box
        printf("\n  ");
        brightred();
        printf(">> CONSOLE LOGS\n");
        reset();

        {
            std::lock_guard<std::mutex> lock(logMutex);
            if (logLines.empty()) {
                gray();
                printf("     No log events recorded.\n");
                reset();
            } else {
                for (const auto& line : logLines) {
                    printf("     %s\n", line.c_str());
                }
            }
        }
        printf("\n");

        gray();
        printf("  [INS] Toggle overlay  [END] Exit\n");
        reset();
    }

    // ========================================================================
    // Logging functions for real-time console feedback
    // ========================================================================
    inline std::vector<std::string> logLines;
    inline std::mutex logMutex;

    inline void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        logLines.push_back(message);
        if (logLines.size() > 8) { // Keep last 8 lines
            logLines.erase(logLines.begin());
        }
    }

    inline void log_formatted(const char* prefix, const char* colorCode, const char* fmt, va_list args) {
        char buf[1024];
        vsnprintf(buf, sizeof(buf), fmt, args);
        
        SYSTEMTIME lt;
        GetLocalTime(&lt);
        char timeBuf[32];
        snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", lt.wHour, lt.wMinute, lt.wSecond);

        std::string formatted = std::string("\033[90m[") + timeBuf + "] " + colorCode + prefix + "\033[0m " + buf;
        log(formatted);
        
        // Trigger a redraw if initialized
        if (initialized) {
            refresh();
        }
    }

    inline void info(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log_formatted("[INFO]", "\033[94m", fmt, args); // Blue prefix
        va_end(args);
    }

    inline void success(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log_formatted("[OK]", "\033[92m", fmt, args); // Green prefix
        va_end(args);
    }

    inline void warn(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log_formatted("[WARN]", "\033[93m", fmt, args); // Yellow prefix
        va_end(args);
    }

    inline void error(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log_formatted("[FAIL]", "\033[91m", fmt, args); // Red prefix
        va_end(args);
    }

    // ========================================================================
    // Refresh — auto-re-render when offsets change (must be after render())
    // ========================================================================
    inline void refresh() {
        COORD top = { 0, 0 };
        SetConsoleCursorPosition(hConsole, top);
        render(false);
    }

    // ========================================================================
    // Init
    // ========================================================================
    inline void init() {
        if (initialized) return;

        DWORD mode = 0;
        if (GetConsoleMode(hConsole, &mode)) {
            mode |= 0x0004; // ENABLE_VIRTUAL_TERMINAL_PROCESSING
            SetConsoleMode(hConsole, mode);
        }

        hide_cursor();

        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        csbi.dwSize.Y = 800;
        SetConsoleScreenBufferSize(hConsole, csbi.dwSize);

        // Black background
        set_bg_black();
        system("color 00");

        SetConsoleTitleA("miserable - offsets monitor");

        initialized = true;
    }

    // ========================================================================
    // Start — render offsets ONCE, no refresh loop
    // ========================================================================
    inline void start() {
        init();
        render();
    }

} // namespace console
