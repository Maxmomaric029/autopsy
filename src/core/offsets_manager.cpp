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

    // Set timeouts: resolve=5s, connect=5s, send=5s, receive=5s
    WinHttpSetTimeouts(hRequest, 5000, 5000, 5000, 5000);

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
    std::string version = http_get(HOST, L"/roblox/version");
    // Trim whitespace/newlines
    while (!version.empty() && (version.back() == '\n' || version.back() == '\r' || version.back() == ' '))
        version.pop_back();
    return version;
}

// ------------------------------------------------------------------
// fetch_offsets_json - GET /offsets.json, returns JSON string
// ------------------------------------------------------------------
std::string OffsetsManager::fetch_offsets_json() {
    std::string body = http_get(HOST, L"/offsets.json");
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

#include "log.h"

// ------------------------------------------------------------------
// load() - Entry point: version check -> download if stale -> parse
// Follows the documented EnsureLatestOffsets() pattern.
// ------------------------------------------------------------------
bool OffsetsManager::load() {
    // Debug file (append)
    FILE* dbg = fopen("offsets_debug.txt", "a");

    try {
        if (dbg) fprintf(dbg, "[offsets] step1: fetch_live_version...\n");
        console::info("Conectando al servidor para buscar version...");
        // 1. Get live version
        std::string liveVersion = fetch_live_version();
        if (dbg) fprintf(dbg, "[offsets] live version: '%s'\n", liveVersion.c_str());
        console::info("Version en servidor: '%s'", liveVersion.c_str());

        // 2. Read cached version from disk
        std::string cachedVersion = read_file(VERSION_FILE);
        // Trim whitespace
        while (!cachedVersion.empty() && (cachedVersion.back() == '\n' || cachedVersion.back() == '\r' || cachedVersion.back() == ' '))
            cachedVersion.pop_back();
        if (dbg) fprintf(dbg, "[offsets] cached version: '%s'\n", cachedVersion.empty() ? "(none)" : cachedVersion.c_str());
        console::info("Version en cache local: '%s'", cachedVersion.empty() ? "(ninguna)" : cachedVersion.c_str());

        // 3. Check if offsets.json exists on disk
        char fullPath[MAX_PATH] = {};
        GetFullPathNameA(OFFSETS_FILE, sizeof(fullPath), fullPath, nullptr);
        bool offsetsExist = std::filesystem::exists(OFFSETS_FILE);
        if (dbg) fprintf(dbg, "[offsets] offsets.json path: %s\n", fullPath);
        
        // 4. Download only if version changed or no cache
        if (liveVersion != cachedVersion || !offsetsExist) {
            if (dbg) fprintf(dbg, "[offsets] step2: fetch_offsets_json...\n");
            console::warn("Cache expirado o ausente. Descargando nuevos offsets...");
            std::string jsonBody = fetch_offsets_json();
            if (dbg) fprintf(dbg, "[offsets] downloaded %zu bytes\n", jsonBody.size());
            write_file(OFFSETS_FILE, jsonBody);
            write_file(VERSION_FILE, liveVersion);
            if (dbg) fprintf(dbg, "[offsets] saved to disk\n");
            console::success("Offsets guardados con exito (%zu bytes)", jsonBody.size());
        } else {
            console::success("Los offsets estan actualizados.");
        }

        // 5. Load and parse the JSON
        if (dbg) fprintf(dbg, "[offsets] step3: read offsets.json...\n");
        std::string jsonStr = read_file(OFFSETS_FILE);
        if (jsonStr.empty()) {
            if (dbg) fprintf(dbg, "[offsets] FATAL: offsets.json is empty\n");
            console::error("Error: offsets.json esta vacio");
            if (dbg) fclose(dbg);
            return false;
        }

        if (dbg) fprintf(dbg, "[offsets] step4: json::parse...\n");
        auto data = json::parse(jsonStr);
        if (dbg) fprintf(dbg, "[offsets] step5: parse_json...\n");
        bool parsed = parse_json(data);
        if (dbg) fprintf(dbg, "[offsets] parse_json: %s\n", parsed ? "SUCCESS" : "FAILED");

        if (parsed) {
            loaded_ = true;
            if (dbg) fprintf(dbg, "[offsets] loaded %d offsets, version=%s\n", total_offsets_, roblox_version_.c_str());
            console::success("Offsets cargados con exito. Version=%s (%d offsets)", roblox_version_.c_str(), total_offsets_);
        } else {
            console::error("Error al procesar JSON de offsets");
        }

        if (dbg) fclose(dbg);
        return loaded_;

    }
    catch (const std::exception& e) {
        if (!dbg) dbg = fopen("offsets_debug.txt", "a");
        if (dbg) fprintf(dbg, "[offsets] EXCEPTION: %s\n", e.what());
        console::error("Excepcion: %s", e.what());
        // Fallback: try loading whatever is on disk
        if (dbg) fprintf(dbg, "[offsets] fallback: trying disk cache...\n");
        console::warn("Intentando cargar offsets locales de respaldo...");
        std::string jsonStr = read_file(OFFSETS_FILE);
        if (!jsonStr.empty()) {
            if (dbg) fprintf(dbg, "[offsets] fallback: found %zu bytes\n", jsonStr.size());
            try {
                auto data = json::parse(jsonStr);
                if (parse_json(data)) {
                    loaded_ = true;
                    if (dbg) fprintf(dbg, "[offsets] fallback: SUCCESS (%d offsets)\n", total_offsets_);
                    console::success("Respaldo local cargado (%d offsets)", total_offsets_);
                    if (dbg) fclose(dbg);
                    return true;
                }
                if (dbg) fprintf(dbg, "[offsets] fallback: parse_json failed\n");
            }
            catch (const std::exception& e2) {
                if (dbg) fprintf(dbg, "[offsets] fallback exception: %s\n", e2.what());
            }
        } else {
            if (dbg) fprintf(dbg, "[offsets] fallback: no offsets.json on disk\n");
            console::error("No se encontraron offsets locales en el disco");
        }

        if (dbg) fclose(dbg);
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

                // Lowercase field name too for case-insensitive matching
                std::string field_lower = offset_name;
                for (auto& c : field_lower) c = (char)tolower(c);

                std::string key = group_lower + "/" + field_lower;
                OffsetsEntry entry;
                entry.value = val;

                std::stringstream ss;
                ss << "0x" << std::hex << val;
                entry.hex = ss.str();

                cache_[key] = entry;
                parsed++;
            }
        }

        return parsed > 0;

    }
    catch (const json::parse_error&) {
        return false;
    }
    catch (const json::type_error&) {
        return false;
    }
    catch (const std::exception&) {
        return false;
    }
}

// ------------------------------------------------------------------
// get_offset - Lookup uintptr_t value by class/field name
// ------------------------------------------------------------------
uintptr_t OffsetsManager::get_offset(const std::string& cls, const std::string& field) const {
    std::string cls_lower = cls;
    for (auto& c : cls_lower) c = (char)tolower(c);
    std::string fld_lower = field;
    for (auto& c : fld_lower) c = (char)tolower(c);
    std::string key = cls_lower + "/" + fld_lower;
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
    std::string fld_lower = field;
    for (auto& c : fld_lower) c = (char)tolower(c);
    std::string key = cls_lower + "/" + fld_lower;
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        return it->second.hex;
    }
    return "0x0";
}
