#pragma once
#include <windows.h>
#include <cstdio>
#include <string>
#include "global.h"

// ============================================================================
// MISERABLE Console — styled terminal monitor
// ============================================================================

namespace console {

    inline HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    inline bool initialized = false;
    inline std::vector<std::string> logLines;
    inline std::mutex logMutex;
    inline void refresh();

    inline int playerCount = 0;
    inline bool connected  = false;
    inline uintptr_t cameraAddress = 0;

    // ========================================================================
    // ANSI helpers
    // ========================================================================
    inline void col(const char* code) { printf("%s", code); }

    // palette
    #define C_RESET    "\033[0m"
    #define C_RED      "\033[38;2;224;48;64m"      // #E03040 accent
    #define C_RED_DIM  "\033[38;2;140;28;38m"      // dimmed red
    #define C_MILK     "\033[38;2;245;240;235m"     // milk white
    #define C_MILK_DIM "\033[38;2;160;148;140m"     // muted milk
    #define C_GRAY     "\033[38;2;80;66;66m"        // muted dark
    #define C_DARK     "\033[38;2;50;18;18m"        // very dark red-tinted
    #define C_GREEN    "\033[38;2;80;210;120m"
    #define C_YELLOW   "\033[38;2;220;180;60m"
    #define C_BLUE     "\033[38;2;100;160;240m"
    #define C_BG       "\033[40m"

    inline void cls() { printf("\033[2J\033[H"); }

    inline void hide_cursor() {
        CONSOLE_CURSOR_INFO ci; GetConsoleCursorInfo(hConsole, &ci);
        ci.bVisible = false; SetConsoleCursorInfo(hConsole, &ci);
    }

    // ========================================================================
    // Box drawing helpers  (CP65001 / UTF-8)
    // ========================================================================
    //  ┌ ─ ┐ ├ ┤ └ ┘ │ ┬ ┴ ┼
    static const int kW = 68; // total box width incl. borders

    inline void hline(int len, const char* ch = "\xe2\x94\x80") {
        for (int i = 0; i < len; i++) fputs(ch, stdout);
    }

    inline void box_top() {
        col(C_GRAY);
        fputs("  \xe2\x94\x8c", stdout); hline(kW - 2); fputs("\xe2\x94\x90\n", stdout);
        col(C_RESET);
    }
    inline void box_bot() {
        col(C_GRAY);
        fputs("  \xe2\x94\x94", stdout); hline(kW - 2); fputs("\xe2\x94\x98\n", stdout);
        col(C_RESET);
    }
    inline void box_sep() {
        col(C_GRAY);
        fputs("  \xe2\x94\x9c", stdout); hline(kW - 2, "\xe2\x94\x80"); fputs("\xe2\x94\xa4\n", stdout);
        col(C_RESET);
    }
    inline void box_left()  { col(C_GRAY); fputs("  \xe2\x94\x82 ", stdout); col(C_RESET); }
    inline void box_right() { col(C_GRAY); fputs(" \xe2\x94\x82\n", stdout); col(C_RESET); }

    // Pad a string to exactly `n` chars (spaces)
    inline void pad_to(int n, int used) {
        int rem = n - used;
        if (rem > 0) for (int i = 0; i < rem; i++) putchar(' ');
    }

    // ========================================================================
    // Header  (smaller, clean)
    // ========================================================================
    inline void draw_header() {
        const int inner = kW - 2;
        // title row
        box_left();
        col(C_RED);
        const char* title = "MISERABLE";
        int tlen = (int)strlen(title);
        int pad  = (inner - tlen) / 2;
        for (int i = 0; i < pad; i++) putchar(' ');
        fputs(title, stdout);
        pad_to(inner, pad + tlen);
        col(C_RESET);
        box_right();

        // subtitle row
        box_left();
        col(C_GRAY);
        const char* sub = "external cheat framework  //  roblox";
        int slen = (int)strlen(sub);
        int spad = (inner - slen) / 2;
        for (int i = 0; i < spad; i++) putchar(' ');
        fputs(sub, stdout);
        pad_to(inner, spad + slen);
        col(C_RESET);
        box_right();
    }

    // ========================================================================
    // Section label inside box
    // ========================================================================
    inline void section(const char* label) {
        box_sep();
        box_left();
        col(C_RED_DIM); fputs("\xe2\x96\xb8 ", stdout);
        col(C_MILK_DIM);
        printf("%s", label);
        int used = 2 + (int)strlen(label);
        pad_to(kW - 4, used);
        col(C_RESET);
        box_right();
    }

    // ========================================================================
    // Two-column data row
    // ========================================================================
    inline void row(const char* label, const char* value, bool ok = false) {
        box_left();
        col(C_GRAY); printf("  %-22s", label);
        col(ok ? C_GREEN : C_MILK_DIM);
        int vlen = (int)strlen(value);
        fputs(value, stdout);
        pad_to(kW - 4 - 24, vlen);
        col(C_RESET);
        box_right();
    }

    inline void row_addr(const char* label, uintptr_t addr) {
        char buf[32];
        if (addr) snprintf(buf, sizeof(buf), "0x%09llX", (unsigned long long)addr);
        else      snprintf(buf, sizeof(buf), "%-11s", "--");
        box_left();
        col(C_GRAY);    printf("  %-22s", label);
        col(addr ? C_MILK : C_DARK);
        fputs(buf, stdout);
        pad_to(kW - 4 - 24, (int)strlen(buf));
        col(C_RESET);
        box_right();
    }

    inline void row_offset(const char* label, uintptr_t off) {
        char buf[32];
        if (off) snprintf(buf, sizeof(buf), "0x%04llX", (unsigned long long)off);
        else     snprintf(buf, sizeof(buf), "--");
        box_left();
        col(C_GRAY);   printf("  %-22s", label);
        col(off ? C_RED_DIM : C_DARK);
        fputs(buf, stdout);
        pad_to(kW - 4 - 24, (int)strlen(buf));
        col(C_RESET);
        box_right();
    }

    // ========================================================================
    // Empty row (breathing room)
    // ========================================================================
    inline void empty_row() {
        box_left();
        pad_to(kW - 4, 0);
        box_right();
    }

    // ========================================================================
    // Full render
    // ========================================================================
    inline void render(bool clearFirst = true) {
        if (clearFirst) cls();
        printf("\n");

        box_top();
        draw_header();

        // ---- STATUS ----
        section("Status");
        {
            bool conn = global::camera.Address != 0;
            row("Connection", conn ? "active" : "waiting", conn);
            char gb[32]; snprintf(gb, sizeof(gb), "%llu", (unsigned long long)global::GameID);
            row("GameID", global::GameID ? gb : "--");
            char pb[8]; snprintf(pb, sizeof(pb), "%d", playerCount);
            row("Players", pb);
        }

        // ---- CAMERA ----
        section("Camera");
        row_addr  ("Address",      global::camera.Address);
        row_offset("CameraSubject",offset::camera::CameraSubject);
        row_offset("FieldOfView",  offset::camera::FieldOfView);

        // ---- PLAYER ----
        section("Player");
        row_addr  ("LocalPlayer",  global::LocalPlayer.player.Address);
        row_addr  ("Character",    global::LocalPlayer.character.Address);
        row_addr  ("HRootPart",    global::LocalPlayer.HumanoidRootPart.Address);
        row_offset("ModelInstance",offset::player::ModelInstance);
        row_offset("DisplayName",  offset::player::DisplayName);

        // ---- SCENE ----
        section("Scene");
        row_addr  ("Model",        global::model.Address);
        row_addr  ("Workspace",    global::workspace.Address);
        row_addr  ("Render",       global::render.Address);
        row_addr  ("View",         global::view.Address);
        row_addr  ("Light",        global::light.Address);
        row_offset("Brightness",   offset::light::Brightness);
        row_offset("FogEnd",       offset::light::FogEnd);

        // ---- FRAMEWORK ----
        section("Framework");
        row_offset("FakeModel",    offset::fakemodel::Pointer);
        row_offset("Render ptr",   offset::render::Pointer);
        row_offset("Task ptr",     offset::task::Pointer);

        // ---- LOGS ----
        section("Logs");
        {
            std::lock_guard<std::mutex> lock(logMutex);
            if (logLines.empty()) {
                box_left();
                col(C_DARK); fputs("  no events yet", stdout);
                pad_to(kW - 4, 16); col(C_RESET); box_right();
            } else {
                for (const auto& line : logLines) {
                    box_left();
                    // line already has ANSI color codes
                    printf(" %s", line.c_str());
                    // reset and close — column alignment not guaranteed with colored strings
                    col(C_RESET); fputs("\n", stdout);
                }
            }
        }

        box_bot();

        // footer hints
        printf("\n");
        col(C_DARK); printf("  INS "); col(C_GRAY); printf("toggle overlay");
        col(C_DARK); printf("   END "); col(C_GRAY); printf("exit\n");
        col(C_RESET);
    }

    // ========================================================================
    // Logging
    // ========================================================================
    inline void log(const std::string& msg) {
        std::lock_guard<std::mutex> lock(logMutex);
        logLines.push_back(msg);
        if (logLines.size() > 10) logLines.erase(logLines.begin());
    }

    inline void log_formatted(const char* prefix, const char* color, const char* fmt, va_list args) {
        char buf[1024]; vsnprintf(buf, sizeof(buf), fmt, args);
        SYSTEMTIME lt; GetLocalTime(&lt);
        char ts[16]; snprintf(ts, sizeof(ts), "%02d:%02d:%02d", lt.wHour, lt.wMinute, lt.wSecond);
        std::string out = std::string(C_DARK) + ts + C_RESET " " + color + prefix + C_RESET " " + C_MILK_DIM + buf + C_RESET;
        log(out);
        if (initialized) refresh();
    }

    inline void info(const char* fmt, ...) {
        va_list a; va_start(a, fmt);
        log_formatted("info", C_BLUE, fmt, a); va_end(a);
    }
    inline void success(const char* fmt, ...) {
        va_list a; va_start(a, fmt);
        log_formatted("ok  ", C_GREEN, fmt, a); va_end(a);
    }
    inline void warn(const char* fmt, ...) {
        va_list a; va_start(a, fmt);
        log_formatted("warn", C_YELLOW, fmt, a); va_end(a);
    }
    inline void error(const char* fmt, ...) {
        va_list a; va_start(a, fmt);
        log_formatted("fail", C_RED, fmt, a); va_end(a);
    }

    // ========================================================================
    // Refresh
    // ========================================================================
    inline void refresh() {
        COORD top = { 0, 0 };
        SetConsoleCursorPosition(hConsole, top);
        render(false);
    }

    // ========================================================================
    // Init / Start
    // ========================================================================
    inline void init() {
        if (initialized) return;
        DWORD mode = 0;
        if (GetConsoleMode(hConsole, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hConsole, mode);
        }
        hide_cursor();
        // UTF-8 output
        SetConsoleOutputCP(65001);
        // Tall buffer so scroll doesn't eat old render
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        csbi.dwSize.Y = 800;
        SetConsoleScreenBufferSize(hConsole, csbi.dwSize);
        system("color 00");
        SetConsoleTitleA("miserable");
        initialized = true;
    }

    inline void start() { init(); render(); }

    #undef C_RESET
    #undef C_RED
    #undef C_RED_DIM
    #undef C_MILK
    #undef C_MILK_DIM
    #undef C_GRAY
    #undef C_DARK
    #undef C_GREEN
    #undef C_YELLOW
    #undef C_BLUE
    #undef C_BG

} // namespace console
