#pragma once

#include "../sdk/sdk.h"

namespace wallcheck
{
    bool valid(const sdk::vector3& v);
    bool can_see(const sdk::vector3& origin, const sdk::vector3& target);
    void update_cache();
}
