#include "engine.h"
#include "global.h"

namespace sdk {

    std::uint64_t view::baseview() {

        auto Rv1 = drive->read<std::uintptr_t>(global::model.Address + offset::datamodel::ToRenderView1);
        auto Rv2 = drive->read<std::uintptr_t>(Rv1 + offset::datamodel::ToRenderView2);
        auto Rv3 = drive->read<std::uintptr_t>(Rv2 + offset::datamodel::ToRenderView3);
        return Rv3;
    }

    void view::invalidate() {

        drive->write<bool>(global::view.baseview() + offset::view::LightingValid, false);
    }

    void view::skybox() {

        drive->write<bool>(global::view.baseview() + offset::view::SkyValid, false);
    }
}
