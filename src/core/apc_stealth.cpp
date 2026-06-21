#include "apc_stealth.h"
#include "drv_client.h"
#include "../log.h"
#include <TlHelp32.h>
#include <thread>
#include <cstddef>   // offsetof

// Forward declarations for luck.asm syscalls
extern "C" intptr_t readvm(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
extern "C" intptr_t writevm(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);

// ======================================================================
// x64 shellcode: Thread hijack payload executor
//
// This shellcode runs INSIDE the target process after a thread is
// hijacked (SuspendThread → SetThreadContext with RIP=shellcode).
// It reads the HijackPayload struct, copies memory, sets done flag.
//
// Entry: rcx = pointer to HijackPayload (in target process memory)
//
// Shellcode (NASM):
//   push rbx
//   push rsi  
//   push rdi
//   mov rbx, rcx              ; rbx = payload address
//   cld                       ; clear direction flag
//   mov eax, [rbx]            ; operation
//   cmp eax, 1                ; HIJACK_OP_READ?
//   je .read
//   cmp eax, 2                ; HIJACK_OP_WRITE?
//   je .write
//   jmp .done
// .read:
//   mov rsi, [rbx + 0x08]     ; src = payload.target_addr
//   mov rdi, [rbx + 0x10]     ; dst = payload.buffer_addr
//   mov rcx, [rbx + 0x18]     ; cnt = payload.size
//   rep movsb
//   jmp .done
// .write:
//   mov rsi, [rbx + 0x10]     ; src = payload.buffer_addr
//   mov rdi, [rbx + 0x08]     ; dst = payload.target_addr
//   mov rcx, [rbx + 0x18]     ; cnt = payload.size
//   rep movsb
// .done:
//   mov dword [rbx + 0x20], 1 ; payload.done = 1
//   pop rdi
//   pop rsi
//   pop rbx
//   ret
// ======================================================================

static const uint8_t kShellcodeBytes[] = {
    0x53,                         // push rbx
    0x56,                         // push rsi
    0x57,                         // push rdi
    0x48, 0x89, 0xCB,             // mov rbx, rcx
    0xFC,                         // cld
    0x8B, 0x03,                   // mov eax, [rbx]
    0x83, 0xF8, 0x01,             // cmp eax, 1
    0x74, 0x07,                   // je .read (0x15)
    0x83, 0xF8, 0x02,             // cmp eax, 2
    0x74, 0x12,                   // je .write (0x25)
    0xEB, 0x1E,                   // jmp .done (0x33)
    // .read (0x15):
    0x48, 0x8B, 0x73, 0x08,       // mov rsi, [rbx + 0x08]
    0x48, 0x8B, 0x7B, 0x10,       // mov rdi, [rbx + 0x10]
    0x48, 0x8B, 0x4B, 0x18,       // mov rcx, [rbx + 0x18]
    0xF3, 0xA4,                   // rep movsb
    0xEB, 0x0E,                   // jmp .done
    // .write (0x25):
    0x48, 0x8B, 0x73, 0x10,       // mov rsi, [rbx + 0x10]
    0x48, 0x8B, 0x7B, 0x08,       // mov rdi, [rbx + 0x08]
    0x48, 0x8B, 0x4B, 0x18,       // mov rcx, [rbx + 0x18]
    0xF3, 0xA4,                   // rep movsb
    // .done (0x33):
    0xC7, 0x43, 0x20, 0x01, 0x00, 0x00, 0x00,
    0x5F,                         // pop rdi
    0x5E,                         // pop rsi
    0x5B,                         // pop rbx
    0xC3                          // ret
};

static constexpr size_t kShellcodeSize = sizeof(kShellcodeBytes);
static constexpr size_t kPayloadSize = sizeof(HijackPayload);
static constexpr size_t kDefaultChunkSize = 32;
static constexpr int kJitterMinMs = 1;
static constexpr int kJitterMaxMs = 15;
static constexpr int kHijackTimeoutMs = 50;

// ======================================================================
// Singleton
// ======================================================================
ApcStealth& ApcStealth::Instance() {
    static ApcStealth s_instance;
    return s_instance;
}

// ======================================================================
// Construction / Destruction
// ======================================================================
ApcStealth::ApcStealth()
    : m_targetProcess(NULL)
    , m_moduleBase(0)
    , m_targetPid(0)
    , m_initialized(false)
    , m_driverAvailable(false)
    , m_mode(StealthMode::DirectSyscall)  // default fallback
    , m_remoteShellcode(nullptr)
    , m_shellcodeSize(0)
    , m_remotePayload(nullptr)
    , m_nextThreadIndex(0)
    , m_rng(static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count()))
{
}

ApcStealth::~ApcStealth() {
    Shutdown();
}

// ======================================================================
// Mode name
// ======================================================================
const char* ApcStealth::ModeName() const {
    switch (m_mode) {
    case StealthMode::Driver:        return "Kernel Driver (Ring-0)";
    case StealthMode::ThreadHijack:  return "Thread Hijack (in-process)";
    case StealthMode::DirectSyscall: return "Direct Syscall (legacy)";
    default:                         return "Unknown";
    }
}

// ======================================================================
// Initialize
//
// Priority chain:
//   1. Kernel driver (most stealthy)
//   2. Thread hijacking (stealthy, requires shellcode injection)
//   3. Direct syscall (fallback, always available via luck.asm)
// ======================================================================
bool ApcStealth::Initialize(HANDLE target_process, uint64_t module_base) {
    if (m_initialized) {
        console::warn("[APC] Already initialized");
        return true;
    }

    if (!target_process || target_process == INVALID_HANDLE_VALUE) {
        console::error("[APC] Invalid target process handle");
        return false;
    }

    m_targetProcess = target_process;
    m_moduleBase = module_base;
    m_targetPid = GetProcessId(target_process);

    // --- Try kernel driver first (most stealthy) ---
    auto& drv = DriverClient::Instance();
    if (drv.Connect() && drv.Ping()) {
        m_driverAvailable = true;
        m_mode = StealthMode::Driver;
        m_initialized = true;
        console::success("[APC] Modo: %s — operaciones via MmCopyVirtualMemory (Ring-0)", ModeName());
        return true;
    } else {
        m_driverAvailable = false;
        console::warn("[APC] Driver no disponible, intentando thread hijacking...");
    }

    // --- Try thread hijacking (stealthy, in-process execution) ---
    if (!BuildShellcode()) {
        console::error("[APC] Failed to build shellcode");
        m_mode = StealthMode::DirectSyscall;
        m_initialized = true;  // syscall always works
        console::warn("[APC] Modo: %s (fallback)", ModeName());
        return true;
    }

    if (!InjectShellcode()) {
        console::warn("[APC] Shellcode injection failed, usando syscall fallback");
        m_mode = StealthMode::DirectSyscall;
        m_initialized = true;
        return true;
    }

    // Allocate payload area (RW first, not RWX)
    m_remotePayload = VirtualAllocEx(
        m_targetProcess, nullptr, kPayloadSize,
        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE
    );

    if (!m_remotePayload) {
        console::error("[APC] Failed to allocate payload (error %lu)", GetLastError());
        VirtualFreeEx(m_targetProcess, m_remoteShellcode, 0, MEM_RELEASE);
        m_remoteShellcode = nullptr;
        m_mode = StealthMode::DirectSyscall;
        m_initialized = true;
        return true;
    }

    m_mode = StealthMode::ThreadHijack;
    m_initialized = true;
    console::success("[APC] Modo: %s — shellcode en 0x%llX, payload en 0x%llX",
        ModeName(),
        (unsigned long long)(uintptr_t)m_remoteShellcode,
        (unsigned long long)(uintptr_t)m_remotePayload);

    return true;
}

// ======================================================================
// Shutdown
// ======================================================================
void ApcStealth::Shutdown() {
    if (m_remotePayload) {
        VirtualFreeEx(m_targetProcess, m_remotePayload, 0, MEM_RELEASE);
        m_remotePayload = nullptr;
    }
    if (m_remoteShellcode) {
        VirtualFreeEx(m_targetProcess, m_remoteShellcode, 0, MEM_RELEASE);
        m_remoteShellcode = nullptr;
    }
    DriverClient::Instance().Disconnect();
    m_initialized = false;
}

// ======================================================================
// BuildShellcode
// ======================================================================
bool ApcStealth::BuildShellcode() {
    m_shellcodeBytes.assign(kShellcodeBytes, kShellcodeBytes + kShellcodeSize);
    m_shellcodeSize = m_shellcodeBytes.size();
    console::info("[APC] Shellcode built: %zu bytes", m_shellcodeSize);
    return true;
}

// ======================================================================
// InjectShellcode - allocates RW then changes to RX (avoids RWX)
// ======================================================================
bool ApcStealth::InjectShellcode() {
    // 1. Allocate as PAGE_READWRITE (not RWX — RWX is heavily monitored)
    m_remoteShellcode = VirtualAllocEx(
        m_targetProcess, nullptr, m_shellcodeSize,
        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE
    );

    if (!m_remoteShellcode) {
        console::error("[APC] VirtualAllocEx failed (error %lu)", GetLastError());
        return false;
    }

    // 2. Write shellcode bytes via DIRECT SYSCALL (bypass user-mode hooks)
    SIZE_T written = 0;
    intptr_t result = writevm(
        m_targetProcess,
        m_remoteShellcode,
        (void*)m_shellcodeBytes.data(),
        m_shellcodeSize,
        &written
    );

    if (result != 0 || written != m_shellcodeSize) {
        console::error("[APC] writevm (syscall) failed for shellcode");
        VirtualFreeEx(m_targetProcess, m_remoteShellcode, 0, MEM_RELEASE);
        m_remoteShellcode = nullptr;
        return false;
    }

    // 3. Change protection to PAGE_EXECUTE_READ (stealthier than RWX)
    DWORD oldProtect = 0;
    if (!VirtualProtectEx(m_targetProcess, m_remoteShellcode, m_shellcodeSize,
                          PAGE_EXECUTE_READ, &oldProtect)) {
        // Fallback: keep as RW if we can't change to RX
        // This still works, but is slightly less stealthy
        console::warn("[APC] VirtualProtectEx to RX failed, keeping RW (error %lu)",
            GetLastError());
    }

    console::info("[APC] Shellcode injected at 0x%llX via syscall (%zu bytes, prot=RX)",
        (unsigned long long)(uintptr_t)m_remoteShellcode, written);

    return true;
}

// ======================================================================
// Change protection from RW to RX (avoids RWX detection)
// ======================================================================
bool ApcStealth::ProtectToExec(void* address, size_t size) {
    DWORD oldProtect = 0;
    return VirtualProtectEx(
        m_targetProcess, address, size,
        PAGE_EXECUTE_READ, &oldProtect
    ) != FALSE;
}

// ======================================================================
// Find a thread to hijack in the target process
// Uses CreateToolhelp32Snapshot to enumerate threads.
// ======================================================================
HANDLE ApcStealth::FindHijackThread() {
    if (!m_targetProcess) return NULL;

    DWORD targetPid = m_targetPid;
    if (!targetPid) {
        targetPid = GetProcessId(m_targetProcess);
        if (!targetPid) return NULL;
    }

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return NULL;

    std::vector<DWORD> threadIds;
    threadIds.reserve(32);

    THREADENTRY32 te{ sizeof(THREADENTRY32) };
    if (Thread32First(hSnapshot, &te)) {
        do {
            if (te.th32OwnerProcessID == targetPid)
                threadIds.push_back(te.th32ThreadID);
        } while (Thread32Next(hSnapshot, &te));
    }
    CloseHandle(hSnapshot);

    if (threadIds.empty()) return NULL;

    // Round-robin: pick a non-main thread to avoid UI lag
    size_t startIndex = m_nextThreadIndex % threadIds.size();
    for (size_t i = 0; i < threadIds.size(); i++) {
        size_t idx = (startIndex + i) % threadIds.size();
        DWORD tid = threadIds[idx];

        HANDLE hThread = OpenThread(
            THREAD_GET_CONTEXT | THREAD_SET_CONTEXT |
            THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION,
            FALSE, tid
        );

        if (hThread) {
            m_nextThreadIndex = idx + 1;
            return hThread;
        }
    }

    return NULL;
}

// ======================================================================
// ExecuteThreadHijack - suspends thread, sets RIP=shellcode, resumes
//
// This replaces QueueUserAPC with thread context manipulation:
//   1. SuspendThread(hThread)
//   2. GetThreadContext → save RIP, RSP, RCX
//   3. SetThreadContext: RIP = shellcode, RCX = payload
//   4. ResumeThread (shellcode executes immediately)
//   5. Wait kHijackTimeoutMs for shellcode to complete
//   6. SuspendThread, restore context, ResumeThread
// ======================================================================
bool ApcStealth::ExecuteThreadHijack(HANDLE thread, HijackPayload* payload) {
    // 1. Write payload to target process via DIRECT SYSCALL
    SIZE_T written = 0;
    intptr_t result = writevm(
        m_targetProcess,
        m_remotePayload,
        payload,
        kPayloadSize,
        &written
    );

    if (result != 0) {
        console::error("[APC] writevm for payload failed");
        return false;
    }

    // 2. Suspend the target thread
    DWORD suspendCount = SuspendThread(thread);
    if (suspendCount == (DWORD)-1) {
        console::error("[APC] SuspendThread failed (error %lu)", GetLastError());
        return false;
    }

    // 3. Save original context and hijack
    CONTEXT ctx{};
    ctx.ContextFlags = CONTEXT_FULL;

    if (!GetThreadContext(thread, &ctx)) {
        console::error("[APC] GetThreadContext failed (error %lu)", GetLastError());
        ResumeThread(thread);
        return false;
    }

    // Save for restoration
    SavedContext saved;
    saved.thread = thread;
    saved.ctx = ctx;
    saved.suspendCount = suspendCount;

    // 4. Hijack: set RIP to shellcode, RCX to payload
    ctx.Rip = reinterpret_cast<DWORD64>(m_remoteShellcode);
    ctx.Rcx = reinterpret_cast<DWORD64>(m_remotePayload);
    ctx.ContextFlags = CONTEXT_CONTROL;

    if (!SetThreadContext(thread, &ctx)) {
        console::error("[APC] SetThreadContext failed (error %lu)", GetLastError());
        ResumeThread(thread);
        return false;
    }

    // 5. Resume thread - shellcode executes immediately
    ResumeThread(thread);

    // 6. Wait for shellcode to complete (microseconds to execute)
    Sleep(kHijackTimeoutMs);

    // 7. Suspend again to restore context
    SuspendThread(thread);

    // Read the done flag to verify execution
    LONG doneCheck = 0;
    result = readvm(
        m_targetProcess,
        static_cast<BYTE*>(m_remotePayload) + offsetof(HijackPayload, done),
        &doneCheck,
        sizeof(doneCheck),
        nullptr
    );

    if (doneCheck != 1) {
        console::warn("[APC] Shellcode may not have completed (done=%d)", doneCheck);
    }

    // 8. Restore original context
    saved.ctx.ContextFlags = CONTEXT_FULL;
    SetThreadContext(thread, &saved.ctx);

    // Resume to original suspend count
    for (DWORD i = 0; i < suspendCount; i++)
        ResumeThread(thread);

    return true;
}

// ======================================================================
// Read via direct syscall (luck.asm)
// Used when in DirectSyscall mode or for internal operations
// ======================================================================
bool ApcStealth::ReadViaSyscall(uint64_t address, void* buffer, size_t size) {
    if (!m_targetProcess) return false;
    intptr_t result = readvm(
        m_targetProcess,
        reinterpret_cast<void*>(address),
        buffer,
        size,
        nullptr
    );
    return result == 0;
}

// ======================================================================
// Write via direct syscall (luck.asm)
// ======================================================================
bool ApcStealth::WriteViaSyscall(uint64_t address, const void* buffer, size_t size) {
    if (!m_targetProcess) return false;
    intptr_t result = writevm(
        m_targetProcess,
        reinterpret_cast<void*>(address),
        const_cast<void*>(buffer),
        size,
        nullptr
    );
    return result == 0;
}

// ======================================================================
// Stealth Read - chooses method based on current mode
// ======================================================================
bool ApcStealth::Read(uint64_t address, void* buffer, size_t size) {
    if (!m_initialized || size == 0) return size == 0;

    switch (m_mode) {
    case StealthMode::Driver:
        // Kernel driver - most stealthy
        return DriverClient::Instance().Read(m_targetPid, address, buffer, size);

    case StealthMode::ThreadHijack: {
        // Thread hijacking - in-process execution
        bool useInline = size <= sizeof(HijackPayload::data);

        void* remoteBuffer = nullptr;
        if (!useInline) {
            remoteBuffer = VirtualAllocEx(
                m_targetProcess, nullptr, size,
                MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE
            );
            if (!remoteBuffer) return false;
        }

        // Build payload
        HijackPayload payload{};
        payload.operation = HIJACK_OP_READ;
        payload.target_addr = address;
        payload.buffer_addr = useInline
            ? reinterpret_cast<uint64_t>(m_remotePayload) + offsetof(HijackPayload, data)
            : reinterpret_cast<uint64_t>(remoteBuffer);
        payload.size = size;
        payload.done = 0;

        // Write payload and execute via thread hijacking
        HANDLE hThread = FindHijackThread();
        if (!hThread) {
            if (remoteBuffer) VirtualFreeEx(m_targetProcess, remoteBuffer, 0, MEM_RELEASE);
            return false;
        }

        if (!ExecuteThreadHijack(hThread, &payload)) {
            CloseHandle(hThread);
            if (remoteBuffer) VirtualFreeEx(m_targetProcess, remoteBuffer, 0, MEM_RELEASE);
            return false;
        }

        CloseHandle(hThread);

        // Read results back via syscall (small, targeted read)
        if (useInline) {
            void* srcOffset = static_cast<uint8_t*>(m_remotePayload) + offsetof(HijackPayload, data);
            ReadViaSyscall(reinterpret_cast<uint64_t>(srcOffset), buffer, size);
        } else {
            ReadViaSyscall(reinterpret_cast<uint64_t>(remoteBuffer), buffer, size);
            VirtualFreeEx(m_targetProcess, remoteBuffer, 0, MEM_RELEASE);
        }

        return true;
    }

    case StealthMode::DirectSyscall:
        // Direct syscall fallback
        return ReadViaSyscall(address, buffer, size);

    default:
        return false;
    }
}

// ======================================================================
// Stealth Write - chooses method based on current mode
// ======================================================================
bool ApcStealth::Write(uint64_t address, const void* buffer, size_t size) {
    if (!m_initialized || size == 0) return size == 0;

    switch (m_mode) {
    case StealthMode::Driver:
        return DriverClient::Instance().Write(m_targetPid, address, buffer, size);

    case StealthMode::ThreadHijack: {
        bool useInline = size <= sizeof(HijackPayload::data);

        if (useInline) {
            // Build payload with embedded data
            HijackPayload payload{};
            payload.operation = HIJACK_OP_WRITE;
            payload.target_addr = address;
            payload.buffer_addr = reinterpret_cast<uint64_t>(m_remotePayload) + offsetof(HijackPayload, data);
            payload.size = size;
            payload.done = 0;
            memcpy(payload.data, buffer, size);

            HANDLE hThread = FindHijackThread();
            if (!hThread) return false;

            bool ok = ExecuteThreadHijack(hThread, &payload);
            CloseHandle(hThread);
            return ok;
        }

        // Large write - allocate separate buffer
        void* remoteBuffer = VirtualAllocEx(
            m_targetProcess, nullptr, size,
            MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE
        );
        if (!remoteBuffer) return false;

        // Write data to remote buffer via syscall
        WriteViaSyscall(reinterpret_cast<uint64_t>(remoteBuffer), buffer, size);

        // Build payload
        HijackPayload payload{};
        payload.operation = HIJACK_OP_WRITE;
        payload.target_addr = address;
        payload.buffer_addr = reinterpret_cast<uint64_t>(remoteBuffer);
        payload.size = size;
        payload.done = 0;

        HANDLE hThread = FindHijackThread();
        if (!hThread) {
            VirtualFreeEx(m_targetProcess, remoteBuffer, 0, MEM_RELEASE);
            return false;
        }

        bool ok = ExecuteThreadHijack(hThread, &payload);
        CloseHandle(hThread);
        VirtualFreeEx(m_targetProcess, remoteBuffer, 0, MEM_RELEASE);
        return ok;
    }

    case StealthMode::DirectSyscall:
        return WriteViaSyscall(address, buffer, size);

    default:
        return false;
    }
}

// ======================================================================
// Fragmented read - splits into random-sized chunks with jitter
// ======================================================================
bool ApcStealth::ReadFragmented(uint64_t address, void* buffer, size_t size, size_t chunk_size) {
    if (!m_initialized) return false;
    if (size == 0) return true;

    // Small reads go direct
    if (size <= kDefaultChunkSize || chunk_size == 0)
        return Read(address, buffer, size);

    if (chunk_size == 0) chunk_size = kDefaultChunkSize;

    uint8_t* dst = static_cast<uint8_t*>(buffer);
    size_t remaining = size;
    uint64_t currentAddr = address;

    while (remaining > 0) {
        int variation = RandomRange(-static_cast<int>(chunk_size / 3),
                                     static_cast<int>(chunk_size / 3));
        size_t thisChunk = std::min(remaining,
                                     chunk_size + (variation > 0 ? variation : 0));

        // 20% chance of dummy read to confuse pattern detection
        if (RandomRange(0, 99) < 20 && remaining > chunk_size * 2) {
            uint64_t dummyAddr = currentAddr + RandomRange(-0x200, 0x200);
            uint8_t dummyBuf[8];
            Read(dummyAddr, dummyBuf, RandomRange(1, 8));
        }

        Read(currentAddr, dst, thisChunk);

        if (remaining > thisChunk) Jitter();

        currentAddr += thisChunk;
        dst += thisChunk;
        remaining -= thisChunk;

        if (RandomRange(0, 99) < 15) Jitter();
    }

    return true;
}

// ======================================================================
// Random timing jitter
// ======================================================================
void ApcStealth::Jitter() {
    static thread_local std::mt19937 localRng(
        static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count())
    );
    std::uniform_int_distribution<int> dist(kJitterMinMs, kJitterMaxMs);
    int ms = dist(localRng);
    if (ms > 0) Sleep(ms);
}

// ======================================================================
// Random number generator
// ======================================================================
int ApcStealth::RandomRange(int min, int max) {
    if (min >= max) return min;
    std::uniform_int_distribution<int> dist(min, max);
    return dist(m_rng);
}
