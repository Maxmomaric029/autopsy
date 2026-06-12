#include "sdk.h"
#include "global.h"

namespace sdk {

    // ---- instance ----

    static bool is_valid_instance_address(uintptr_t addr)
    {
        return addr != 0 && addr != 0xFFFFFFFFFFFFFFFF;
    }

    std::string instance::name() const {
        if (!Address) return "?";
        uintptr_t StringPointer = drive->read<uintptr_t>(Address + offset::instance::name);
        if (!StringPointer) return "?";
        return drive->readstring(StringPointer);
    }

    std::string instance::text() const {
        return drive->readstring(this->Address + offset::gui::text);
    }

    std::string instance::kind() const {
        if (!Address) return "?";
        uintptr_t Descriptor = drive->read<uintptr_t>(Address + offset::instance::ClassDescriptor);
        if (!Descriptor) return "?";
        uintptr_t StringPointer = drive->read<uintptr_t>(Descriptor + offset::instance::ClassName);
        if (!StringPointer) return "?";
        return drive->readstring(StringPointer);
    }

    instance instance::parent() const {
        if (!Address) return instance();
        return drive->read<instance>(Address + offset::instance::parent);
    }

    std::vector<instance> instance::children() const {
        std::vector<instance> Container;
        if (!Address) return Container;

        // Read container pointer from Instance+ChildrenStart
        uintptr_t container = drive->read<uintptr_t>(Address + offset::instance::ChildrenStart);
        if (!container) return Container;

        // Read begin and end FROM the container
        // Container { [0x00] begin_ptr, [0x08] end_ptr }
        uintptr_t begin = drive->read<uintptr_t>(container);
        uintptr_t end   = drive->read<uintptr_t>(container + offset::instance::ChildrenEnd);
        if (!begin || !end || end <= begin) return Container;

        // Safety cap: never iterate more than 5000 entries
        const int MAX_CHILDREN = 5000;
        int count = 0;

        // Iterate with stride = sizeof(uintptr_t) = 8
        for (uintptr_t ptr = begin; ptr < end && count < MAX_CHILDREN; ptr += sizeof(uintptr_t))
        {
            uintptr_t child = drive->read<uintptr_t>(ptr);
            if (is_valid_instance_address(child))
                Container.emplace_back(child);
            count++;
        }

        return Container;
    }

    instance instance::child(const std::string& name) const {
        if (!Address || name.empty()) return instance();
        for (instance Child : children()) {
            if (!Child.Address) continue;
            if (Child.name() == name) return Child;
        }
        return instance();
    }

    instance instance::childclass(const std::string& Class_Name) const {
        if (!Address || Class_Name.empty()) return instance();
        for (instance Child : children()) {
            if (!Child.Address) continue;
            if (Child.kind() == Class_Name) return Child;
        }
        return instance();
    }

    instance instance::character() const {
        if (!this->Address) return instance();
        uintptr_t charAddr = drive->read<uintptr_t>(this->Address + offset::player::ModelInstance);
        return instance(charAddr);
    }

    // ---- actor ----

    std::uint64_t actor::userid() const {
        return drive->read<std::uint64_t>(this->Address + offset::player::UserId);
    }

    std::uint64_t actor::teamid() const {
        return drive->read<uint64_t>(this->Address + offset::player::team);
    }

    actor actor::local() const {
        std::uint64_t local_address = drive->read<std::uint64_t>(this->Address + offset::player::LocalPlayer);
        return actor{ local_address };
    }

    std::string actor::display() const {
        if (!this->Address) return "?";
        return drive->readstring(this->Address + offset::player::DisplayName);
    }

    // ---- camera ----

    vector3 camera::position() const {
        return drive->read<vector3>(this->Address + offset::camera::Position);
    }

    matrix3 camera::rotation() const {
        return drive->read<matrix3>(this->Address + offset::camera::Rotation);
    }

    void camera::position(const vector3& pos) const {
        drive->write<vector3>(this->Address + offset::camera::Position, pos);
    }

    void camera::rotation(const matrix3& rot) const {
        drive->write<matrix3>(this->Address + offset::camera::Rotation, rot);
    }

    vector2 camera::viewportat(vector2 target_screen_pos, vector2 screen_size) {
        vector2 result;
        result.x = (int16_t)(2 * (screen_size.x - target_screen_pos.x));
        result.y = (int16_t)(2 * (screen_size.y - target_screen_pos.y));
        return result;
    }

    void camera::set_viewport(viewport Vp) const {
        drive->write<viewport>(this->Address + offset::camera::Viewport, Vp);
    }

    // ---- humanoid ----

    float humanoid::health() const {
        if (!this->Address) return 0.0f;
        union Conv { std::uint64_t hex; float f; } EasyConversion;
        uintptr_t healthPtr = drive->read<uintptr_t>(this->Address + offset::humanoid::Health);
        uintptr_t healthRead = drive->read<uintptr_t>(healthPtr);
        EasyConversion.hex = healthPtr ^ healthRead;
        return EasyConversion.f;
    }

    float humanoid::maxhealth() const {
        if (!this->Address) return 0.0f;
        union Conv { std::uint64_t hex; float f; } EasyConversion;
        uintptr_t healthPtr = drive->read<uintptr_t>(this->Address + offset::humanoid::MaxHealth);
        uintptr_t healthRead = drive->read<uintptr_t>(healthPtr);
        EasyConversion.hex = healthPtr ^ healthRead;
        return EasyConversion.f;
    }

    void humanoid::kill() const {
        if (!this->Address) return;
        drive->write<uintptr_t>(this->Address + offset::humanoid::Health, 0);
    }

    int humanoid::rig() const {
        if (!this->Address) return 0;
        return drive->read<int>(this->Address + offset::humanoid::RigType);
    }

    // ---- light ----

    void light::ambient(uintptr_t LightingAddress, vector3 tocolor) {
        drive->write<vector3>(LightingAddress + offset::light::Ambient, tocolor);
        drive->write<vector3>(LightingAddress + offset::light::OutdoorAmbient, tocolor);
    }

    void light::fog(uintptr_t LightingAddress, float end, vector3 tocolor) {
        drive->write<float>(LightingAddress + offset::light::FogEnd, end);
        drive->write<vector3>(LightingAddress + offset::light::FogColor, tocolor);
    }

    void light::brightness(uintptr_t LightingAddress, float Value) {
        drive->write<float>(LightingAddress + offset::light::Brightness, Value);
    }

    void light::exposure(uintptr_t LightingAddress, float Value) {
        drive->write<float>(LightingAddress + offset::light::ExposureCompensation, Value);
    }

    void light::fov(uintptr_t CameraAddress, float Degrees) {
        drive->write<float>(CameraAddress + offset::camera::FieldOfView, Degrees * 0.0174533f);
    }

    // ---- model ----

    std::uint64_t model::place() const {
        return drive->read<uint64_t>(this->Address + offset::datamodel::PlaceId);
    }

    std::uint64_t model::game() const {
        return drive->read<uint64_t>(this->Address + offset::datamodel::GameId);
    }

    std::uint64_t model::creator() const {
        return drive->read<uint64_t>(this->Address + offset::datamodel::CreatorId);
    }

    std::uint64_t model::server() const {
        return drive->read<uint64_t>(this->Address + offset::datamodel::ServerIP);
    }

    // ---- part ----

    struct RawRotPos {
        matrix3 rot;
        vector3 pos;
    };

    bool part::get_primitive_data(primitive_data& out) const {
        if (!Address) return false;
        uintptr_t prim = drive->read<uintptr_t>(Address + offset::basepart::primitive);
        if (!prim) {
            prim = Address;
        }
        RawRotPos rotpos = drive->read<RawRotPos>(prim + offset::primitive::Rotation);
        out.rotation = rotpos.rot;
        out.position = rotpos.pos;
        out.size = drive->read<vector3>(prim + offset::primitive::Size);
        return true;
    }

    vector3 part::position() const {
        return drive->read<vector3>(Address + offset::primitive::Position);
    }

    matrix3 part::rotation() const {
        return drive->read<matrix3>(Address + offset::primitive::Rotation);
    }

    vector3 part::size() const {
        return drive->read<vector3>(Address + offset::primitive::Size);
    }

    vector3 part::movedir() const {
        return drive->read<vector3>(Address + offset::player::CameraMode);
    }

    vector3 part::velocity() const {
        if (!Address) return {};
        uintptr_t primitive = drive->read<uintptr_t>(Address + offset::basepart::primitive);
        if (!primitive) return {};
        return drive->read<vector3>(primitive + offset::primitive::AssemblyAngularVelocity);
    }

    bool part::anchored() const {
        return drive->read<bool>(Address + offset::primitiveflag::Anchored);
    }

    bool part::cancollide() const {
        return drive->read<bool>(Address + offset::primitiveflag::CanCollide);
    }

    vector3 part::cframe() const {
        return drive->read<vector3>(Address + offset::primitive::Rotation);
    }

    part part::primitive() const {
        uintptr_t primitiveAddress = drive->read<uintptr_t>(Address + offset::basepart::primitive);
        return part{ primitiveAddress };
    }

    vector3 part::partposition() const {
        auto item = this->primitive();
        if (!item.Address) return {};
        return drive->read<vector3>(item.Address + offset::primitive::Position);
    }

    float part::transparency() const {
        return drive->read<float>(Address + offset::basepart::Transparency);
    }

    void part::velocity(const vector3& Velocity) const {
        auto item = this->primitive();
        if (!item.Address) return;
        drive->write<vector3>(item.Address + offset::primitive::AssemblyLinearVelocity, Velocity);
    }

    void part::partposition(const vector3& Position) const {
        auto item = this->primitive();
        if (!item.Address) return;
        drive->write<vector3>(item.Address + offset::primitive::Position, Position);
    }

    void part::rotation(const matrix3& Rotation) const {
        drive->write<matrix3>(Address + offset::primitive::Rotation, Rotation);
    }

    void part::mesh(const std::string& MeshID) const {
        drive->writestring(Address + offset::meshpart::MeshId, MeshID);
    }

    void part::transparency(float Transparency) const {
        drive->write<float>(Address + offset::basepart::Transparency, Transparency);
    }

    void part::movedir(const vector3& MoveDir) const {
        drive->write<vector3>(Address + offset::player::CameraMode, MoveDir);
    }

    // ---- render ----

    vector2 render::size() const {
        return drive->read<vector2>(this->Address + offset::render::Dimensions);
    }

    matrix4 render::matrix() const {
        return drive->read<matrix4>(this->Address + offset::render::ViewMatrix);
    }

    vector2 render::screen(const vector3& world) const {
        matrix4 ViewMatrix = matrix();
        vector2 Dimensions = size();
        vector4 Q;
        Q.x = (world.x * ViewMatrix.data[0]) + (world.y * ViewMatrix.data[1]) + (world.z * ViewMatrix.data[2]) + ViewMatrix.data[3];
        Q.y = (world.x * ViewMatrix.data[4]) + (world.y * ViewMatrix.data[5]) + (world.z * ViewMatrix.data[6]) + ViewMatrix.data[7];
        Q.z = (world.x * ViewMatrix.data[8]) + (world.y * ViewMatrix.data[9]) + (world.z * ViewMatrix.data[10]) + ViewMatrix.data[11];
        Q.w = (world.x * ViewMatrix.data[12]) + (world.y * ViewMatrix.data[13]) + (world.z * ViewMatrix.data[14]) + ViewMatrix.data[15];
        if (Q.w < 0.1f) return { -1, -1 };
        vector3 NDC;
        NDC.x = Q.x / Q.w;
        NDC.y = Q.y / Q.w;
        NDC.z = Q.z / Q.w;
        vector2 ScreenPosition = {
            (Dimensions.x / 2 * NDC.x) + (Dimensions.x / 2),
            -(Dimensions.y / 2 * NDC.y) + (Dimensions.y / 2),
        };
        if (ScreenPosition.x < 0 || ScreenPosition.x > Dimensions.x || ScreenPosition.y < 0 || ScreenPosition.y > Dimensions.y)
            return { -1, -1 };
        return ScreenPosition;
    }

    // ---- view ----

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
