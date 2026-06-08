#include "offsets_manager.h"
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>

// WinHTTP
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

// nlohmann/json (single header) - Dependencies is in vcxproj IncludePath
#include "nlohmann_json.hpp"
using json = nlohmann::json;

// ------------------------------------------------------------------
// Singleton
// ------------------------------------------------------------------
OffsetsManager& OffsetsManager::instance() {
    static OffsetsManager mgr;
    return mgr;
}

// ------------------------------------------------------------------
// load() - Entry point: try remote, fallback to local
// ------------------------------------------------------------------
bool OffsetsManager::load() {
    std::string json_str;

    // 1. Try remote fetch
    bool remote_ok = fetch_from_server(json_str);

    // 2. If remote worked, save as local fallback
    if (remote_ok && !json_str.empty()) {
        save_to_file(LOCAL_FILE, json_str);
    }
    else {
        // 3. Fallback: try local file
        printf("[OFFSETS] Server unreachable, trying local fallback...\n");
        remote_ok = load_from_file(LOCAL_FILE, json_str);
    }

    // 4. Parse whatever we got
    if (!json_str.empty() && parse_json(json_str)) {
        loaded_ = true;
        printf("[OFFSETS] Loaded successfully. Version: %s\n",
            client_version_.empty() ? "unknown" : client_version_.c_str());
        return true;
    }

    printf("[OFFSETS] FATAL: Could not load offsets from server or disk.\n");
    return false;
}

// ------------------------------------------------------------------
// fetch_from_server - WinHTTP GET to imtheo.lol
// ------------------------------------------------------------------
bool OffsetsManager::fetch_from_server(std::string& out_json) {
    printf("[OFFSETS] Fetching from %s ...\n", REMOTE_URL);

    // Parse URL
    std::wstring url(L"https://imtheo.lol/Offsets");
    URL_COMPONENTS urlComp = { sizeof(URL_COMPONENTS) };
    urlComp.dwSchemeLength = (DWORD)-1;
    urlComp.dwHostNameLength = (DWORD)-1;
    urlComp.dwUrlPathLength = (DWORD)-1;

    if (!WinHttpCrackUrl(url.c_str(), (DWORD)url.length(), 0, &urlComp)) {
        printf("[OFFSETS] Failed to parse URL\n");
        return false;
    }

    std::wstring hostName(urlComp.lpszHostName, urlComp.dwHostNameLength);
    std::wstring urlPath(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
    if (urlPath.empty()) urlPath = L"/Offsets";

    HINTERNET hSession = nullptr;
    HINTERNET hConnect = nullptr;
    HINTERNET hRequest = nullptr;
    bool success = false;

    do {
        // Open session
        hSession = WinHttpOpen(L"Autopsy/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            nullptr, nullptr, 0);
        if (!hSession) {
            printf("[OFFSETS] WinHttpOpen failed: %lu\n", GetLastError());
            break;
        }

        // Connect
        hConnect = WinHttpConnect(hSession, hostName.c_str(),
            INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (!hConnect) {
            printf("[OFFSETS] WinHttpConnect failed: %lu\n", GetLastError());
            break;
        }

        // Open request
        hRequest = WinHttpOpenRequest(hConnect, L"GET", urlPath.c_str(),
            nullptr, nullptr, nullptr,
            WINHTTP_FLAG_SECURE);
        if (!hRequest) {
            printf("[OFFSETS] WinHttpOpenRequest failed: %lu\n", GetLastError());
            break;
        }

        // Send request
        if (!WinHttpSendRequest(hRequest, nullptr, 0, nullptr, 0, 0, 0)) {
            printf("[OFFSETS] WinHttpSendRequest failed: %lu\n", GetLastError());
            break;
        }

        // Receive response
        if (!WinHttpReceiveResponse(hRequest, nullptr)) {
            printf("[OFFSETS] WinHttpReceiveResponse failed: %lu\n", GetLastError());
            break;
        }

        // Read response
        DWORD statusCode = 0;
        DWORD statusSize = sizeof(statusCode);
        WinHttpQueryHeaders(hRequest,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            nullptr, &statusCode, &statusSize, nullptr);

        if (statusCode != 200) {
            printf("[OFFSETS] Server returned HTTP %lu\n", statusCode);
            break;
        }

        out_json.clear();
        std::vector<char> buffer(8192);
        DWORD bytesRead = 0;

        while (WinHttpReadData(hRequest, buffer.data(),
            (DWORD)buffer.size(), &bytesRead) && bytesRead > 0) {
            out_json.append(buffer.data(), bytesRead);
        }

        if (out_json.empty()) {
            printf("[OFFSETS] Empty response from server\n");
            break;
        }

        printf("[OFFSETS] Received %zu bytes\n", out_json.size());
        success = true;

    } while (false);

    // Cleanup
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    if (!success) {
        printf("[OFFSETS] Fetch failed\n");
    }

    return success;
}

// ------------------------------------------------------------------
// load_from_file - Read local offsets.json
// ------------------------------------------------------------------
bool OffsetsManager::load_from_file(const std::string& path, std::string& out_json) {
    std::ifstream file(path);
    if (!file.is_open()) {
        printf("[OFFSETS] Local file '%s' not found\n", path.c_str());
        return false;
    }

    std::stringstream ss;
    ss << file.rdbuf();
    out_json = ss.str();
    file.close();

    printf("[OFFSETS] Loaded %zu bytes from '%s'\n", out_json.size(), path.c_str());
    return !out_json.empty();
}

// ------------------------------------------------------------------
// save_to_file - Write JSON to disk for future fallback
// ------------------------------------------------------------------
bool OffsetsManager::save_to_file(const std::string& path, const std::string& json) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    file << json;
    file.close();
    return true;
}

// ------------------------------------------------------------------
// parse_json - Parse the RbxDumperV2 JSON format
// ------------------------------------------------------------------
bool OffsetsManager::parse_json(const std::string& json_str) {
    try {
        auto j = json::parse(json_str);

        // ClientVersion (opcional)
        if (j.contains("ClientVersion") && j["ClientVersion"].is_string()) {
            client_version_ = j["ClientVersion"].get<std::string>();
        }

        // Offsets es el objeto principal
        // Formato esperado:
        // { "ClientVersion": "...", "Instance": { "ChildrenStart": 120, ... }, "Camera": { ... }, ... }
        // O: { "ClientVersion": "...", "Offsets": { "Instance": { ... }, ... } }

        json offsets_obj;
        if (j.contains("Offsets") && j["Offsets"].is_object()) {
            offsets_obj = j["Offsets"];
        }
        else {
            // Try the root object (no nested "Offsets" key)
            offsets_obj = j;
        }

        int total_offsets = 0;

        for (auto& [group_name, group_obj] : offsets_obj.items()) {
            if (!group_obj.is_object()) continue;
            if (group_name == "ClientVersion" || group_name == "Offsets") continue;

            for (auto& [offset_name, offset_value] : group_obj.items()) {
                if (!offset_value.is_number()) continue;

                uintptr_t val = static_cast<uintptr_t>(offset_value.get<uint64_t>());

                // Build composite key: "Group/Name"
                std::string key = group_name + "/" + offset_name;

                OffsetsEntry entry;
                entry.value = val;

                // Hex representation for debug
                std::stringstream ss;
                ss << "0x" << std::hex << val;
                entry.hex = ss.str();

                cache_[key] = entry;
                total_offsets++;
            }
        }

        printf("[OFFSETS] Parsed %d offsets across %zu groups\n",
            total_offsets, offsets_obj.size());
        return total_offsets > 0;

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
// get_offset - Lookup uintptr_t value by group/name
// ------------------------------------------------------------------
uintptr_t OffsetsManager::get_offset(const std::string& group, const std::string& name) const {
    std::string key = group + "/" + name;
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        return it->second.value;
    }
    return 0;
}

// ------------------------------------------------------------------
// get_hex_offset - Lookup hex string for debug
// ------------------------------------------------------------------
std::string OffsetsManager::get_hex_offset(const std::string& group, const std::string& name) const {
    std::string key = group + "/" + name;
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        return it->second.hex;
    }
    return "0x0";
}
