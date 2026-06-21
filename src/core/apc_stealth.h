#pragma once
#include <windows.h>
#include <cstdint>
#include <vector>
#include <atomic>
#include <random>
#include <chrono>

// ======================================================================
// APC Stealth Memory System v2
//
// Instead of calling ReadProcessMemory/WriteProcessMemory externally,
// we execute shellcode INSIDE the target process via thread hijacking
// (SuspendThread + SetThreadContext + ResumeThread).
//
// This is stealthier than QueueUserAPC because:
//   - No APC queue needed (can be monitored by anti-cheats)
//   - Thread executes our code IMMEDIATELY (no alertable state needed)
//   - Original context is restored after execution
//
// Stealth chain priority:
//   1. Kernel Driver (Ring-0 MmCopyVirtualMemory) - most stealthy
//   2. APC Thread Hijacking (in-process execution)   - stealthy
//   3. Direct Syscall (NtReadVirtualMemory via asm)   - fallback
// ======================================================================

// Operation types for shellcode payload
enum HijackOperation : LONG {
    HIJACK_OP_IDLE = 0,
    HIJACK_OP_READ = 1,
    HIJACK_OP_WRITE = 2,
};

// Payload structure written into target process memory
// The shellcode reads this structure to know what to do
#pragma pack(push, 8)
struct HijackPayload {
    volatile LONG    operation;      // HijackOperation
    uint64_t         target_addr;    // game address to read/write
    uint64_t         buffer_addr;    // buffer address IN TARGET PROCESS
    size_t           size;           // number of bytes
    volatile LONG    done;           // 1 when shellcode completes
    uint8_t          data[256];      // inline buffer for small transfers
};
#pragma pack(pop)

// Stealth mode selection
enum class StealthMode : int {
    Driver = 0,     // Kernel driver (most stealthy)
    ThreadHijack,   // Thread hijacking (stealthy)
    DirectSyscall,  // Direct syscall (least stealthy, but works without driver)
};

class ApcStealth {
public:
    ApcStealth();
    ~ApcStealth();

    // --- Lifecycle ---
    bool Initialize(HANDLE target_process, uint64_t module_base);
    void Shutdown();

    // --- Mode ---
    void SetMode(StealthMode mode) { m_mode = mode; }
    StealthMode GetMode() const { return m_mode; }
    const char* ModeName() const;

    // --- Stealth memory operations ---
    // These execute inside the target process via thread hijacking OR driver
    bool Read(uint64_t address, void* buffer, size_t size);
    bool Write(uint64_t address, const void* buffer, size_t size);

    // Templated convenience wrappers
    template<typename T>
    T Read(uint64_t address) {
        T val{};
        Read(address, &val, sizeof(T));
        return val;
    }

    template<typename T>
    void Write(uint64_t address, T value) {
        Write(address, &value, sizeof(T));
    }

    // --- Fragmented read with random jitter ---
    bool ReadFragmented(uint64_t address, void* buffer, size_t size, size_t chunk_size = 0);

    // --- Thread management ---
    HANDLE FindHijackThread();

    // --- Status ---
    bool IsInitialized() const { return m_initialized; }
    bool IsDriverAvailable() const { return m_driverAvailable; }

    // --- Random timing ---
    static void Jitter();

    // Singleton access
    static ApcStealth& Instance();

private:
    // Shellcode management
    bool BuildShellcode();
    bool InjectShellcode();

    // Thread hijacking execution
    bool ExecuteThreadHijack(HANDLE thread, HijackPayload* payload);

    // Direct syscall helpers (use luck.asm readvm/writevm)
    bool ReadViaSyscall(uint64_t address, void* buffer, size_t size);
    bool WriteViaSyscall(uint64_t address, const void* buffer, size_t size);

    // Convert protoctions (RW→RX to avoid RWX detection)
    bool ProtectToExec(void* address, size_t size);

    // Generate random number for jitter
    int RandomRange(int min, int max);

    // State
    HANDLE     m_targetProcess;
    uint64_t   m_moduleBase;
    DWORD      m_targetPid;
    bool       m_initialized;
    bool       m_driverAvailable;
    StealthMode m_mode;

    // Shellcode (injected into target process)
    void*      m_remoteShellcode;    // address in target process
    size_t     m_shellcodeSize;

    // Pre-allocated payload area in target process
    void*      m_remotePayload;      // address in target process

    // Saved original thread contexts for restoration
    struct SavedContext {
        HANDLE  thread;
        CONTEXT ctx;
        DWORD   suspendCount;
    };
    std::vector<SavedContext> m_savedContexts;

    // Thread round-robin
    size_t m_nextThreadIndex;

    // RNG
    std::mt19937 m_rng;
};
