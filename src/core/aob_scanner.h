#pragma once
#include <cstdint>
#include <string>
#include <vector>

// AOB scan result
struct AOBResult {
    uint64_t pointer_addr = 0; // Absolute address of the global that holds the pointer
    bool found = false;
};

// AOB scanner — parses PE from disk, scans .text section for patterns,
// extracts RIP-relative addresses to find global pointer variables.
class AOBScanner {
public:
    // Scan for FakeDataModel global pointer in RobloxPlayerBeta.exe
    // Pattern: "48 8B 0D ?? ?? ?? ?? 48 85 C9 74 ?? 48 8B 49 ??"
    //   mov rcx, [rip+offset]    ; load FakeDataModel from global
    //   test rcx, rcx
    //   je short ...
    //   mov rcx, [rcx+field]     ; dereference RealDataModel
    static AOBResult find_fakemodel(uint64_t module_base, const std::string& exe_path);

    // Scan for VisualEngine global pointer
    // Pattern: "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 ?? 48 8B 40 ??"
    static AOBResult find_visualengine(uint64_t module_base, const std::string& exe_path);

    // Scan for TaskScheduler global pointer
    // Pattern: "48 8B 0D ?? ?? ?? ?? 48 85 C0 74 ?? 48 8B 40 ??"
    static AOBResult find_taskscheduler(uint64_t module_base, const std::string& exe_path);

private:
    struct SectionInfo {
        uint32_t raw_offset = 0;
        uint32_t raw_size = 0;
        uint32_t virtual_addr = 0; // RVA
        uint32_t virtual_size = 0;
    };

    // Parse PE file and find .text section info
    static bool find_text_section(const std::string& exe_path, SectionInfo& out);

    // Search for hex pattern in data buffer
    // Pattern format: "48 8B 0D ?? ?? ?? ?? 48 85 C9" (spaces, ?? = wildcard)
    static bool search_pattern(const uint8_t* data, size_t data_size,
                               const std::string& pattern_hex, size_t& match_offset);

    // Convert hex pattern string to bytes with wildcard markers
    // Returns vector of (byte, is_wildcard) pairs
    static std::vector<std::pair<uint8_t, bool>> parse_pattern(const std::string& pattern_hex);

    // Extract RIP-relative address from a matched pattern
    // pattern offset 3 = start of 32-bit relative offset (for 7-byte instructions)
    static uint64_t extract_rip_addr(uint64_t module_base, const SectionInfo& text,
                                     size_t match_file_offset, const uint8_t* data);
};
