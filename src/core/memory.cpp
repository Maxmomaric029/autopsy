#include "memory.h"
#include "../sdk/offsets/offset.h"

std::uint32_t driver::process(const std::string& Process_Name)
{
    std::uint32_t Local_Process = 0;
    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Snapshot == INVALID_HANDLE_VALUE)
    {
        return Local_Process;
    }

    PROCESSENTRY32 Process_Entry{};
    Process_Entry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(Snapshot, &Process_Entry))
    {
        do
        {
            if (!_stricmp(Process_Name.c_str(), Process_Entry.szExeFile))
            {
                Local_Process = Process_Entry.th32ProcessID;
                pid = Local_Process;
                break;
            }
        } while (Process32Next(Snapshot, &Process_Entry));
    }

    CloseHandle(Snapshot);
    return Local_Process;
}

std::uint64_t driver::module(const std::string& Module_Name)
{
    std::uint64_t Module_Address = 0;

    if (!handle)
    {
        return Module_Address;
    }

    DWORD pid = GetProcessId(handle);
    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);

    if (Snapshot == INVALID_HANDLE_VALUE)
    {
        return Module_Address;
    }

    MODULEENTRY32 Module_Entry{};
    Module_Entry.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(Snapshot, &Module_Entry))
    {
        do
        {
            if (!_stricmp(Module_Name.c_str(), Module_Entry.szModule))
            {
                Module_Address = reinterpret_cast<uint64_t>(Module_Entry.modBaseAddr);
                base = Module_Address;
                break;
            }
        } while (Module32Next(Snapshot, &Module_Entry));
    }

    CloseHandle(Snapshot);
    return Module_Address;
}

bool driver::attach(const std::string& Process_Name)
{
    HANDLE Process = OpenProcess(PROCESS_ALL_ACCESS, false, process(Process_Name));

    if (Process == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    handle = Process;

    return true;
}

std::string driver::readstring(std::uint64_t Address)
{
    std::uint32_t String_Length = read<std::uint32_t>(Address + offset::misc::StringLength);
    std::uint64_t String_Address = (String_Length >= 16) ? read<std::uint64_t>(Address) : Address;

    if (String_Length == 0 || String_Length > 255)
    {
        return "Unknown";
    }

    std::vector<char> Buffer(String_Length + 1, 0);
    readvm(handle, reinterpret_cast<void*>(String_Address), Buffer.data(), Buffer.size(), nullptr);

    return std::string(Buffer.data(), String_Length);
}

void driver::writestring(std::uint64_t Address, const std::string& Value)
{
    auto Str = read<rbxstring>(Address);
    std::uint64_t OldDataPointer = (Str.Length > 15) ? Str.Data.Pointer : 0;

    if (Value.length() > Str.Capacity)
    {
        while (Value.length() > Str.Capacity)
            Str.Capacity = Str.Capacity * 2 + 1;

        // Free old heap allocation before allocating new one
        if (OldDataPointer)
            VirtualFreeEx(handle, reinterpret_cast<LPVOID>(OldDataPointer), 0, MEM_RELEASE);

        Str.Data.Pointer = reinterpret_cast<std::uint64_t>(
            VirtualAllocEx(
                handle,
                nullptr,
                Str.Capacity,
                MEM_RESERVE | MEM_COMMIT,
                PAGE_READWRITE
            )
            );
    }
    // else: no reallocation needed, keep OldDataPointer for cleanup below
    // (it will be freed in the short-branch if switching from heap→inline)

    Str.Length = Value.length();

    if (Str.Length > 15)
    {
        write<rbxstring>(Address, Str);
        writevm(
            handle,
            reinterpret_cast<void*>(Str.Data.Pointer),
            (void*)Value.data(),
            Value.length(),
            nullptr
        );
    }
    else
    {
        // Switching from long to short: free old heap allocation
        if (OldDataPointer)
            VirtualFreeEx(handle, reinterpret_cast<LPVOID>(OldDataPointer), 0, MEM_RELEASE);

        Str.Capacity = 15;
        write<rbxstring>(Address, Str);
        writevm(
            handle,
            reinterpret_cast<void*>(Address),
            (void*)Value.data(),
            Value.length(),
            nullptr
        );
    }
}

std::uint32_t driver::processid()
{
    return pid;
}

std::uint64_t driver::modulebase()
{
    return base;
}

HANDLE driver::processhandle()
{
    return handle;
}
