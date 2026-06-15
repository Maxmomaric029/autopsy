#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <string_view>

#include <atomic>
#include <cmath>

#include "../sdk/sdk.h"

// Forward declarations (defined as inline in features/phantom.h)
inline void cacheplayer(std::vector<sdk::player>& actor, const sdk::vector3& LocalPos, const std::string& LocalName);
inline void rescancache(std::vector<sdk::player>& actor, const sdk::vector3& LocalPos, const std::string& LocalName);

namespace cache
{
    extern std::mutex Mutex;
    extern std::atomic<bool> Is_Running;
    void run();
}
