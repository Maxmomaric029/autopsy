#pragma once
#include <d3d11.h>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <atomic>
#include <cstring>

#include <WinHTTP.h>
#pragma comment(lib, "winhttp.lib")

#include "texture.h"

// ========================================================================
// Roblox Avatar downloader
//
// Flow:
//   1. Get Windows username from environment
//   2. POST https://users.roblox.com/v1/usernames/users → get userId
//   3. GET  https://thumbnails.roblox.com/v1/users/avatar-headshot?userIds=X → get imageUrl
//   4. Download PNG from imageUrl
//   5. Decode with stb_image → create D3D11 SRV
//
// Falls back to initial letters if any step fails.
// Init is async on a background thread.
// ========================================================================

namespace avatar {

    // ---- State (g_srv is atomic for thread-safe read from main thread) ----
    static std::atomic<ID3D11ShaderResourceView*> g_srv{ nullptr };
    static int g_w = 0;
    static int g_h = 0;
    static std::string g_username;
    static std::atomic<bool> g_loading{ false };
    static std::atomic<bool> g_loaded{ false };
    static bool g_init_called = false;

    // ====================================================================
    // Get the Windows username (cached)
    // ====================================================================
    inline std::string get_username() {
        if (!g_username.empty()) return g_username;
        char buf[128] = {};
        DWORD len = GetEnvironmentVariableA("USERNAME", buf, (DWORD)sizeof(buf) - 1);
        if (len > 0 && len < sizeof(buf)) {
            g_username = buf;
        } else {
            g_username = "user";
        }
        return g_username;
    }

    // ====================================================================
    // Simple WinHTTP helpers
    // ====================================================================
    static std::vector<unsigned char> http_get(const wchar_t* host, const wchar_t* path,
        int* outStatusCode = nullptr) {

        std::vector<unsigned char> result;
        if (outStatusCode) *outStatusCode = 0;

        HINTERNET hSession = WinHttpOpen(L"autopsy-agent/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0);
        if (!hSession) return result;

        HINTERNET hConnect = WinHttpConnect(hSession, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (!hConnect) { WinHttpCloseHandle(hSession); return result; }

        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path,
            nullptr, nullptr, nullptr, WINHTTP_FLAG_SECURE);
        if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return result; }

        // Timeouts
        DWORD t = 8000;
        WinHttpSetOption(hRequest, WINHTTP_OPTION_CONNECT_TIMEOUT, &t, sizeof(t));
        WinHttpSetOption(hRequest, WINHTTP_OPTION_SEND_TIMEOUT, &t, sizeof(t));
        WinHttpSetOption(hRequest, WINHTTP_OPTION_RECEIVE_TIMEOUT, &t, sizeof(t));

        LPCWSTR headers = L"Accept: application/json\r\n";
        if (!WinHttpSendRequest(hRequest, headers, -1, nullptr, 0, 0, 0)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return result;
        }
        if (!WinHttpReceiveResponse(hRequest, nullptr)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return result;
        }

        DWORD sc = 0, scs = sizeof(sc);
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            nullptr, &sc, &scs, nullptr);
        if (outStatusCode) *outStatusCode = (int)sc;

        if (sc != 200) {
            // Still read the body for completeness
        }

        DWORD bytesRead = 0;
        unsigned char buf[8192];
        while (WinHttpReadData(hRequest, buf, sizeof(buf), &bytesRead) && bytesRead > 0)
            result.insert(result.end(), buf, buf + bytesRead);

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    static std::vector<unsigned char> http_post(const wchar_t* host, const wchar_t* path,
        const char* postData, int* outStatusCode = nullptr) {

        std::vector<unsigned char> result;
        if (outStatusCode) *outStatusCode = 0;
        if (!postData) return result;

        HINTERNET hSession = WinHttpOpen(L"autopsy-agent/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0);
        if (!hSession) return result;

        HINTERNET hConnect = WinHttpConnect(hSession, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (!hConnect) { WinHttpCloseHandle(hSession); return result; }

        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path,
            nullptr, nullptr, nullptr, WINHTTP_FLAG_SECURE);
        if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return result; }

        DWORD t = 8000;
        WinHttpSetOption(hRequest, WINHTTP_OPTION_CONNECT_TIMEOUT, &t, sizeof(t));
        WinHttpSetOption(hRequest, WINHTTP_OPTION_SEND_TIMEOUT, &t, sizeof(t));
        WinHttpSetOption(hRequest, WINHTTP_OPTION_RECEIVE_TIMEOUT, &t, sizeof(t));

        LPCWSTR headers = L"Content-Type: application/json\r\nAccept: application/json\r\n";
        int dataLen = (int)strlen(postData);

        if (!WinHttpSendRequest(hRequest, headers, -1,
            (LPVOID)postData, dataLen, dataLen, 0)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return result;
        }
        if (!WinHttpReceiveResponse(hRequest, nullptr)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return result;
        }

        DWORD sc = 0, scs = sizeof(sc);
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            nullptr, &sc, &scs, nullptr);
        if (outStatusCode) *outStatusCode = (int)sc;

        DWORD bytesRead = 0;
        unsigned char buf[8192];
        while (WinHttpReadData(hRequest, buf, sizeof(buf), &bytesRead) && bytesRead > 0)
            result.insert(result.end(), buf, buf + bytesRead);

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    // ====================================================================
    // Minimal JSON value extractors (no library dependency)
    // ====================================================================
    // Extracts the value of a string key from a JSON response.
    // Handles: {"key":"value"} and nested objects.
    // Returns empty string if not found.
    inline std::string extract_json_string(const std::vector<unsigned char>& json,
        const char* key) {

        std::string text((const char*)json.data(), json.size());
        std::string search = "\"" + std::string(key) + "\":\"";
        auto pos = text.find(search);
        if (pos == std::string::npos) return "";

        pos += search.size();
        std::string result;
        while (pos < text.size()) {
            if (text[pos] == '\\') {
                // Escaped character
                if (pos + 1 < text.size()) {
                    result += text[pos + 1];
                    pos += 2;
                } else { break; }
            } else if (text[pos] == '"') {
                break;
            } else {
                result += text[pos];
                pos++;
            }
        }
        return result;
    }

    inline long extract_json_int(const std::vector<unsigned char>& json,
        const char* key) {

        std::string text((const char*)json.data(), json.size());
        std::string search = "\"" + std::string(key) + "\":";
        auto pos = text.find(search);
        if (pos == std::string::npos) return 0;

        pos += search.size();
        // Skip whitespace
        while (pos < text.size() && (text[pos] == ' ' || text[pos] == '\t'))
            pos++;

        // Read number
        long val = 0;
        bool neg = false;
        if (pos < text.size() && text[pos] == '-') { neg = true; pos++; }
        while (pos < text.size() && text[pos] >= '0' && text[pos] <= '9') {
            val = val * 10 + (text[pos] - '0');
            pos++;
        }
        return neg ? -val : val;
    }

    // ====================================================================
    // Full avatar download pipeline
    // ====================================================================
    inline bool download_avatar(ID3D11Device* device,
        const std::string& username) {

        // Step 1: Resolve username → userId
        std::string postBody = "{\"usernames\":[\"" + username + "\"],\"excludeBannedUsers\":true}";
        int statusCode = 0;
        auto userResp = http_post(L"users.roblox.com", L"/v1/usernames/users",
            postBody.c_str(), &statusCode);

        if (statusCode != 200 || userResp.empty()) {
            OutputDebugStringA("[avatar] users.roblox.com failed\n");
            return false;
        }

        long userId = extract_json_int(userResp, "id");
        if (userId <= 0) {
            OutputDebugStringA("[avatar] userId not found in response\n");
            return false;
        }

        char userIdStr[32];
        sprintf_s(userIdStr, "%ld", userId);

        // Step 2: Get avatar headshot image URL
        std::string thumbPath = "/v1/users/avatar-headshot?userIds="
            + std::string(userIdStr)
            + "&size=48x48&format=Png&isCircular=false";

        auto thumbResp = http_get(L"thumbnails.roblox.com",
            std::wstring(thumbPath.begin(), thumbPath.end()).c_str(),
            &statusCode);

        if (statusCode != 200 || thumbResp.empty()) {
            OutputDebugStringA("[avatar] thumbnails.roblox.com failed\n");
            return false;
        }

        std::string imageUrl = extract_json_string(thumbResp, "imageUrl");
        if (imageUrl.empty()) {
            OutputDebugStringA("[avatar] imageUrl not found\n");
            return false;
        }

        // Step 3: Download the actual PNG image
        // Parse host and path from imageUrl
        // imageUrl format: https://tr.rbxcdn.com/HASH
        std::string imageHost, imagePath;
        size_t protoEnd = imageUrl.find("://");
        if (protoEnd == std::string::npos) return false;
        size_t hostStart = protoEnd + 3;
        size_t pathStart = imageUrl.find('/', hostStart);
        if (pathStart == std::string::npos) return false;

        imageHost = imageUrl.substr(0, pathStart);
        // For the host, we need just "tr.rbxcdn.com"
        size_t hostOnlyStart = imageUrl.find("://") + 3;
        imageHost = imageUrl.substr(hostOnlyStart, pathStart - hostOnlyStart);
        imagePath = imageUrl.substr(pathStart);

        auto imgResp = http_get(
            std::wstring(imageHost.begin(), imageHost.end()).c_str(),
            std::wstring(imagePath.begin(), imagePath.end()).c_str(),
            &statusCode);

        if (statusCode != 200 || imgResp.empty()) {
            OutputDebugStringA("[avatar] image download failed\n");
            return false;
        }

        // Step 4: Decode with stb_image and create SRV
        int width = 0, height = 0, channels = 0;
        unsigned char* imgData = stbi_load_from_memory(
            imgResp.data(), (int)imgResp.size(),
            &width, &height, &channels, 4);

        if (!imgData || width <= 0 || height <= 0) {
            OutputDebugStringA("[avatar] stb_image decode failed\n");
            if (imgData) stbi_image_free(imgData);
            return false;
        }

        ID3D11ShaderResourceView* srv = nullptr;
        bool ok = tex::create_srv(device, imgData, width, height, &srv);
        stbi_image_free(imgData);

        if (ok && srv) {
            // Release any existing SRV
            ID3D11ShaderResourceView* old = g_srv.load();
            if (old) old->Release();
            g_srv.store(srv);
            g_w = width;
            g_h = height;
            OutputDebugStringA("[avatar] loaded successfully\n");
            return true;
        }

        OutputDebugStringA("[avatar] D3D11 SRV creation failed\n");
        return false;
    }

    // ====================================================================
    // Public API
    // ====================================================================

    // Init: downloads avatar on a background thread
    inline void init(ID3D11Device* device) {
        if (g_init_called) return;
        g_init_called = true;

        std::string username = get_username();
        if (username.empty() || username == "user") {
            g_loaded = true;
            return;
        }

        g_loading = true;

        // Copy device pointer for background thread
        // We need to AddRef the device to keep it alive
        ID3D11Device* dev = device;
        if (dev) dev->AddRef();

        std::thread([dev, username]() {
            bool success = false;
            if (dev) {
                success = download_avatar(dev, username);
                dev->Release();
            }
            g_loading = false;
            g_loaded = true;
            if (!success) {
                OutputDebugStringA("[avatar] download complete (fallback to initials)\n");
            }
        }).detach();
    }

    // Get the SRV (nullptr if still loading or failed)
    inline ID3D11ShaderResourceView* get_srv() { return g_srv; }
    inline int get_width() { return g_w; }
    inline int get_height() { return g_h; }
    inline bool is_loaded() { return g_loaded; }
    inline bool is_loading() { return g_loading; }

    // Shutdown: releases the SRV
    inline void shutdown() {
        if (g_srv) {
            g_srv->Release();
            g_srv = nullptr;
        }
        g_w = g_h = 0;
        g_loading = false;
        g_loaded = false;
        g_init_called = false;
    }

} // namespace avatar
