#include <iostream>
#include <thread>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <cstdio>
#include <chrono>
#include <random>
#include "config.h"

#include "core/memory.h"
#include "core/apc_stealth.h"
#include "loader/loader.h"
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

    DWORD code = ep->ExceptionRecord->ExceptionCode;

    // Ignorar excepciones de debug output — no son crashes reales
    if (code == 0x40010006 || code == 0x4001000A)
        return EXCEPTION_CONTINUE_SEARCH;

    // Solo manejar crashes reales (bit alto en 0 = informacional de sistema)
    if (code < 0x80000000)
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

    global::render.Address = drive->read<std::uint64_t>(drive->modulebase() + offset::render::Pointer);

    // Resolver DataModel con reintentos — 3 rutas (Directa, VisualEngine, RenderJob)
    global::model.Address = sdk::resolve_datamodel();
    int dmRetries = 0;
    while (!global::model.Address && dmRetries < 20) {
        console::warn("Reintentando DataModel (%d/20)...", dmRetries + 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        global::model.Address = sdk::resolve_datamodel();
        dmRetries++;
    }

    if (global::model.Address) {
        global::actor.Address = global::model.childclass("Players").Address;
        global::workspace.Address = global::model.childclass("Workspace").Address;
        global::camera.Address = global::workspace.childclass("Camera").Address;
        auto Lightin = global::model.childclass("Lighting");
        global::light = sdk::light(Lightin.Address);
        console::success("DataModel obtenido: 0x%llX", (unsigned long long)global::model.Address);
    } else {
        console::error("NO SE PUDO RESOLVER EL DATAMODEL (ver datamodel_debug.txt)");
    }

    // ================================================================
    // BYOVD Loader — Cargar amdgpio3.sys para operaciones Ring-0
    // Después de cargar StealthDrv, descargamos amdgpio3 para
    // minimizar superficie de detección.
    // ================================================================
    {
        auto& byovd = ByovdLoader::Instance();
        
        // amdgpio3.sys debe estar en el mismo directorio que el cheat
        wchar_t amdPath[MAX_PATH];
        GetModuleFileNameW(NULL, amdPath, MAX_PATH);
        wchar_t* lastSlash = wcsrchr(amdPath, L'\\');
        if (lastSlash) *lastSlash = 0;
        wcscat_s(amdPath, L"\\amdgpio3.sys");
        
        if (GetFileAttributesW(amdPath) != INVALID_FILE_ATTRIBUTES) {
            console::info("[BYOVD] amdgpio3.sys encontrado, cargando...");
            
            if (byovd.LoadDriver(amdPath)) {
                // Buscar StealthDrv.sys en el mismo directorio
                wchar_t drvPath[MAX_PATH];
                wcscpy_s(drvPath, amdPath);
                lastSlash = wcsrchr(drvPath, L'\\');
                if (lastSlash) {
                    *(lastSlash + 1) = 0;
                    wcscat_s(drvPath, L"StealthDrv.sys");
                }
                
                if (GetFileAttributesW(drvPath) != INVALID_FILE_ATTRIBUTES) {
                    console::info("[BYOVD] StealthDrv.sys encontrado, intentando cargar...");
                    byovd.LoadTargetDriver(drvPath);
                } else {
                    console::info("[BYOVD] StealthDrv.sys no encontrado — necesita compilarse");
                    console::info("[BYOVD]   Compilar con: msbuild driver/driver.vcxproj /p:Config=Release /p:Platform=x64");
                }
                
                // Descargar amdgpio3.sys — ya no lo necesitamos
                // (minimiza superficie de detección)
                console::info("[BYOVD] Descargando amdgpio3.sys...");
                byovd.Unload();
            }
        } else {
            console::info("[BYOVD] amdgpio3.sys no encontrado, saltando BYOVD...");
            console::info("[BYOVD]   El cheat usará thread hijacking + syscalls directos");
        }
    }

    // ================================================================
    // Stealth layer initialization
    // After memory driver is attached, initialize the stealth layer.
    // Priority chain: Kernel Driver (Ring-0) → Thread Hijack → Syscall
    //
    // Once initialized, ALL driver::read/write calls go through the
    // stealth layer transparently via the g_useApcStealth toggle.
    // ================================================================
    {
        auto& apc = ApcStealth::Instance();
        if (apc.Initialize(drive->processhandle(), drive->modulebase())) {
            g_useApcStealth.store(true, std::memory_order_release);
            console::success("[STEALTH] Modo: %s — operaciones de memoria invisibles",
                apc.ModeName());

            if (apc.GetMode() == StealthMode::Driver) {
                console::info("[STEALTH] Usando kernel driver Ring-0 (MmCopyVirtualMemory)");
            } else if (apc.GetMode() == StealthMode::ThreadHijack) {
                console::info("[STEALTH] Shellcode inyectado en proceso target, threads hijackeados");
            } else {
                console::info("[STEALTH] Usando syscalls directos via ASM (modo legacy)");
            }
        } else {
            console::warn("[STEALTH] No se pudo inicializar — usando syscalls directos (menos seguro)");
            g_useApcStealth.store(false, std::memory_order_release);
        }
    }

    // ---- Background threads ----
    // Threads still exist for feature responsiveness, but ALL memory
    // operations (driver::read/write) now go through APC stealth
    // transparently via the g_useApcStealth toggle in memory.h.
    // This means reads/writes execute INSIDE RobloxPlayerBeta.exe,
    // not from our external process.
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
