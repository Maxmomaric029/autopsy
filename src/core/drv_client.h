#pragma once
#include <windows.h>
#include <cstdint>
#include <string>

// ======================================================================
// Kernel Driver Client
//
// Provides usermode access to the Ring-0 stealth driver.
// The driver uses MmCopyVirtualMemory for memory operations that
// cannot be intercepted by user-mode anti-cheats (Hyperion, etc.).
//
// Communication: \\.\StealthDrv via DeviceIoControl
// ======================================================================

class DriverClient {
public:
    DriverClient();
    ~DriverClient();

    // --- Lifecycle ---
    bool Connect();
    void Disconnect();
    bool IsConnected() const { return m_driverHandle != INVALID_HANDLE_VALUE && m_driverHandle != NULL; }

    // --- Memory operations (all go through kernel via IOCTL) ---
    bool Read(uint64_t pid, uint64_t address, void* buffer, size_t size);
    bool Write(uint64_t pid, uint64_t address, const void* buffer, size_t size);

    // --- Convenience templates ---
    template<typename T>
    T Read(uint64_t pid, uint64_t address) {
        T val{};
        Read(pid, address, &val, sizeof(T));
        return val;
    }

    template<typename T>
    void Write(uint64_t pid, uint64_t address, T value) {
        Write(pid, address, &value, sizeof(T));
    }

    // --- Process helpers (optional, can use usermode as fallback) ---
    uint64_t GetProcessId(const std::wstring& processName);
    uint64_t GetModuleBase(uint64_t pid, const std::wstring& moduleName);

    // --- Status ---
    bool Ping();
    const char* LastError() const { return m_lastError; }

    // Singleton
    static DriverClient& Instance();

private:
    HANDLE  m_driverHandle;
    char    m_lastError[256];

    // IOCTL buffer size limits
    static constexpr size_t kMaxBufferSize = 4096;
};
