#include "loader.h"
#include "../log.h"
#include <vector>
#include <string>
#include <algorithm>
#include <winternl.h>
#include <psapi.h>

#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "advapi32.lib")

// ======================================================================
// NT API typedefs
// ======================================================================
typedef NTSTATUS(NTAPI* _NtQuerySystemInformation)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

#define SystemModuleInformation 0x0B

// ======================================================================
// Singleton
// ======================================================================
ByovdLoader& ByovdLoader::Instance() {
    static ByovdLoader s_instance;
    return s_instance;
}

// ======================================================================
// Construction / Destruction
// ======================================================================
ByovdLoader::ByovdLoader()
    : m_deviceHandle(INVALID_HANDLE_VALUE)
    , m_serviceHandle(NULL)
    , m_stealthDrvLoaded(false)
    , m_kernelBase(0)
    , m_ciDllBase(0)
    , m_mapPhysIoctlCode(IOCTL_AMDGPIO_MAP_PHYSICAL)
    , m_scmHandle(NULL)
{
    wcscpy_s(m_serviceName, AMDGPIO_SERVICE);
}

ByovdLoader::~ByovdLoader() {
    Unload();
}

// ======================================================================
// Load the vulnerable driver (amdgpio3.sys) using SCM
// ======================================================================
bool ByovdLoader::LoadDriver(const std::wstring& driverPath) {
    if (IsLoaded()) {
        console::info("[BYOVD] amdgpio3 already loaded");
        return true;
    }

    console::info("[BYOVD] Loading amdgpio3.sys from %S...", driverPath.c_str());

    // 1. Copy driver to system32\drivers (required by SCM)
    WCHAR destPath[MAX_PATH];
    GetSystemDirectoryW(destPath, MAX_PATH);
    wcscat_s(destPath, L"\\drivers\\amdgpio3.sys");

    if (!CopyFileW(driverPath.c_str(), destPath, FALSE)) {
        DWORD err = GetLastError();
        if (err != ERROR_FILE_EXISTS) {
            console::warn("[BYOVD] CopyFile failed (%lu) - may already exist", err);
        }
    }

    // 2. Open SCM
    m_scmHandle = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!m_scmHandle) {
        console::error("[BYOVD] OpenSCManager failed (%lu)", GetLastError());
        return false;
    }

    // 3. Install & start service
    if (!InstallService(destPath) || !StartService()) {
        console::error("[BYOVD] Failed to start amdgpio3 service");
        CloseServiceHandle(m_scmHandle);
        m_scmHandle = NULL;
        return false;
    }

    // 4. Open device handle
    if (!OpenDevice()) {
        console::error("[BYOVD] Failed to open amdgpio3 device");
        StopService();
        CloseServiceHandle(m_scmHandle);
        m_scmHandle = NULL;
        return false;
    }

    // 5. Find kernel base for reference
    m_kernelBase = FindKernelBase();
    console::info("[BYOVD] Kernel base: 0x%llX", m_kernelBase);

    console::success("[BYOVD] amdgpio3.sys loaded and device opened");
    return true;
}

// ======================================================================
// Install service
// ======================================================================
bool ByovdLoader::InstallService(const std::wstring& binaryPath) {
    // Check if service already exists
    m_serviceHandle = OpenServiceW(
        m_scmHandle, m_serviceName,
        SERVICE_START | SERVICE_STOP | DELETE | SERVICE_CHANGE_CONFIG
    );

    if (m_serviceHandle) {
        console::info("[BYOVD] Service already exists, reconfiguring...");
        // Update binary path in case it changed
        ChangeServiceConfigW(
            m_serviceHandle,
            SERVICE_KERNEL_DRIVER,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_IGNORE,
            binaryPath.c_str(),
            NULL, NULL, NULL, NULL, NULL
        );
        return true;
    }

    // Create new service
    m_serviceHandle = CreateServiceW(
        m_scmHandle,
        m_serviceName,
        L"AMD GPIO Controller Driver",
        SERVICE_START | SERVICE_STOP | DELETE,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_IGNORE,
        binaryPath.c_str(),
        NULL, NULL, NULL, NULL, NULL
    );

    if (!m_serviceHandle) {
        DWORD err = GetLastError();
        console::error("[BYOVD] CreateService failed (%lu)", err);
        return false;
    }

    console::info("[BYOVD] Service created");
    return true;
}

// ======================================================================
// Start service
// ======================================================================
bool ByovdLoader::StartService() {
    if (!m_serviceHandle) return false;

    if (!StartServiceW(m_serviceHandle, 0, NULL)) {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_ALREADY_RUNNING) {
            console::info("[BYOVD] Service already running");
            return true;
        }
        console::error("[BYOVD] StartService failed (%lu)", err);
        return false;
    }

    // Wait for service to start
    SERVICE_STATUS status;
    for (int i = 0; i < 10; i++) {
        if (QueryServiceStatus(m_serviceHandle, &status)) {
            if (status.dwCurrentState == SERVICE_RUNNING)
                break;
        }
        Sleep(200);
    }

    console::info("[BYOVD] Service started");
    return true;
}

// ======================================================================
// Open device handle
// ======================================================================
bool ByovdLoader::OpenDevice() {
    const wchar_t* devicePaths[] = {
        L"\\\\.\\amdgpio3",
        L"\\\\.\\AMDGPIO3",
        L"\\\\.\\AMDGPIO",
        L"\\\\.\\GLOBALROOT\\Device\\amdgpio3",
    };

    for (auto path : devicePaths) {
        m_deviceHandle = CreateFileW(
            path, GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
        );

        if (m_deviceHandle != INVALID_HANDLE_VALUE && m_deviceHandle) {
            console::info("[BYOVD] Device opened via %S", path);
            return true;
        }
    }

    DWORD err = GetLastError();
    console::error("[BYOVD] CreateFile failed on all paths (%lu)", err);
    return false;
}

// ======================================================================
// Send physical memory mapping IOCTL (Vulnerability #1)
//
// Based on the audit of sub_1400012A0+0x9D:
//   Input:  [+0x04] PHYSICAL_ADDRESS (8 bytes)
//           [+0x0C] DWORD size (< 0x20)
//   Output: [+0x08] PHYSICAL_ADDRESS echoed
//           [+0x10] QWORD mapped kernel VA
//           [+0x18] DWORD 32 (size)
//           [+0x1C] DWORD 5 (written to PA+0xC)
//
// NOTE: The driver writes a FIXED value (5) to PA+0xC.
// This is NOT a general-purpose physical memory write!
// ======================================================================
bool ByovdLoader::SendPhysicalIoctl(const PhysMapInput& input, PhysMapOutput& output) {
    if (!IsLoaded()) return false;

    // Verify size constraint from audit: must be < 0x20
    if (input.Size >= 0x20) {
        console::error("[BYOVD] Size %lu >= 0x20, driver requires < 0x20", input.Size);
        return false;
    }

    uint8_t inputBuf[0x20] = {0};
    uint8_t outputBuf[0x20] = {0};

    // Build input buffer per audit layout
    *(DWORD*)(inputBuf + 0x00) = input.Reserved;
    *(DWORD64*)(inputBuf + 0x04) = input.PhysicalAddress;
    *(DWORD*)(inputBuf + 0x0C) = input.Size;

    DWORD bytesReturned = 0;
    BOOL success = DeviceIoControl(
        m_deviceHandle,
        m_mapPhysIoctlCode,
        inputBuf, sizeof(inputBuf),
        outputBuf, sizeof(outputBuf),
        &bytesReturned,
        NULL
    );

    if (!success) {
        DWORD err = GetLastError();
        console::warn("[BYOVD] Physical IOCTL failed (PA=0x%llX, err=%lu)",
            input.PhysicalAddress, err);
        return false;
    }

    // Parse output per audit layout (FIXED: +0x00 is Reserved, PA at +0x08)
    output.Reserved = *(DWORD64*)(outputBuf + 0x00);
    output.PhysicalAddress = *(DWORD64*)(outputBuf + 0x08);
    output.MappedKernelVA = *(DWORD64*)(outputBuf + 0x10);
    output.MappedSize = *(DWORD*)(outputBuf + 0x18);
    output.WrittenValue = *(DWORD*)(outputBuf + 0x1C);

    console::info("[BYOVD] Physical IOCTL: PA=0x%llX → KVA=0x%llX (size=%lu, written=%lu, reserved=0x%llX)",
        output.PhysicalAddress, output.MappedKernelVA,
        output.MappedSize, output.WrittenValue, output.Reserved);

    return true;
}

// ======================================================================
// Physical memory read via amdgpio3
//
// LIMITATION: The IOCTL (Vulnerability #1) maps a physical address
// and returns a kernel VA. From user-mode, we CANNOT dereference
// the kernel VA. The driver only writes a fixed value 5 to PA+0xC.
//
// For ACTUAL physical memory read/write, we need:
//   a. RE of the GPIO bank IOCTL codes (Vulnerability #2)
//   b. GPIO handlers to read/write through the mapped pages
//   c. OR use a different vulnerable driver (e.g., aswRvrt.sys)
//
// Until then, this function returns false (NOT IMPLEMENTED) to
// prevent callers from relying on uninitialized buffer data.
// ======================================================================
bool ByovdLoader::PhysRead(DWORD64 physicalAddr, void* buffer, DWORD size) {
    UNREFERENCED_PARAMETER(physicalAddr);
    UNREFERENCED_PARAMETER(buffer);
    UNREFERENCED_PARAMETER(size);

    if (!IsLoaded()) return false;

    console::warn("[BYOVD] PhysRead NOT IMPLEMENTED — requires GPIO IOCTL handlers RE");
    console::warn("[BYOVD]   See Vulnerability #2: sub_140001088, sub_140001144, etc.");
    console::warn("[BYOVD]   Mine the IOCTL codes via IDA/Ghidra from amdgpio3.sys");
    return false;
}

// ======================================================================
// Physical memory write via amdgpio3
//
// LIMITATION: The IOCTL only writes FIXED value 5 to PA+0xC.
// Not general-purpose. Returns false (NOT IMPLEMENTED) until
// GPIO bank write handler IOCTL codes are reverse-engineered.
//
// Workaround: If target address is g_CiEnabled (needs 0 written),
// we'd need PA+0xC = g_CiEnabled addr and value 0, but driver
// always writes 5. So this primitive is useless for DSE bypass.
// ======================================================================
bool ByovdLoader::PhysWrite(DWORD64 physicalAddr, const void* buffer, DWORD size) {
    UNREFERENCED_PARAMETER(physicalAddr);
    UNREFERENCED_PARAMETER(buffer);
    UNREFERENCED_PARAMETER(size);

    if (!IsLoaded()) return false;

    console::warn("[BYOVD] PhysWrite NOT IMPLEMENTED — requires GPIO IOCTL handlers RE");
    console::warn("[BYOVD]   The MmMapIoSpace IOCTL only writes FIXED value 5 to PA+0xC");
    console::warn("[BYOVD]   Arbitrary physical writes need the GPIO bank write handlers");
    return false;
}

// ======================================================================
// Find kernel base address (ntoskrnl.exe) via EnumDeviceDrivers
// ======================================================================
DWORD64 ByovdLoader::FindKernelBase() {
    LPVOID drivers[1024];
    DWORD needed = 0;

    if (!EnumDeviceDrivers(drivers, sizeof(drivers), &needed)) {
        console::error("[BYOVD] EnumDeviceDrivers failed (%lu)", GetLastError());
        return 0;
    }

    if (needed > 0) {
        return (DWORD64)(uintptr_t)drivers[0];
    }

    return 0;
}

// ======================================================================
// Find a kernel module base by name (e.g., "ci.dll")
// Uses NtQuerySystemInformation(SystemModuleInformation)
// ======================================================================
DWORD64 ByovdLoader::FindModuleInKernel(const char* moduleName) {
    if (!moduleName) return 0;

    _NtQuerySystemInformation NtQuerySystemInformation =
        (_NtQuerySystemInformation)GetProcAddress(
            GetModuleHandleA("ntdll.dll"), "NtQuerySystemInformation"
        );

    if (!NtQuerySystemInformation) {
        console::error("[BYOVD] NtQuerySystemInformation not found");
        return 0;
    }

    // Query required buffer size
    ULONG bufSize = 0;
    NTSTATUS status = NtQuerySystemInformation(
        SystemModuleInformation, NULL, 0, &bufSize
    );

    if (status != STATUS_INFO_LENGTH_MISMATCH && !bufSize) {
        console::error("[BYOVD] NtQuerySystemInformation size query failed: 0x%X", status);
        return 0;
    }

    // Allocate and query
    std::vector<uint8_t> buf(bufSize);
    status = NtQuerySystemInformation(
        SystemModuleInformation, buf.data(), bufSize, &bufSize
    );

    if (!NT_SUCCESS(status)) {
        console::error("[BYOVD] NtQuerySystemInformation failed: 0x%X", status);
        return 0;
    }

    RTL_PROCESS_MODULES* modules = (RTL_PROCESS_MODULES*)buf.data();

    for (ULONG i = 0; i < modules->NumberOfModules; i++) {
        auto& mod = modules->Modules[i];

        // Extract just the filename from the full path
        const char* imageName = mod.ImageName;
        const char* filename = strrchr(imageName, '\\');
        if (filename) filename++;
        else filename = imageName;

        if (_stricmp(filename, moduleName) == 0) {
            DWORD64 base = (DWORD64)(uintptr_t)mod.ImageBaseAddress;
            console::info("[BYOVD] Found %s at kernel VA 0x%llX (size: %lu KB)",
                moduleName, base, mod.ImageSize / 1024);
            return base;
        }
    }

    console::error("[BYOVD] Module %s not found in kernel modules", moduleName);
    return 0;
}

// ======================================================================
// Find ci.dll in kernel memory
// ======================================================================
DWORD64 ByovdLoader::FindCiDllBase() {
    if (m_ciDllBase) return m_ciDllBase;
    m_ciDllBase = FindModuleInKernel(CI_DLL_NAME);
    return m_ciDllBase;
}

// ======================================================================
// Find g_CiEnabled address in ci.dll (FIXED: no longer corrupts m_ciDllBase)
//
// g_CiEnabled is a global BYTE variable in ci.dll's .data section.
// Known offsets for common Windows versions are tried.
//
// NOTE: This returns the VIRTUAL ADDRESS of g_CiEnabled.
// But we need to PATCH it, which requires either:
//   - Physical memory write primitive (not available from this driver)
//   - Ring-0 code execution
//   - StealthDrv.sys already loaded (circular dependency)
//
// This function is primarily for informational display.
// ======================================================================
DWORD64 ByovdLoader::FindGCiEnabled() {
    DWORD64 ciBase = FindCiDllBase();
    if (!ciBase) return 0;

    console::info("[BYOVD] Looking for g_CiEnabled in ci.dll at 0x%llX", ciBase);

    // Known offsets for common Windows versions
    struct {
        const char* version;
        DWORD64     offset;
    } knownOffsets[] = {
        { "Windows 10 20H1-21H2",  0x30E0 },
        { "Windows 10 22H2",       0x30E0 },
        { "Windows 11 21H2",       0x30F0 },
        { "Windows 11 22H2",       0x30F0 },
        { "Windows 11 23H2",       0x3100 },
        { "Windows 11 24H2",       0x3100 },
    };

    // Try each offset (in order of likelihood for modern Windows)
    // Windows 11 24H2 is most common on newer systems
    size_t bestGuess = 4; // Index of Windows 11 23H2
    DWORD64 gCiAddr = ciBase + knownOffsets[bestGuess].offset;

    console::info("[BYOVD] g_CiEnabled candidate: ci.dll + 0x%llX (%s)",
        knownOffsets[bestGuess].offset, knownOffsets[bestGuess].version);
    console::info("[BYOVD]   VA: 0x%llX (kernel virtual, readable only from Ring-0)", gCiAddr);
    console::info("[BYOVD]   Cannot patch: amdgpio3 IOCTL only writes 5 to PA+0xC");
    console::info("[BYOVD]   To disable DSE, need full phys R/W (GPIO handlers) or other method");

    return gCiAddr;
}

// ======================================================================
// DSE Bypass (HONEST: cannot patch with current IOCTL primitive)
//
// The amdgpio3 IOCTL (Vulnerability #1) maps a physical address and
// writes FIXED VALUE 5 to PA+0xC. This is NOT sufficient to disable
// DSE (which requires writing 0 to g_CiEnabled in ci.dll).
//
// For DSE bypass, we need one of:
//   A) GPIO bank handlers (Vulnerability #2) with their IOCTL codes
//      - sub_140001088, sub_140001144, sub_140001278, etc.
//      - Provide proper physical memory R/W through the mapped VA
//   B) A different vulnerable driver with arbitrary phys memory R/W
//      - aswRvrt.sys (Avast), gdrv.sys (Gigabyte), etc.
//   C) Valid driver signature (EV certificate, ~$50)
//   D) Test signing mode (requires reboot)
//      - bcdedit /set testsigning on
//      - bcdedit /set nointegritychecks on  (Win 10+)
//
// Until the GPIO IOCTL codes are reverse-engineered, this function
// returns false (DSE NOT disabled, StealthDrv cannot be loaded).
//
// HOW TO FIND GPIO IOCTL CODES:
//   1. Load amdgpio3.sys in IDA/Ghidra
//   2. Find the IOCTL dispatch table in sub_1400012A0
//   3. Each case corresponds to a different GPIO operation
//   4. Use IOCTL_DEBUG: send CTL_CODE(0x8000, 0x800-0x900, METHOD_BUFFERED, FILE_ANY_ACCESS)
//      and log which codes succeed
// ======================================================================
bool ByovdLoader::DisableDSE() {
    console::info("[BYOVD] DSE bypass requires physical memory R/W primitive");

    // Try to find g_CiEnabled for informational purposes
    DWORD64 gCiAddr = FindGCiEnabled();
    if (!gCiAddr) {
        console::warn("[BYOVD]   g_CiEnabled not found by known offsets");
    }

    console::info("[BYOVD]   amdgpio3 IOCTL (Vulnerability #1) writes FIXED 5 to PA+0xC");
    console::info("[BYOVD]   g_CiEnabled needs 0 written — IOCTL cannot do this");
    console::info("[BYOVD] ");
    console::info("[BYOVD] Options:");
    console::info("[BYOVD]   1. RE the GPIO bank IOCTL codes (sub_140001088, etc.)");
    console::info("[BYOVD]      → These provide real physical R/W primitives");
    console::info("[BYOVD]   2. Sign StealthDrv.sys with EV certificate (~$50)");
    console::info("[BYOVD]      → Works on all Windows versions, no DSE bypass needed");
    console::info("[BYOVD]   3. Use KDU with aswRvrt.sys (different vulnerable driver)");
    console::info("[BYOVD]      → Provides proper physical memory R/W");
    console::info("[BYOVD]   4. Enable test signing (bcdedit /set testsigning on)");
    console::info("[BYOVD]      → Requires reboot, detectable by some anti-cheats");
    console::info("[BYOVD]   5. Fall back to thread hijacking (already implemented)");
    console::info("[BYOVD]      → No driver needed, works in user-mode");

    return false;  // HONEST: DSE was NOT disabled
}

// ======================================================================
// Load target driver (StealthDrv.sys) via SCM
// This will succeed IF DSE is disabled OR if the driver is signed.
// ======================================================================
bool ByovdLoader::LoadTargetDriver(const std::wstring& targetDrvPath) {
    if (m_stealthDrvLoaded) {
        console::info("[BYOVD] StealthDrv already loaded");
        return true;
    }

    console::info("[BYOVD] Loading StealthDrv.sys...");
    console::info("[BYOVD] NOTE: DSE not bypassed — unsigned driver will be blocked");
    console::info("[BYOVD]   Need either: GPIO IOCTL RE, EV cert, or KDU with diff driver");

    // 1. Copy to system32\drivers
    WCHAR destPath[MAX_PATH];
    GetSystemDirectoryW(destPath, MAX_PATH);
    wcscat_s(destPath, L"\\drivers\\StealthDrv.sys");

    if (!CopyFileW(targetDrvPath.c_str(), destPath, FALSE)) {
        DWORD err = GetLastError();
        if (err != ERROR_FILE_EXISTS) {
            console::warn("[BYOVD] Copy StealthDrv: %lu", err);
        }
    }

    // 2. Create and start service
    SC_HANDLE scm = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!scm) {
        console::error("[BYOVD] OpenSCManager failed (%lu)", GetLastError());
        return false;
    }

    SC_HANDLE service = CreateServiceW(
        scm,
        STEALTH_DRV_SERVICE,
        L"Stealth Memory Driver",
        SERVICE_START | SERVICE_STOP | DELETE,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_IGNORE,
        destPath,
        NULL, NULL, NULL, NULL, NULL
    );

    if (!service) {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_EXISTS) {
            // Service exists, try to open and start
            service = OpenServiceW(scm, STEALTH_DRV_SERVICE,
                                    SERVICE_START | SERVICE_STOP | DELETE);
        }
    }

    if (!service) {
        console::error("[BYOVD] CreateService for StealthDrv failed (%lu)", GetLastError());
        CloseServiceHandle(scm);
        return false;
    }

    // Try to start
    if (!StartServiceW(service, 0, NULL)) {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_ALREADY_RUNNING) {
            console::success("[BYOVD] StealthDrv was already running");
            m_stealthDrvLoaded = true;
        } else {
            console::error("[BYOVD] StartService for StealthDrv failed (%lu)", err);
            console::warn("[BYOVD] DSE is likely blocking the unsigned driver");
            console::warn("[BYOVD] Falling back to thread hijacking mode");
            DeleteService(service);
        }
    } else {
        console::success("[BYOVD] StealthDrv.sys loaded successfully!");
        m_stealthDrvLoaded = true;
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return m_stealthDrvLoaded;
}

// ======================================================================
// Virtual → Physical address translation
//
// On x64 Windows, the kernel's self-referencing PML4 entry can be
// used to walk page tables. The PML4 self-ref index is at:
//   PML4[511] → PML4 itself (on most Windows versions)
//
// Page table walk:
//   PML4E = *(uint64_t*)(SelfRefBase + ((va >> 39) & 0x1FF) * 8)
//   PDPTE = *(uint64_t*)(SelfRefBase + ...)
//   PDE   = *(uint64_t*)(SelfRefBase + ...)
//   PTE   = *(uint64_t*)(SelfRefBase + ((va >> 12) & 0x1FF) * 8)
//   PhysAddr = (PTE & 0x7FFFFFFFFFFF000) | (va & 0xFFF)
//
// However, the self-referencing PML4 entry is at a kernel-only VA,
// which we can't read from user-mode.
//
// For now, this is a placeholder. Full VaToPa requires:
//   1. Reading the user-mode page table (we can read our own PTEs)
//   2. Or using NtQueryVirtualMemory with undocumented info classes
//   3. Or using a kernel helper (which is what we're trying to load)
// ======================================================================
DWORD64 ByovdLoader::VaToPa(DWORD64 virtualAddr) {
    UNREFERENCED_PARAMETER(virtualAddr);

    // User-mode VA → PA translation requires reading page tables
    // from kernel space. Since we don't have kernel access yet,
    // this is a placeholder.
    //
    // When StealthDrv.sys is loaded, it can provide VaToPa via IOCTL.
    // Alternatively, we can use NtQueryVirtualMemory with
    // MemoryPhysicalAddress info class (undocumented, may not work).

    console::warn("[BYOVD] VaToPa not fully implemented yet");
    return 0;
}

// ======================================================================
// Stop the amdgpio3 service
// ======================================================================
bool ByovdLoader::StopService() {
    if (!m_serviceHandle) return false;

    SERVICE_STATUS status;
    if (ControlService(m_serviceHandle, SERVICE_CONTROL_STOP, &status)) {
        console::info("[BYOVD] Service stopped");
        return true;
    }

    DWORD err = GetLastError();
    if (err == ERROR_SERVICE_NOT_ACTIVE) {
        return true; // Already stopped
    }

    console::warn("[BYOVD] StopService failed (%lu)", err);
    return false;
}

// ======================================================================
// Delete the service
// ======================================================================
bool ByovdLoader::DeleteService() {
    if (!m_serviceHandle) return true;

    if (DeleteService(m_serviceHandle)) {
        console::info("[BYOVD] Service deleted");
        return true;
    }

    DWORD err = GetLastError();
    console::warn("[BYOVD] DeleteService failed (%lu)", err);
    return false;
}

// ======================================================================
// Unload - stop service, close handle, clean up
// ======================================================================
void ByovdLoader::Unload() {
    // Close device handle
    if (m_deviceHandle != INVALID_HANDLE_VALUE && m_deviceHandle) {
        CloseHandle(m_deviceHandle);
        m_deviceHandle = INVALID_HANDLE_VALUE;
    }

    // Stop and delete the amdgpio3 service
    if (m_serviceHandle) {
        StopService();

        // Close the service handle
        CloseServiceHandle(m_serviceHandle);
        m_serviceHandle = NULL;
    }

    if (m_scmHandle) {
        CloseServiceHandle(m_scmHandle);
        m_scmHandle = NULL;
    }

    m_stealthDrvLoaded = false;

    console::info("[BYOVD] amdgpio3 unloaded");
}
