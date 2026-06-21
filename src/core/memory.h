#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <vector>
#include <string>
#include <memory>
#include <atomic>
#include <chrono>
#include <random>

// ======================================================================
// Stealth memory systems (chained: driver → APC → syscall)
// ======================================================================
#include "apc_stealth.h"
#include "drv_client.h"

// ======================================================================
// Legacy ASM syscall wrappers (kept as fallback)
// ======================================================================
extern "C" intptr_t readvm(
    HANDLE ProcessHandle,
    PVOID BaseAddress,
    PVOID Buffer,
    SIZE_T NumberOfBytesToRead,
    PSIZE_T NumberOfBytesRead
);

extern "C" intptr_t writevm(
    HANDLE ProcessHandle,
    PVOID BaseAddress,
    PVOID Buffer,
    SIZE_T NumberOfBytesToWrite,
    PSIZE_T NumberOfBytesWritten
);

union rbxdata {
    char Inline[16];
    std::uint64_t Pointer;
};

struct rbxstring {
    rbxdata Data;
    std::uint64_t Length;
    std::uint64_t Capacity;
};

// ======================================================================
// Global toggle: use stealth layer instead of direct syscalls
// Set by ApcStealth::Initialize() based on available methods:
//   1. Kernel Driver (Ring-0 MmCopyVirtualMemory) - most stealthy
//   2. Thread Hijacking (in-process shellcode execution)
//   3. Direct Syscall (NtReadVirtualMemory via luck.asm) - fallback
// ======================================================================
extern std::atomic<bool> g_useApcStealth;

// ======================================================================
// Jitter helpers for random timing (defeat pattern detection)
// ======================================================================
namespace stealth {
    // Add random delay to break predictable access patterns
    inline void jitter() {
        ApcStealth::Jitter();
    }

    // Thread-safe RNG for randomizing operations
    inline int random_range(int min, int max) {
        static thread_local std::mt19937 rng(
            static_cast<unsigned>(
                std::chrono::steady_clock::now().time_since_epoch().count()
            )
        );
        if (min >= max) return min;
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng);
    }

    // Get current stealth mode name for diagnostics
    inline const char* mode_name() {
        return ApcStealth::Instance().ModeName();
    }

    // Check if running with kernel driver (most stealthy mode)
    inline bool is_driver_mode() {
        return ApcStealth::Instance().GetMode() == StealthMode::Driver;
    }
}

// ======================================================================
// Enhanced driver with APC Stealth support
// ======================================================================
class driver final {
public:
    driver() = default;
    ~driver() = default;

    std::uint32_t process(const std::string& Process_Name);
    std::uint64_t module(const std::string& Module_Name);
    bool attach(const std::string& Process_Name);

    std::string readstring(std::uint64_t Address);
    void writestring(std::uint64_t Address, const std::string& Value);

    template <typename T>
    T read(std::uint64_t Address);

    template <typename T>
    void write(std::uint64_t Address, T Value);

    std::uint32_t processid();
    std::uint64_t modulebase();
    HANDLE processhandle();

private:
    std::uint32_t pid;
    std::uint64_t base;
    HANDLE handle;
};

// ======================================================================
// Stealth read/write templates
// When g_useApcStealth is true, these use APC injection instead of syscalls
// ======================================================================
template <typename T>
T driver::read(std::uint64_t Address) {
    T Buffer{};
    if (g_useApcStealth.load(std::memory_order_relaxed)) {
        // Fragmented stealth read with random jitter
        auto& apc = ApcStealth::Instance();
        apc.ReadFragmented(Address, &Buffer, sizeof(T));
    } else {
        // Legacy syscall fallback
        readvm(handle, reinterpret_cast<void*>(Address), &Buffer, sizeof(T), nullptr);
    }
    return Buffer;
}

template <typename T>
void driver::write(std::uint64_t Address, T Value) {
    if (g_useApcStealth.load(std::memory_order_relaxed)) {
        // Stealth write via APC
        auto& apc = ApcStealth::Instance();
        apc.Write(Address, &Value, sizeof(T));
        stealth::jitter();
    } else {
        // Legacy syscall fallback
        writevm(handle, reinterpret_cast<void*>(Address), &Value, sizeof(T), nullptr);
    }
}

inline std::unique_ptr<driver> drive = std::make_unique<driver>();
