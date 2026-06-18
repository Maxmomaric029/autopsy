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
// Vectored Exception Handler — log to DESKTOP so file is always writable.
// Uses only Win32 API (no CRT) for maximum safety in corrupted state.
// Also shows a MessageBox as visible confirmation.
// ========================================================================
static LONG WINAPI VectoredHandler(PEXCEPTION_POINTERS ep)
{
    if (!ep || !ep->ExceptionRecord)
        return EXCEPTION_CONTINUE_SEARCH;

    // Get desktop path (always writable)
    char path[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, path) == S_OK)
    {
        // Append "\crash_log.txt" via memcpy (compiler intrinsic, safe)
        size_t baseLen = strlen(path);
        if (baseLen + 15 <= MAX_PATH)
        {
            memcpy(path + baseLen, "\\crash_log.txt", 15); // includes null

            HANDLE hFile = CreateFileA(path, GENERIC_WRITE, 0, NULL,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                DWORD n;
                auto W = [&](const void* d, DWORD len) { WriteFile(hFile, d, len, &n, NULL); };

                // Manual hex formatting — no CRT printf
                auto hex32 = [](char* out, DWORD v) {
                    for (int i = 7; i >= 0; i--) {
                        int nib = (v >> (i * 4)) & 0xF;
                        out[7 - i] = nib < 10 ? '0' + (char)nib : 'A' + (char)(nib - 10);
                    }
                    out[8] = 0;
                };
                auto hex64 = [](char* out, ULONG64 v) {
                    for (int i = 15; i >= 0; i--) {
                        int nib = (int)((v >> (i * 4)) & 0xF);
                        out[15 - i] = nib < 10 ? '0' + (char)nib : 'A' + (char)(nib - 10);
                    }
                    out[16] = 0;
                };

                DWORD code = ep->ExceptionRecord->ExceptionCode;
                ULONG64 addr = (ULONG64)(uintptr_t)ep->ExceptionRecord->ExceptionAddress;
                DWORD flags = ep->ExceptionRecord->ExceptionFlags;

                W("CRASH\r\nCode: 0x", 12);
                char tmp[17];
                hex32(tmp, code); W(tmp, 8);

                W("\r\nAddr: 0x", 10);
                hex64(tmp, addr); W(tmp, 16);

                W("\r\nFlag: 0x", 10);
                hex32(tmp, flags); W(tmp, 8);

                if (code == EXCEPTION_ACCESS_VIOLATION && ep->ExceptionRecord->NumberParameters >= 2)
                {
                    ULONG64 target = ep->ExceptionRecord->ExceptionInformation[1];
                    W("\r\nAccs: ", 7);
                    W(ep->ExceptionRecord->ExceptionInformation[0] ? "WRITE" : "READ", 4);
                    W("\r\nTrgt: 0x", 10);
                    hex64(tmp, target); W(tmp, 16);
                }

                W("\r\n", 2);
                CloseHandle(hFile);
            }
        }
    }

    // MessageBox as visible confirmation
    char msg[128];
    wsprintfA(msg, "Crash! Code: 0x%08lX\nAddr: %p",
        ep->ExceptionRecord->ExceptionCode,
        ep->ExceptionRecord->ExceptionAddress);
    MessageBoxA(NULL, msg, "miserable.exe - Crash Handler", MB_OK | MB_ICONERROR);

    OutputDebugStringA("[VEH] Caught crash, wrote to Desktop\\crash_log.txt\n");
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
    PVOID vehHandle = AddVectoredExceptionHandler(1, VectoredHandler);
    if (vehHandle) {
        OutputDebugStringA("[VEH] VEH registered successfully\n");
    } else {
        OutputDebugStringA("[VEH] VEH REGISTRATION FAILED!\n");
    }

    renderLoop();
    return 0;
}
