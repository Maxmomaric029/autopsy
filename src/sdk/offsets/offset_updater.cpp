#include "offset_updater.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <fstream>
#include <filesystem>
#include <vector>
#include <sstream>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

#include "nlohmann_json.hpp"

namespace Offsets
{
    nlohmann::json data;
    std::string Version;

    namespace internal
    {
        std::string exe_dir()
        {
            char path[MAX_PATH];
            GetModuleFileNameA(nullptr, path, MAX_PATH);
            std::filesystem::path p(path);
            return p.parent_path().string();
        }

        bool http_get(const std::wstring& host, const std::wstring& path, std::string& out)
        {
            HINTERNET hSession = WinHttpOpen(L"Autopsy/1.0",
                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0);
            if (!hSession)
                return false;

            HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(),
                INTERNET_DEFAULT_HTTPS_PORT, 0);
            if (!hConnect)
            {
                WinHttpCloseHandle(hSession);
                return false;
            }

            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
                nullptr, nullptr, nullptr,
                WINHTTP_FLAG_SECURE);
            if (!hRequest)
            {
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return false;
            }

            // Set timeout
            DWORD timeout = 10000;
            WinHttpSetOption(hRequest, WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

            bool result = false;
            if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
            {
                if (WinHttpReceiveResponse(hRequest, nullptr))
                {
                    DWORD status = 0;
                    DWORD statusSize = sizeof(status);
                    if (WinHttpQueryHeaders(hRequest,
                        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        nullptr, &status, &statusSize, nullptr) &&
                        status == 200)
                    {
                        DWORD bytesAvailable = 0;
                        std::stringstream ss;
                        char buffer[4096];

                        while (WinHttpQueryDataAvailable(hRequest, &bytesAvailable) && bytesAvailable > 0)
                        {
                            DWORD bytesRead = 0;                    DWORD chunkSize = bytesAvailable < (DWORD)sizeof(buffer) ? bytesAvailable : (DWORD)sizeof(buffer);
                    if (WinHttpReadData(hRequest, buffer, chunkSize, &bytesRead))
                    {
                                ss.write(buffer, bytesRead);
                    }
                        }

                        out = ss.str();
                        result = true;
                    }
                }
            }

            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return result;
        }
    }

    bool Load()
    {
        const std::string dir = internal::exe_dir();
        const std::string jsonPath = dir + "\\offsets.json";
        const std::string verPath = dir + "\\offsets.version";
        std::string serverVersion;
        bool needsUpdate = false;

        // Fetch version from server
        std::string versionResponse;
        if (internal::http_get(L"offsets.imtheo.lol", L"/roblox/version", versionResponse))
        {
            // Trim whitespace
            while (!versionResponse.empty() &&
                (versionResponse.back() == '\r' || versionResponse.back() == '\n' ||
                    versionResponse.back() == ' ' || versionResponse.back() == '\t'))
                versionResponse.pop_back();

            serverVersion = versionResponse;

            // Read cached version
            std::string cachedVersion;
            std::ifstream verFile(verPath);
            if (verFile)
                std::getline(verFile, cachedVersion);

            if (cachedVersion != serverVersion || !std::filesystem::exists(jsonPath))
                needsUpdate = true;
        }
        else if (!std::filesystem::exists(jsonPath))
        {
            // Can't reach server and no cached file
            return false;
        }

        // Download new offsets if needed
        if (needsUpdate)
        {
            std::string jsonResponse;
            if (!internal::http_get(L"offsets.imtheo.lol", L"/offsets.json", jsonResponse))
            {
                if (!std::filesystem::exists(jsonPath))
                    return false;
            }
            else
            {
                // Save offsets.json
                {
                    std::ofstream out(jsonPath, std::ios::binary | std::ios::trunc);
                    if (out)
                        out.write(jsonResponse.data(), jsonResponse.size());
                }

                // Save version
                if (!serverVersion.empty())
                {
                    std::ofstream out(verPath, std::ios::trunc);
                    if (out)
                        out << serverVersion;
                }
            }
        }

        // Parse JSON
        std::ifstream file(jsonPath);
        if (!file)
            return false;

        try
        {
            file >> data;
            if (!serverVersion.empty())
                Version = serverVersion;
            else
            {
                std::ifstream vf(verPath);
                if (vf)
                    std::getline(vf, Version);
            }
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    uintptr_t Get(const char* cls, const char* field)
    {
        try
        {
            return data[cls][field].get<uintptr_t>();
        }
        catch (...)
        {
            return 0;
        }
    }
}
