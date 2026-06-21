#pragma once
#include <winioctl.h>

// ======================================================================
// Stealth Kernel Driver - IOCTL Interface
//
// This header is shared between the kernel driver (driver.c) and the
// usermode client (drv_client.cpp). It defines the IOCTL codes and
// structures used for communication.
//
// Driver device: \\.\StealthDrv
// ======================================================================

// --- IOCTL codes ---
// Function codes (0x800-0x80F)
#define IOCTL_STEALTH_READ_MEMORY  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STEALTH_WRITE_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STEALTH_GET_PID      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STEALTH_GET_MODULE   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STEALTH_PING         CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)

#pragma pack(push, 8)

// --- Structures ---

// IOCTL_STEALTH_READ_MEMORY
// Read memory from target process using MmCopyVirtualMemory
struct StealthReadRequest {
    ULONG64 ProcessId;        // PID of target process
    ULONG64 Address;          // Address to read from
    ULONG   Size;             // Number of bytes to read
    UCHAR   Data[1];          // Output buffer (variable size)
};

// IOCTL_STEALTH_WRITE_MEMORY
// Write memory to target process using MmCopyVirtualMemory
struct StealthWriteRequest {
    ULONG64 ProcessId;        // PID of target process
    ULONG64 Address;          // Address to write to
    ULONG   Size;             // Number of bytes to write
    UCHAR   Data[1];          // Input buffer (variable size)
};

// IOCTL_STEALTH_GET_PID
// Get PID of a process by name
struct StealthPidRequest {
    WCHAR  ProcessName[260];  // Process name (e.g., L"RobloxPlayerBeta.exe")
    ULONG64 ProcessId;        // Output: PID
};

// IOCTL_STEALTH_GET_MODULE
// Get base address of a module in a process
struct StealthModuleRequest {
    ULONG64 ProcessId;        // PID of target process
    WCHAR  ModuleName[64];    // Module name (e.g., L"RobloxPlayerBeta.exe")
    ULONG64 BaseAddress;      // Output: module base
    ULONG   ModuleSize;       // Output: module size
};

#pragma pack(pop)

// --- Status codes ---
#define STEALTH_STATUS_SUCCESS      0
#define STEALTH_STATUS_FAILED       1
#define STEALTH_STATUS_INVALID      2
#define STEALTH_STATUS_NOT_FOUND    3
