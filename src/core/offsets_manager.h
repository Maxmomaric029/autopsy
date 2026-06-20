#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <filesystem>

#define NOMINMAX
#include <windows.h>

// ------------------------------------------------------------------
// OffsetsManager - Auto-Updater via rbxoffsets.xyz API
// ------------------------------------------------------------------
// Source: https://rbxoffsets.xyz/api/latest/raw
//   Header: rbxoffsets.xyz: apiv1
//
//   1. GET /api/latest/raw -> flat C++ header with inline constexpr offsets
//   2. Compare with cached offsets.hpp on disk (by size)
//   3. If different: save as offsets.hpp
//   4. Parse flat names (PascalCase) into class/field keys for MAP/TRY macros
// ------------------------------------------------------------------

struct OffsetsEntry {
    uintptr_t value = 0;
    std::string hex;           // "0xABC" for debug
};

class OffsetsManager {
public:
    static OffsetsManager& instance();

    // Full load: download .hpp, compare, cache, parse
    bool load();

    // Helpers (return 0 if not found)
    uintptr_t get_offset(const std::string& cls, const std::string& field) const;
    std::string get_hex_offset(const std::string& cls, const std::string& field) const;

    bool is_loaded() const { return loaded_; }
    int total_offsets() const { return total_offsets_; }

private:
    OffsetsManager() = default;
    ~OffsetsManager() = default;
    OffsetsManager(const OffsetsManager&) = delete;
    OffsetsManager& operator=(const OffsetsManager&) = delete;

    // WinHTTP GET helper (throws on failure)
    static std::string http_get(const wchar_t* host, const wchar_t* path);

    // Download the .hpp file from GitHub
    static std::string fetch_hpp();

    // File I/O for caching
    static std::string read_file(const std::string& path);
    static void write_file(const std::string& path, const std::string& content);

    // Parse .hpp content into cache_
    bool parse_hpp(const std::string& content);

    std::unordered_map<std::string, OffsetsEntry> cache_;
    int total_offsets_ = 0;
    bool loaded_ = false;

    static constexpr const wchar_t* HOST = L"rbxoffsets.xyz";
    static constexpr const wchar_t* PATH = L"/api/latest/raw";
    static constexpr const char* HPP_CACHE_FILE = "offsets.hpp";
};
