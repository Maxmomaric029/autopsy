#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <nlohmann/json.hpp>

#define NOMINMAX
#include <windows.h>

using json = nlohmann::json;

// ------------------------------------------------------------------
// OffsetsManager - Auto-Updater via WinHTTP
// ------------------------------------------------------------------
// Host: offsets.imtheo.lol
//   1. GET /roblox/version   -> plain text: "version-abc123..."
//   2. Compare vs cached offsets.version
//   3. If different: GET /offsets.json -> save as offsets.json
//   4. Load offsets.json into memory
// ------------------------------------------------------------------

struct OffsetsEntry {
    uintptr_t value = 0;
    std::string hex;           // "0xABC" for debug
};

class OffsetsManager {
public:
    static OffsetsManager& instance();

    // Full load: check version, download if stale, parse
    bool load();

    // Helpers (return 0 if not found)
    uintptr_t get_offset(const std::string& cls, const std::string& field) const;
    std::string get_hex_offset(const std::string& cls, const std::string& field) const;

    bool is_loaded() const { return loaded_; }
    std::string roblox_version() const { return roblox_version_; }
    int total_offsets() const { return total_offsets_; }

private:
    OffsetsManager() = default;
    ~OffsetsManager() = default;
    OffsetsManager(const OffsetsManager&) = delete;
    OffsetsManager& operator=(const OffsetsManager&) = delete;

    // WinHTTP GET helper (throws on failure)
    static std::string http_get(const wchar_t* host, const wchar_t* path);

    // Get live version from server
    static std::string fetch_live_version();

    // Download offsets.json from server
    static std::string fetch_offsets_json();

    // File I/O for caching
    static std::string read_file(const std::string& path);
    static void write_file(const std::string& path, const std::string& content);

    // Parse JSON into cache
    bool parse_json(const json& data);

    std::unordered_map<std::string, OffsetsEntry> cache_;
    std::string roblox_version_;
    int total_offsets_ = 0;
    bool loaded_ = false;

    static constexpr const wchar_t* HOST = L"offsets.imtheo.lol";
    static constexpr const char* VERSION_FILE = "offsets.version";
    static constexpr const char* OFFSETS_FILE = "offsets.json";
};

// Helper: safe offset lookup from json data
inline uintptr_t GetOffset(const json& data, const char* cls, const char* field) {
    if (!data.contains("Offsets")) return 0;
    const auto& offsets = data["Offsets"];
    if (!offsets.contains(cls)) return 0;
    const auto& entry = offsets[cls];
    if (!entry.contains(field)) return 0;
    return entry[field].get<uintptr_t>();
}
