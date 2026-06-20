#include "offsets_manager.h"
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <algorithm>
#include <cctype>

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
// WinHTTP GET helper (with custom header for rbxoffsets.xyz)
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

    // Required auth header for rbxoffsets.xyz API
    LPCWSTR headers = L"rbxoffsets.xyz: apiv1\r\n";
    BOOL ok = WinHttpSendRequest(
        hRequest, headers, (DWORD)wcslen(headers),
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
// fetch_hpp - Download offsets from rbxoffsets.xyz API
// ------------------------------------------------------------------
std::string OffsetsManager::fetch_hpp() {
    // Try latest first, then fallback to version-specific
    try {
        return http_get(HOST, PATH);
    } catch (...) {
        // If latest fails, try version-specific as fallback
        // (PATH already points to /api/latest/raw)
        throw;
    }
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
// parse_hpp - Parse offsets from rbxoffsets.xyz flat format
// ------------------------------------------------------------------
// New format (rbxoffsets.xyz):
//   namespace offsets {
//       inline constexpr uintptr_t FakeDataModelPointer = 0x7bcf6a8;
//       inline constexpr uintptr_t FakeDataModelToDataModel = 0x1d8;
//   }
//
// We store keys as "namespace/field" so existing MAP/TRY macros work.
// For flat names like "FakeDataModelPointer", we split by PascalCase:
// "FakeDataModel" + "Pointer" -> key "fakedatamodel/pointer"
// ------------------------------------------------------------------
static void trim(std::string& s) {
    s.erase(0, s.find_first_not_of(" \t\r\n"));
    s.erase(s.find_last_not_of(" \t\r\n") + 1);
}

// Split a PascalCase name like "FakeDataModelPointer" into namespace + field
// Tries to find the natural split point between words
static bool split_pascal_case(const std::string& name, std::string& cls, std::string& field) {
    if (name.empty()) return false;

    // If name has underscore, split on first underscore
    auto us = name.find('_');
    if (us != std::string::npos && us > 0 && us < name.size() - 1) {
        cls = name.substr(0, us);
        field = name.substr(us + 1);
        return true;
    }

    // Try to split by PascalCase transitions:
    // Scan for uppercase letters after lowercase sequences
    // The split point is where a new "word" starts after the class name ends
    size_t split = 0;
    for (size_t i = 1; i < name.size(); ++i) {
        if (isupper(name[i]) && islower(name[i - 1])) {
            // Transition: lowercase -> uppercase = word boundary
            if (i > 1) {
                split = i;
            }
        }
    }

    // If we found a split point, the class is everything before it
    // and field is from split point onward
    if (split > 0 && split < name.size()) {
        cls = name.substr(0, split);
        field = name.substr(split);
        return true;
    }

    // Can't split — store as general/name
    cls = "general";
    field = name;
    return false;
}

bool OffsetsManager::parse_hpp(const std::string& content) {
    std::istringstream stream(content);
    std::string line;
    std::string currentNamespace;
    int parsed = 0;

    while (std::getline(stream, line)) {
        trim(line);

        if (line.empty()) continue;

        // Detect namespace open: "namespace Name {"
        {
            const std::string nsPrefix = "namespace ";
            if (line.rfind(nsPrefix, 0) == 0) {
                std::string rest = line.substr(nsPrefix.size());
                auto bracePos = rest.find('{');
                if (bracePos != std::string::npos) {
                    currentNamespace = rest.substr(0, bracePos);
                    trim(currentNamespace);
                    continue;
                }
            }
        }

        // Detect namespace close: "}"
        if (line == "}") {
            currentNamespace.clear();
            continue;
        }

        if (currentNamespace.empty()) continue;

        // Detect constexpr line: must contain "constexpr" and "uintptr_t"
        if (line.find("constexpr") == std::string::npos) continue;
        if (line.find("uintptr_t") == std::string::npos) continue;

        // Extract field name: between "uintptr_t" and "="
        auto uintptrPos = line.find("uintptr_t");
        std::string afterType = line.substr(uintptrPos + 9); // skip "uintptr_t"
        trim(afterType);

        auto equalsPos = afterType.find('=');
        if (equalsPos == std::string::npos) continue;

        std::string fieldName = afterType.substr(0, equalsPos);
        trim(fieldName);

        // Extract value: after "=" until ";"
        std::string afterEquals = afterType.substr(equalsPos + 1);
        trim(afterEquals);

        auto semicolonPos = afterEquals.find(';');
        if (semicolonPos == std::string::npos) continue;

        std::string valueStr = afterEquals.substr(0, semicolonPos);
        trim(valueStr);

        // Parse hex value
        uintptr_t val = 0;
        if (valueStr.size() > 2 && valueStr[0] == '0' && (valueStr[1] == 'x' || valueStr[1] == 'X')) {
            val = std::stoull(valueStr, nullptr, 16);
        } else {
            val = std::stoull(valueStr, nullptr, 0);
        }

        // Build cache key: split flat names like "FakeDataModelPointer" -> "fakedatamodel/pointer"
        std::string cls, field;
        bool hasSplit = split_pascal_case(fieldName, cls, field);

        // If the namespace is "offsets" and we have a split, use it
        // Otherwise fallback to namespace/fieldname
        std::string ns_lower = currentNamespace;
        for (auto& c : ns_lower) c = (char)std::tolower((unsigned char)c);

        std::string field_lower = fieldName;
        for (auto& c : field_lower) c = (char)std::tolower((unsigned char)c);

        if (hasSplit && ns_lower == "offsets") {
            std::string cls_lower = cls;
            for (auto& c : cls_lower) c = (char)std::tolower((unsigned char)c);
            std::string fld_lower = field;
            for (auto& c : fld_lower) c = (char)std::tolower((unsigned char)c);

            // Store primary key: "classname/fieldname" (for MAP/TRY)
            std::string key = cls_lower + "/" + fld_lower;
            OffsetsEntry entry;
            entry.value = val;
            std::stringstream ss;
            ss << "0x" << std::hex << val;
            entry.hex = ss.str();
            cache_[key] = entry;

            // Also store flat key: "offsets/name" for direct lookup
            std::string flatKey = ns_lower + "/" + field_lower;
            cache_[flatKey] = entry;

            parsed++;
        } else {
            // Traditional format: namespace/fieldname
            std::string key = ns_lower + "/" + field_lower;
            OffsetsEntry entry;
            entry.value = val;
            std::stringstream ss;
            ss << "0x" << std::hex << val;
            entry.hex = ss.str();
            cache_[key] = entry;
            parsed++;
        }
    }

    total_offsets_ = parsed;
    return parsed > 0;
}

// ------------------------------------------------------------------
// load() - Entry point: download .hpp -> compare with disk -> cache -> parse
// ------------------------------------------------------------------
bool OffsetsManager::load() {
    FILE* dbg = fopen("offsets_debug.txt", "a");

    try {
        if (dbg) fprintf(dbg, "[offsets] step1: fetch_hpp from GitHub...\n");
        console::info("Descargando offsets desde GitHub...");
        std::string hppContent = fetch_hpp();
        if (dbg) fprintf(dbg, "[offsets] downloaded %zu bytes\n", hppContent.size());
        console::info("Descargados %zu bytes del .hpp", hppContent.size());

        // Compare with cached file on disk (by size for fast check)
        std::string cachedContent = read_file(HPP_CACHE_FILE);
        bool needsUpdate = false;

        if (cachedContent.empty()) {
            if (dbg) fprintf(dbg, "[offsets] no cache file on disk\n");
            console::warn("No hay cache local. Guardando...");
            needsUpdate = true;
        } else if (cachedContent.size() != hppContent.size()) {
            if (dbg) fprintf(dbg, "[offsets] size mismatch: cached=%zu vs remote=%zu\n",
                cachedContent.size(), hppContent.size());
            console::warn("Offsets actualizados disponibles (%zu -> %zu bytes)",
                cachedContent.size(), hppContent.size());
            needsUpdate = true;
        } else if (cachedContent != hppContent) {
            if (dbg) fprintf(dbg, "[offsets] content differs (same size)\n");
            console::warn("Offsets diferentes detectados. Actualizando...");
            needsUpdate = true;
        } else {
            if (dbg) fprintf(dbg, "[offsets] cache is up to date\n");
            console::success("Offsets en cache estan actualizados.");
        }

        if (needsUpdate) {
            write_file(HPP_CACHE_FILE, hppContent);
            if (dbg) fprintf(dbg, "[offsets] saved to disk\n");
            console::success("Offsets guardados en disco (%zu bytes)", hppContent.size());
            cachedContent = hppContent; // use the freshly downloaded content
        }

        // Parse the .hpp content
        if (dbg) fprintf(dbg, "[offsets] step2: parse_hpp...\n");
        bool parsed = parse_hpp(cachedContent);
        if (dbg) fprintf(dbg, "[offsets] parse_hpp: %s (%d offsets)\n",
            parsed ? "SUCCESS" : "FAILED", total_offsets_);

        if (parsed) {
            loaded_ = true;
            console::success("Offsets cargados: %d offsets", total_offsets_);
        } else {
            console::error("Error al procesar offsets.hpp");
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
        std::string cachedContent = read_file(HPP_CACHE_FILE);
        if (!cachedContent.empty()) {
            if (dbg) fprintf(dbg, "[offsets] fallback: found %zu bytes\n", cachedContent.size());
            try {
                if (parse_hpp(cachedContent)) {
                    loaded_ = true;
                    if (dbg) fprintf(dbg, "[offsets] fallback: SUCCESS (%d offsets)\n", total_offsets_);
                    console::success("Respaldo local cargado (%d offsets)", total_offsets_);
                    if (dbg) fclose(dbg);
                    return true;
                }
                if (dbg) fprintf(dbg, "[offsets] fallback: parse_hpp failed\n");
            }
            catch (const std::exception& e2) {
                if (dbg) fprintf(dbg, "[offsets] fallback exception: %s\n", e2.what());
            }
        } else {
            if (dbg) fprintf(dbg, "[offsets] fallback: no offsets.hpp on disk\n");
            console::error("No se encontraron offsets locales en el disco");
        }

        if (dbg) fclose(dbg);
        return false;
    }
}

// ------------------------------------------------------------------
// get_offset - Lookup uintptr_t value by class/field name
// ------------------------------------------------------------------
uintptr_t OffsetsManager::get_offset(const std::string& cls, const std::string& field) const {
    std::string cls_lower = cls;
    for (auto& c : cls_lower) c = (char)std::tolower((unsigned char)c);
    std::string fld_lower = field;
    for (auto& c : fld_lower) c = (char)std::tolower((unsigned char)c);
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
    for (auto& c : cls_lower) c = (char)std::tolower((unsigned char)c);
    std::string fld_lower = field;
    for (auto& c : fld_lower) c = (char)std::tolower((unsigned char)c);
    std::string key = cls_lower + "/" + fld_lower;
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        return it->second.hex;
    }
    return "0x0";
}
