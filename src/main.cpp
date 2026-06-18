#include <iostream>
#include <thread>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <cstdio>
#include "config.h"

#include "core/memory.h"
#include "core/offsets_manager.h"
#include "global.h"
#include "log.h"
#include "ui/graphic.h"
#include "sdk/sdk.h"
#include "core/cache.h"
#include "features/misc.h"
#include "features/world.h"
#include "features/ball.h"
#include "features/aimbot.h"
#include "features/silent.h"
#include <ShlObj.h>
#pragma comment(lib, "Shell32.lib")

// Forward declaration from graphic.cpp
extern bool g_frameHadContent;

// ========================================================================
// Vectored Exception Handler — catches crashes on ALL threads.
// Uses Win32 API directly (CreateFile/WriteFile) to avoid CRT heap
// dependency in case heap is corrupted.
// ========================================================================
static LONG WINAPI VectoredHandler(PEXCEPTION_POINTERS ep)
{
    if (!ep || !ep->ExceptionRecord)
        return EXCEPTION_CONTINUE_SEARCH;

    // Only log actionable hardware exceptions
    const DWORD code = ep->ExceptionRecord->ExceptionCode;
    if (code != EXCEPTION_ACCESS_VIOLATION &&
        code != EXCEPTION_ILLEGAL_INSTRUCTION &&
        code != EXCEPTION_STACK_OVERFLOW &&
        code != EXCEPTION_INT_DIVIDE_BY_ZERO &&
        code != EXCEPTION_BREAKPOINT)
        return EXCEPTION_CONTINUE_SEARCH;

    // Build crash log path in the exe directory
    char exeDir[MAX_PATH];
    DWORD exeLen = GetModuleFileNameA(NULL, exeDir, MAX_PATH);
    if (exeLen == 0 || exeLen >= MAX_PATH)
        return EXCEPTION_CONTINUE_SEARCH;

    // Find last backslash and replace with "crash_log.txt"
    char* sep = strrchr(exeDir, '\\');
    if (!sep) return EXCEPTION_CONTINUE_SEARCH;
    strcpy_s(sep + 1, MAX_PATH - (sep - exeDir + 1), "crash_log.txt");

    HANDLE hFile = CreateFileA(exeDir, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return EXCEPTION_CONTINUE_SEARCH;

    char buf[1024];
    int off = 0;
    off += sprintf_s(buf + off, sizeof(buf) - off,
        "[CRASH] Vectored Exception Handler\r\n"
        "Exception code: 0x%08lX\r\n"
        "Exception address: 0x%p\r\n"
        "Exception flags: 0x%lX\r\n",
        code, ep->ExceptionRecord->ExceptionAddress,
        ep->ExceptionRecord->ExceptionFlags);

    if (code == EXCEPTION_ACCESS_VIOLATION && ep->ExceptionRecord->NumberParameters >= 2)
    {
        off += sprintf_s(buf + off, sizeof(buf) - off,
            "Access type: %s\r\n"
            "Target address: 0x%p\r\n",
            ep->ExceptionRecord->ExceptionInformation[0] ? "WRITE" : "READ",
            (void*)ep->ExceptionRecord->ExceptionInformation[1]);
    }

    off += sprintf_s(buf + off, sizeof(buf) - off,
        "\r\nDisassemble the exception address to find the crashing source line.\r\n");

    DWORD written = 0;
    WriteFile(hFile, buf, off, &written, NULL);
    CloseHandle(hFile);

    OutputDebugStringA("[VEH] Crash logged to crash_log.txt\n");

    return EXCEPTION_CONTINUE_SEARCH;
}

// ========================================================================
// Render loop with step markers (no SEH — VEH handles all threads)
// ========================================================================
static void renderLoop()
{
    double lastConsoleRefresh = 0.0;

    for (;;)
    {
        OutputDebugStringA("[RENDER] begin\n");
        screen->begin();

        OutputDebugStringA("[RENDER] visual\n");
        screen->visual();

        OutputDebugStringA("[RENDER] menu\n");
        screen->menu();

        OutputDebugStringA("[RENDER] end\n");
        screen->end();

        double now = ImGui::GetTime();
        if (now - lastConsoleRefresh >= 2.0) {
            lastConsoleRefresh = now;
            console::refresh();
        }
    }
}

namespace
{
    bool process(const char* processName)
    {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE)
            return false;

        PROCESSENTRY32 entry{};
        entry.dwSize = sizeof(entry);

        bool found = false;
        if (Process32First(snapshot, &entry))
        {
            do
            {
                if (_stricmp(entry.szExeFile, processName) == 0)
                {
                    found = true;
                    break;
                }
            } while (Process32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);
        return found;
    }

    void clearstate()
    {
        {
            std::lock_guard<std::mutex> lock(cache::Mutex);
            global::Player_Cache.clear();
        }

        global::GameID = 0;
        global::LocalPlayer = {};
        global::model.Address = 0;
        global::render.Address = 0;
        global::actor.Address = 0;
        global::workspace.Address = 0;
        global::camera.Address = 0;
        global::light = {};
    }

    bool relaunch()
    {
        char path[MAX_PATH]{};
        if (!GetModuleFileNameA(nullptr, path, MAX_PATH))
            return false;

        STARTUPINFOA si{};
        PROCESS_INFORMATION pi{};
        si.cb = sizeof(si);

        std::string cmd = "\"";
        cmd += path;
        cmd += "\"";

        const bool created = CreateProcessA(
            nullptr,
            cmd.data(),
            nullptr,
            nullptr,
            FALSE,
            CREATE_NO_WINDOW,
            nullptr,
            nullptr,
            &si,
            &pi) != FALSE;

        if (created)
        {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }

        return created;
    }

    void watch(const char* processName)
    {
        for (;;)
        {
            Sleep(1000);
            if (!process(processName))
            {
                clearstate();

                while (!process(processName))
                    Sleep(1000);

                relaunch();
                ExitProcess(0);
            }
        }
    }
}

std::int32_t main(std::int32_t argc, char** argv[])
{
    // Start console UI thread immediately so logs are printed to the screen
    console::start();

    // Load auto-saved JSON configuration
    config::load_json("autoload");

    // Load offsets with try-catch: remote fetch -> fallback to local file
    {
        // Write debug to file so it's never lost
        FILE* dbg = fopen("offsets_debug.txt", "w");
        if (dbg) {
            char cwdBuf[MAX_PATH];
            GetCurrentDirectoryA(sizeof(cwdBuf), cwdBuf);
            fprintf(dbg, "[offsets] CWD: %s\n", cwdBuf);
        }

        bool ok = false;
        try {
            ok = OffsetsManager::instance().load();
        }
        catch (...) {
        }

        if (dbg) {
            fprintf(dbg, "[offsets] load(): %s\n", ok ? "SUCCESS" : "FAILED");
            fprintf(dbg, "[offsets] is_loaded: %s\n",
                OffsetsManager::instance().is_loaded() ? "YES" : "NO");
            if (ok) {
                fprintf(dbg, "[offsets] total: %d\n", OffsetsManager::instance().total_offsets());
            }
            fprintf(dbg, "[offsets] _before_ init() - defaults:\n");
            fprintf(dbg, "[offsets]   fakemodel::Pointer = 0x%llX\n", (unsigned long long)offset::fakemodel::Pointer);
            fprintf(dbg, "[offsets]   render::Pointer = 0x%llX\n", (unsigned long long)offset::render::Pointer);
            fprintf(dbg, "[offsets]   task::Pointer = 0x%llX\n", (unsigned long long)offset::task::Pointer);
        }

        // Apply loaded offsets to the runtime offset:: namespace
        offset::init();

        if (dbg) {
            fprintf(dbg, "[offsets] _after_ init() - values:\n");
            fprintf(dbg, "[offsets]   fakemodel::Pointer = 0x%llX\n", (unsigned long long)offset::fakemodel::Pointer);
            fprintf(dbg, "[offsets]   render::Pointer = 0x%llX\n", (unsigned long long)offset::render::Pointer);
            fprintf(dbg, "[offsets]   task::Pointer = 0x%llX\n", (unsigned long long)offset::task::Pointer);
            fclose(dbg);
        }
    }

    static constexpr const char* BINARY_NAME = { "RobloxPlayerBeta.exe" };
    const bool alreadyRunning = process(BINARY_NAME);

    if (!alreadyRunning)
    {
        console::warn("Esperando a que se inicie %s...", BINARY_NAME);
        while (!process(BINARY_NAME))
        {
            Sleep(500);
        }
    }

    if (!alreadyRunning)
        Sleep(5000);
    
    console::info("Iniciando hilo de watch para %s...", BINARY_NAME);
    std::thread(watch, BINARY_NAME).detach();

    console::info("Acoplando driver...");
    drive->process(BINARY_NAME);
    drive->attach(BINARY_NAME);
    drive->module(BINARY_NAME);
    console::success("Driver acoplado con exito. ModuloBase: 0x%llX", (unsigned long long)drive->modulebase());

    auto fakemodel = drive->read<std::uint64_t>(drive->modulebase() + offset::fakemodel::Pointer);
    global::model.Address = drive->read<std::uint64_t>(fakemodel + offset::fakemodel::RealDataModel);
    global::render.Address = drive->read<std::uint64_t>(drive->modulebase() + offset::render::Pointer);
    global::actor.Address = global::model.childclass("Players").Address;
    global::workspace.Address = global::model.childclass("Workspace").Address;
    global::camera.Address = global::workspace.childclass("Camera").Address;
    auto Lightin = global::model.childclass("Lighting");
    global::light = sdk::light(Lightin.Address);
    console::success("DataModel obtenido: 0x%llX", (unsigned long long)global::model.Address);

    // ---- Background update threads ----
    // cache, aim, silent, ball each have their own timing and data requirements.
    // world + misc are merged into a single combined thread to reduce context switching.
    //
    // NOTE: global::Player_Cache is protected by cache::Mutex (see cache.cpp).
    // Other shared globals (camera, LocalPlayer, etc.) are read concurrently
    // across threads — consider std::atomic or std::shared_mutex if crashes occur.
    std::thread(cache::run).detach();
    std::thread([]() {
        world::run();  // spawns skybox, atmosphere, fog, brightness, exposure, fov
        misc::run();   // spawns fly, walkspeed, hitbox
    }).detach();
    std::thread(aim::run).detach();
    std::thread(silent::run).detach();
    std::thread(ball::run).detach();

    auto workspacetoworld = drive->read<uintptr_t>(global::workspace.Address + offset::workspace::world);
    drive->write<float>(workspacetoworld + offset::world::GravityOverride, 200 * 4.f);

    if (!screen->window())
    {
        return 1;
    }

    if (!screen->device())
    {
        return 1;
    }

    if (!screen->imgui())
    {
        return 1;
    }
    // Register VEH early — catches crashes on ALL threads (including cache/aim/etc.)
    AddVectoredExceptionHandler(1, VectoredHandler);
    OutputDebugStringA("[VEH] Registered VectoredExceptionHandler\n");

    renderLoop();
    return 0;
}
