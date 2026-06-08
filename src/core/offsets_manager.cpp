#include "offsets_manager.h"
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdexcept>

#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

// ------------------------------------------------------------------
// Singleton
// ------------------------------------------------------------------
OffsetsManager& OffsetsManager::instance() {
    static OffsetsManager mgr;
    return mgr;
}

// ------------------------------------------------------------------
// WinHTTP GET helper - matches the documented implementation
// ------------------------------------------------------------------
std::string OffsetsManager::http_get(const wchar_t* host, const wchar_t* path) {
    HINTERNET hSession = WinHttpOpen(
        L"OffsetsClient/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);
    if (!hSession) throw std::runtime_error("WinHttpOpen failed");

    HINTERNET hConnect = WinHttpConnect(hSession, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        throw std::runtime_error("WinHttpConnect failed");
    }

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect, L"GET", path,
        nullptr, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        throw std::runtime_error("WinHttpOpenRequest failed");
    }

    BOOL ok = WinHttpSendRequest(
        hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (!ok || !WinHttpReceiveResponse(hRequest, nullptr)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        throw std::runtime_error("Request failed");
    }

    // Check status code
    DWORD status = 0;
    DWORD statusSize = sizeof(status);
    WinHttpQueryHeaders(
        hRequest,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize, WINHTTP_NO_HEADER_INDEX);

    if (status != 200) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        throw std::runtime_error("HTTP " + std::to_string(status));
    }

    // Read body
    std::string body;
    DWORD available = 0;
    while (WinHttpQueryDataAvailable(hRequest, &available) && available > 0) {
        std::vector<char> buf(available);
        DWORD read = 0;
        if (!WinHttpReadData(hRequest, buf.data(), available, &read)) break;
        body.append(buf.data(), read);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return body;
}

// ------------------------------------------------------------------
// fetch_live_version - GET /roblox/version, returns plain text
// ------------------------------------------------------------------
std::string OffsetsManager::fetch_live_version() {
    printf("[OFFSETS] Fetching live version from %ls/roblox/version ...\n", HOST);
    std::string version = http_get(HOST, L"/roblox/version");
    // Trim whitespace/newlines
    while (!version.empty() && (version.back() == '\n' || version.back() == '\r' || version.back() == ' '))
        version.pop_back();
    printf("[OFFSETS] Live version: %s\n", version.c_str());
    return version;
}

// ------------------------------------------------------------------
// fetch_offsets_json - GET /offsets.json, returns JSON string
// ------------------------------------------------------------------
std::string OffsetsManager::fetch_offsets_json() {
    printf("[OFFSETS] Downloading offsets.json from %ls/offsets.json ...\n", HOST);
    std::string body = http_get(HOST, L"/offsets.json");
    printf("[OFFSETS] Downloaded %zu bytes\n", body.size());
    return body;
}

// ------------------------------------------------------------------
// File I/O helpers
// ------------------------------------------------------------------
std::string OffsetsManager::read_file(const std::string& path) {
    std::ifstream f(path);
    if (!f) return {};
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

void OffsetsManager::write_file(const std::string& path, const std::string& content) {
    std::ofstream f(path, std::ios::trunc | std::ios::binary);
    f << content;
}

// ------------------------------------------------------------------
// load() - Entry point: version check -> download if stale -> parse
// Follows the documented EnsureLatestOffsets() pattern.
// ------------------------------------------------------------------
bool OffsetsManager::load() {
    try {
        // 1. Get live version
        std::string liveVersion = fetch_live_version();

        // 2. Read cached version from disk
        std::string cachedVersion = read_file(VERSION_FILE);
        // Trim whitespace
        while (!cachedVersion.empty() && (cachedVersion.back() == '\n' || cachedVersion.back() == '\r' || cachedVersion.back() == ' '))
            cachedVersion.pop_back();

        // 3. Check if offsets.json exists on disk
        bool offsetsExist = std::filesystem::exists(OFFSETS_FILE);

        // 4. Download only if version changed or no cache
        if (liveVersion != cachedVersion || !offsetsExist) {
            printf("[OFFSETS] Version changed or cache missing. Downloading new offsets...\n");
            std::string jsonBody = fetch_offsets_json();
            write_file(OFFSETS_FILE, jsonBody);
            write_file(VERSION_FILE, liveVersion);
            printf("[OFFSETS] Saved to %s and %s\n", OFFSETS_FILE, VERSION_FILE);
        }
        else {
            printf("[OFFSETS] Version unchanged (%s), using cached %s\n",
                liveVersion.c_str(), OFFSETS_FILE);
        }

        // 5. Load and parse the JSON
        std::string jsonStr = read_file(OFFSETS_FILE);
        if (jsonStr.empty()) {
            printf("[OFFSETS] FATAL: %s is empty or missing\n", OFFSETS_FILE);
            return false;
        }

        auto data = json::parse(jsonStr);
        bool parsed = parse_json(data);

        if (parsed) {
            loaded_ = true;
            printf("[OFFSETS] Loaded successfully. Roblox version: %s, Total offsets: %d\n",
                roblox_version_.c_str(), total_offsets_);
        }
        else {
            printf("[OFFSETS] FATAL: Failed to parse offsets from %s\n", OFFSETS_FILE);
        }

        return loaded_;

    }
    catch (const std::exception& e) {
        printf("[OFFSETS] ERROR: %s\n", e.what());

        // Fallback: try loading whatever is on disk
        printf("[OFFSETS] Trying fallback: load existing %s from disk...\n", OFFSETS_FILE);
        std::string jsonStr = read_file(OFFSETS_FILE);
        if (!jsonStr.empty()) {
            try {
                auto data = json::parse(jsonStr);
                if (parse_json(data)) {
                    loaded_ = true;
                    printf("[OFFSETS] Fallback loaded from disk (%d offsets).\n", total_offsets_);
                    return true;
                }
            }
            catch (...) {
                printf("[OFFSETS] Fallback also failed.\n");
            }
        }

        return false;
    }
}

// ------------------------------------------------------------------
// parse_json - Parse the documented offsets.json structure
// ------------------------------------------------------------------
bool OffsetsManager::parse_json(const json& data) {
    try {
        // Roblox Version
        if (data.contains("Roblox Version") && data["Roblox Version"].is_string()) {
            roblox_version_ = data["Roblox Version"].get<std::string>();
        }
        // Also check "ClientVersion" as fallback
        else if (data.contains("ClientVersion") && data["ClientVersion"].is_string()) {
            roblox_version_ = data["ClientVersion"].get<std::string>();
        }

        // Total Offsets
        if (data.contains("Total Offsets") && data["Total Offsets"].is_number()) {
            total_offsets_ = data["Total Offsets"].get<int>();
        }

        // The actual offsets are under "Offsets" key
        if (!data.contains("Offsets") || !data["Offsets"].is_object()) {
            printf("[OFFSETS] JSON missing 'Offsets' key\n");
            return false;
        }

        const auto& offsets_obj = data["Offsets"];
        int parsed = 0;

        for (auto& [group_name, group_obj] : offsets_obj.items()) {
            if (!group_obj.is_object()) continue;

            // Normalize group name to lowercase for case-insensitive matching
            std::string group_lower = group_name;
            for (auto& c : group_lower) c = (char)tolower(c);

            for (auto& [offset_name, offset_value] : group_obj.items()) {
                if (!offset_value.is_number()) continue;

                uintptr_t val = static_cast<uintptr_t>(offset_value.get<uint64_t>());

                std::string key = group_lower + "/" + offset_name;
                OffsetsEntry entry;
                entry.value = val;

                std::stringstream ss;
                ss << "0x" << std::hex << val;
                entry.hex = ss.str();

                cache_[key] = entry;
                parsed++;
            }
        }

        printf("[OFFSETS] Parsed %d offsets across %zu groups\n",
            parsed, offsets_obj.size());
        return parsed > 0;

    }
    catch (const json::parse_error& e) {
        printf("[OFFSETS] JSON parse error: %s\n", e.what());
        return false;
    }
    catch (const json::type_error& e) {
        printf("[OFFSETS] JSON type error: %s\n", e.what());
        return false;
    }
    catch (const std::exception& e) {
        printf("[OFFSETS] Unexpected error: %s\n", e.what());
        return false;
    }
}

// ------------------------------------------------------------------
// get_offset - Lookup uintptr_t value by class/field name
// ------------------------------------------------------------------
uintptr_t OffsetsManager::get_offset(const std::string& cls, const std::string& field) const {
    std::string cls_lower = cls;
    for (auto& c : cls_lower) c = (char)tolower(c);
    std::string key = cls_lower + "/" + field;
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        return it->second.value;
    }
    return 0;
}

// ------------------------------------------------------------------
// get_hex_offset - Lookup hex string for debug
// ------------------------------------------------------------------
std::string OffsetsManager::get_hex_offset(const std::string& cls, const std::string& field) const {
    std::string cls_lower = cls;
    for (auto& c : cls_lower) c = (char)tolower(c);
    std::string key = cls_lower + "/" + field;
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        return it->second.hex;
    }
    return "0x0";
}
