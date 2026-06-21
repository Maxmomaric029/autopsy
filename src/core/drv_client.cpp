#include "drv_client.h"
#include "../log.h"
#include "../../driver/driver.h"
#include <string>
#include <vector>

// ======================================================================
// Singleton
// ======================================================================
DriverClient& DriverClient::Instance() {
    static DriverClient s_instance;
    return s_instance;
}

// ======================================================================
// Construction / Destruction
// ======================================================================
DriverClient::DriverClient()
    : m_driverHandle(INVALID_HANDLE_VALUE)
{
    m_lastError[0] = 0;
}

DriverClient::~DriverClient() {
    Disconnect();
}

// ======================================================================
// Connect to kernel driver
// ======================================================================
bool DriverClient::Connect() {
    if (IsConnected()) {
        console::warn("[DRV] Already connected to driver");
        return true;
    }

    m_driverHandle = CreateFileW(
        L"\\\\.\\StealthDrv",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (m_driverHandle == INVALID_HANDLE_VALUE || !m_driverHandle) {
        DWORD err = GetLastError();
        snprintf(m_lastError, sizeof(m_lastError),
            "Failed to connect to StealthDrv (error %lu)", err);
        console::error("[DRV] %s", m_lastError);
        return false;
    }

    // Verify with ping
    if (!Ping()) {
        console::warn("[DRV] Connected but ping failed - driver may be stale");
    } else {
        console::success("[DRV] Connected to StealthDrv kernel driver");
    }

    return true;
}

// ======================================================================
// Disconnect from driver
// ======================================================================
void DriverClient::Disconnect() {
    if (IsConnected()) {
        CloseHandle(m_driverHandle);
    }
    m_driverHandle = INVALID_HANDLE_VALUE;
}

// ======================================================================
// Ping - verify driver is responsive
// ======================================================================
bool DriverClient::Ping() {
    if (!IsConnected()) return false;

    DWORD bytesReturned = 0;
    BOOL success = DeviceIoControl(
        m_driverHandle,
        IOCTL_STEALTH_PING,
        NULL, 0,                  // input
        NULL, 0,                  // output
        &bytesReturned,
        NULL
    );

    return success != FALSE;
}

// ======================================================================
// Read memory via kernel driver
// ======================================================================
bool DriverClient::Read(uint64_t pid, uint64_t address, void* buffer, size_t size) {
    if (!IsConnected()) return false;
    if (!buffer || size == 0) return true;

    // Allocate IOCTL buffer (input = request, output = request + data)
    size_t bufSize = sizeof(StealthReadRequest) + size;
    if (bufSize > kMaxBufferSize) {
        console::error("[DRV] Read size %zu exceeds max buffer", size);
        return false;
    }

    std::vector<uint8_t> ioBuf(bufSize, 0);
    StealthReadRequest* req = reinterpret_cast<StealthReadRequest*>(ioBuf.data());
    req->ProcessId = pid;
    req->Address = address;
    req->Size = (ULONG)size;

    DWORD bytesReturned = 0;
    BOOL success = DeviceIoControl(
        m_driverHandle,
        IOCTL_STEALTH_READ_MEMORY,
        ioBuf.data(), (DWORD)bufSize,     // input
        ioBuf.data(), (DWORD)bufSize,     // output
        &bytesReturned,
        NULL
    );

    if (!success) {
        DWORD err = GetLastError();
        snprintf(m_lastError, sizeof(m_lastError),
            "Driver read failed (error %lu)", err);
        console::error("[DRV] %s", m_lastError);
        return false;
    }

    // Copy data from output
    size_t dataSize = bytesReturned - sizeof(StealthReadRequest);
    if (dataSize > 0) {
        memcpy(buffer, req->Data, min(dataSize, size));
    }

    return true;
}

// ======================================================================
// Write memory via kernel driver
// ======================================================================
bool DriverClient::Write(uint64_t pid, uint64_t address, const void* buffer, size_t size) {
    if (!IsConnected()) return false;
    if (!buffer || size == 0) return true;

    // Allocate IOCTL buffer (input = request + data)
    size_t bufSize = sizeof(StealthWriteRequest) + size;
    if (bufSize > kMaxBufferSize) {
        console::error("[DRV] Write size %zu exceeds max buffer", size);
        return false;
    }

    std::vector<uint8_t> ioBuf(bufSize, 0);
    StealthWriteRequest* req = reinterpret_cast<StealthWriteRequest*>(ioBuf.data());
    req->ProcessId = pid;
    req->Address = address;
    req->Size = (ULONG)size;
    memcpy(req->Data, buffer, size);

    DWORD bytesReturned = 0;
    BOOL success = DeviceIoControl(
        m_driverHandle,
        IOCTL_STEALTH_WRITE_MEMORY,
        ioBuf.data(), (DWORD)bufSize,     // input
        ioBuf.data(), (DWORD)bufSize,     // output
        &bytesReturned,
        NULL
    );

    if (!success) {
        DWORD err = GetLastError();
        snprintf(m_lastError, sizeof(m_lastError),
            "Driver write failed (error %lu)", err);
        console::error("[DRV] %s", m_lastError);
        return false;
    }

    return true;
}

// ======================================================================
// Get process ID via kernel driver
// ======================================================================
uint64_t DriverClient::GetProcessId(const std::wstring& processName) {
    if (!IsConnected()) return 0;

    StealthPidRequest req{};
    wcsncpy_s(req.ProcessName, processName.c_str(), _TRUNCATE);

    DWORD bytesReturned = 0;
    BOOL success = DeviceIoControl(
        m_driverHandle,
        IOCTL_STEALTH_GET_PID,
        &req, sizeof(req),          // input
        &req, sizeof(req),          // output
        &bytesReturned,
        NULL
    );

    if (!success) {
        console::error("[DRV] GetProcessId failed for %S", processName.c_str());
        return 0;
    }

    return req.ProcessId;
}

// ======================================================================
// Get module base via kernel driver
// ======================================================================
uint64_t DriverClient::GetModuleBase(uint64_t pid, const std::wstring& moduleName) {
    if (!IsConnected()) return 0;

    StealthModuleRequest req{};
    req.ProcessId = pid;
    wcsncpy_s(req.ModuleName, moduleName.c_str(), _TRUNCATE);

    DWORD bytesReturned = 0;
    BOOL success = DeviceIoControl(
        m_driverHandle,
        IOCTL_STEALTH_GET_MODULE,
        &req, sizeof(req),          // input
        &req, sizeof(req),          // output
        &bytesReturned,
        NULL
    );

    if (!success) {
        console::warn("[DRV] GetModuleBase failed for %S", moduleName.c_str());
        return 0;
    }

    return req.BaseAddress;
}
