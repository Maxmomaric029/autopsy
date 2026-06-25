#pragma once
#include <windows.h>
#include <cstdio>
#include <string>
#include <vector>
#include <mutex>
#include "global.h"

// ============================================================================
// MISERABLE Console — boot sequence display, static (no auto-refresh)
// Shows ASCII art, logs durante init, offsets al final, luego inicia UI sola.
// ============================================================================

namespace console {

    inline HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    inline bool initialized = false;

    // Log buffer — solo se imprime, no se refresca en loop
    inline std::vector<std::string> logLines;
    inline std::mutex logMutex;

    // Señal para que main.cpp sepa cuando imprimir offsets y arrancar UI
    inline bool boot_complete = false;

    // Variables de estado usadas por cache.cpp
    inline int playerCount = 0;
    inline bool connected  = false;
    inline uintptr_t cameraAddress = 0;

    // Needed by renderLoop in main.cpp — no-op, consola es estática
    inline void refresh() {}

    // ========================================================================
    // Color palette (ANSI 24-bit)
    // ========================================================================
    #define C_RESET    "\033[0m"
    #define C_RED      "\033[38;2;224;48;64m"
    #define C_RED_DIM  "\033[38;2;140;28;38m"
    #define C_MILK     "\033[38;2;245;240;235m"
    #define C_MILK_DIM "\033[38;2;160;148;140m"
    #define C_GRAY     "\033[38;2;90;72;72m"
    #define C_DARK     "\033[38;2;50;18;18m"
    #define C_GREEN    "\033[38;2;80;210;120m"
    #define C_YELLOW   "\033[38;2;220;180;60m"
    #define C_BLUE     "\033[38;2;100;160;240m"

    inline void cls() { printf("\033[2J\033[H"); }

    inline void hide_cursor() {
        CONSOLE_CURSOR_INFO ci;
        GetConsoleCursorInfo(hConsole, &ci);
        ci.bVisible = false;
        SetConsoleCursorInfo(hConsole, &ci);
    }

    // ========================================================================
    // ASCII art — ancho ~68 chars, centrado en ventana 80col
    // ========================================================================
    inline void draw_ascii() {
        // margen izquierdo de 6 espacios para centrar en 80
        const char* M = "      ";
        printf("%s" C_RED, M);
        printf(" __ __ _ ____  ___ ____  ___  ____  __   ____\n");
        printf("%s" C_RED, M);
        printf("|  \\/  | |_ _|/ __| __| | _ \\| __ )|  \\ |  __|\n");
        printf("%s" C_RED, M);
        printf("| |\/| | | \\__ \\___ \\| __|  |   /|  _/  > _|\n");
        printf("%s" C_RED, M);
        printf("|_|  |_|___|___/|____|___|  |_|\\_\\___|_|\\_\\____|\n");
        printf(C_RESET);
    }

    // ========================================================================
    // Separador simple
    // ========================================================================
    inline void sep() {
        printf("  " C_DARK);
        for (int i = 0; i < 64; i++) fputs("\xe2\x94\x80", stdout);
        printf(C_RESET "\n");
    }

    // ========================================================================
    // draw_boot — pantalla inicial: ASCII + línea de estado
    // Se llama UNA vez al arrancar. Los logs se van agregando abajo.
    // ========================================================================
    inline void draw_boot() {
        cls();
        printf("\n");
        draw_ascii();
        printf("\n");
        sep();
        printf("  " C_DARK "external  //  roblox" C_RESET "\n");
        sep();
        printf("\n");
    }

    // ========================================================================
    // print_log_line — imprime la línea directamente, no refresca todo
    // ========================================================================
    inline void print_log_line(const char* color, const char* prefix, const char* msg) {
        SYSTEMTIME lt; GetLocalTime(&lt);
        printf("  " C_DARK "%02d:%02d:%02d" C_RESET "  %s%-4s" C_RESET "  " C_MILK_DIM "%s" C_RESET "\n",
            lt.wHour, lt.wMinute, lt.wSecond,
            color, prefix,
            msg);
    }

    // ========================================================================
    // print_offsets — se llama SOLO al terminar el boot, muestra tabla limpia
    // ========================================================================
    inline void print_offsets() {
        printf("\n");
        sep();
        printf("  " C_RED_DIM "offsets" C_RESET "\n");
        sep();

        auto row = [](const char* label, uintptr_t val) {
            if (val)
                printf("  " C_GRAY "%-24s" C_RESET C_RED_DIM "0x%04llX" C_RESET "\n",
                    label, (unsigned long long)val);
            else
                printf("  " C_GRAY "%-24s" C_RESET C_DARK "--" C_RESET "\n", label);
        };

        row("fakemodel::Pointer",   offset::fakemodel::Pointer);
        row("render::Pointer",      offset::render::Pointer);
        row("task::Pointer",        offset::task::Pointer);
        row("camera::CameraSubject",offset::camera::CameraSubject);
        row("camera::FieldOfView",  offset::camera::FieldOfView);
        row("player::ModelInstance",offset::player::ModelInstance);
        row("player::DisplayName",  offset::player::DisplayName);
        row("light::Brightness",    offset::light::Brightness);
        row("light::FogEnd",        offset::light::FogEnd);

        sep();
        printf("\n");
    }

    // ========================================================================
    // print_ready — mensaje final antes de que la UI abra sola
    // ========================================================================
    inline void print_ready() {
        printf("  " C_GREEN "ready" C_RESET "  " C_MILK_DIM "overlay activo — INS para abrir menú" C_RESET "\n\n");
    }

    // ========================================================================
    // Logging público — imprime directamente, sin re-render de pantalla
    // ========================================================================
    inline void log_raw(const char* color, const char* prefix, const char* fmt, va_list args) {
        char buf[1024];
        vsnprintf(buf, sizeof(buf), fmt, args);
        print_log_line(color, prefix, buf);
        // Guardar también en buffer por si acaso
        std::lock_guard<std::mutex> lock(logMutex);
        logLines.push_back(std::string(prefix) + " " + buf);
        if (logLines.size() > 64) logLines.erase(logLines.begin());
    }

    inline void info(const char* fmt, ...) {
        va_list a; va_start(a, fmt);
        log_raw(C_BLUE, "info", fmt, a); va_end(a);
    }
    inline void success(const char* fmt, ...) {
        va_list a; va_start(a, fmt);
        log_raw(C_GREEN, "ok", fmt, a); va_end(a);
    }
    inline void warn(const char* fmt, ...) {
        va_list a; va_start(a, fmt);
        log_raw(C_YELLOW, "warn", fmt, a); va_end(a);
    }
    inline void error(const char* fmt, ...) {
        va_list a; va_start(a, fmt);
        log_raw(C_RED, "fail", fmt, a); va_end(a);
    }

    // ========================================================================
    // Init / Start — solo setup de consola, dibuja la pantalla de boot
    // ========================================================================
    inline void init() {
        if (initialized) return;
        DWORD mode = 0;
        if (GetConsoleMode(hConsole, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hConsole, mode);
        }
        hide_cursor();
        SetConsoleOutputCP(65001);
        // Buffer generoso para que no se pierda nada durante el boot
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        csbi.dwSize.Y = 600;
        SetConsoleScreenBufferSize(hConsole, csbi.dwSize);
        system("color 00");
        SetConsoleTitleA("miserable");
        initialized = true;
    }

    // start() — llamado desde main() al arrancar
    inline void start() {
        init();
        draw_boot();
    }

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

} // namespace console
