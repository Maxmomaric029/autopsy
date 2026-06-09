#include <iostream>
#include <thread>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>

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
    // Load offsets with try-catch: remote fetch -> fallback to local file
    try {
        OffsetsManager::instance().load();
    }
    catch (...) {
    }

    // Apply loaded offsets to the runtime offset:: namespace
    offset::init();

    // Start console UI thread
    console::start();

    static constexpr const char* BINARY_NAME = { "RobloxPlayerBeta.exe" };
    const bool alreadyRunning = process(BINARY_NAME);

    if (!alreadyRunning)
    {
        while (!process(BINARY_NAME))
        {
            Sleep(500);
        }
    }

    if (!alreadyRunning)
        Sleep(5000);
    std::thread(watch, BINARY_NAME).detach();

    drive->process(BINARY_NAME);
    drive->attach(BINARY_NAME);
    drive->module(BINARY_NAME);

    auto fakemodel = drive->read<std::uint64_t>(drive->modulebase() + offset::fakemodel::Pointer);
    global::model.Address = drive->read<std::uint64_t>(fakemodel + offset::fakemodel::RealDataModel);
    global::render.Address = drive->read<std::uint64_t>(drive->modulebase() + offset::render::Pointer);
    global::actor.Address = global::model.childclass("Players").Address;
    global::workspace.Address = global::model.childclass("Workspace").Address;
    global::camera.Address = global::workspace.childclass("Camera").Address;
    auto Lightin = global::model.childclass("Lighting");
    global::light = sdk::light(Lightin.Address);

    std::thread(cache::run).detach();
    std::thread(world::run).detach();
    std::thread(aim::run).detach();
    std::thread(silent::run).detach();
    std::thread(misc::run).detach();
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

    // Console auto-refresh timer (every 2 seconds to show updated offsets)
    double lastConsoleRefresh = 0.0;

    for (;;)
    {
        screen->begin();
        screen->visual();
        screen->menu();
        screen->end();

        // Refresh console offsets display every 2 seconds
        double now = ImGui::GetTime();
        if (now - lastConsoleRefresh >= 2.0) {
            lastConsoleRefresh = now;
            console::refresh();
        }
    }

    return 0;
}
