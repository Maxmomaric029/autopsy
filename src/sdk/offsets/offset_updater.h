#pragma once

#include <cstdint>
#include <string>

namespace Offsets
{
    extern bool Load();
    extern uintptr_t Get(const char* cls, const char* field);
    extern std::string Version;

    namespace internal
    {
        extern std::string exe_dir();
        extern bool http_get(const std::wstring& host, const std::wstring& path, std::string& out);
    }
}
