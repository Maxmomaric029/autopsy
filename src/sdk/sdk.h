#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <complex>
#include <cmath>

#include "../core/memory.h"
#include "offsets/offset.h"

namespace sdk
{
    // Math types
    struct vector2 final {
        float x{ 0.0f }, y{ 0.0f };

        vector2 operator+(const vector2& rhs) const { return { x + rhs.x, y + rhs.y }; }
        vector2 operator-(const vector2& rhs) const { return { x - rhs.x, y - rhs.y }; }
        vector2 operator*(float scalar) const { return { x * scalar, y * scalar }; }
        vector2& operator+=(const vector2& rhs) { x += rhs.x; y += rhs.y; return *this; }
        vector2& operator-=(const vector2& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
        float distance(const vector2& other) const {
            float dx = x - other.x, dy = y - other.y;
            return std::sqrt(dx * dx + dy * dy);
        }
    };

    struct vector3 final {
        float x{ 0.0f }, y{ 0.0f }, z{ 0.0f };

        vector3 operator+(const vector3& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
        vector3 operator-(const vector3& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }
        vector3 operator*(float scalar) const { return { x * scalar, y * scalar, z * scalar }; }
        vector3 operator*(const vector3& o) const { return { x * o.x, y * o.y, z * o.z }; }
        vector3 operator/(const vector3& rhs) const { return { x / rhs.x, y / rhs.y, z / rhs.z }; }
        vector3& operator+=(const vector3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
        vector3& operator-=(const vector3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
        bool operator==(const vector3& o) const { return x == o.x && y == o.y && z == o.z; }
        vector3 operator/(float scalar) const { return { x / scalar, y / scalar, z / scalar }; }
        vector3 operator-() const { return { -x, -y, -z }; }

        float magnitude() const { return sqrtf(powf(x, 2) + powf(y, 2) + powf(z, 2)); }
        float distance(vector3 v) const { return (*this - v).magnitude(); }

        vector3 normalize() const {
            float mag = magnitude();
            return { x / mag, y / mag, z / mag };
        }

        vector3 cross(vector3 vec) const {
            return {
                y * vec.z - z * vec.y,
                -(x * vec.z - z * vec.x),
                x * vec.y - y * vec.x
            };
        }
    };

    struct matrix4 final { float data[16]; };

    struct matrix3 final {
        float data[9];
        vector3 operator*(const vector3& v) const {
            return {
                data[0] * v.x + data[1] * v.y + data[2] * v.z,
                data[3] * v.x + data[4] * v.y + data[5] * v.z,
                data[6] * v.x + data[7] * v.y + data[8] * v.z
            };
        }
    };

    struct vector4 final { float x, y, z, w; };
    struct viewport { unsigned short x, y; };

    // SDK types
    struct instance {
        uintptr_t Address = 0;

        instance() = default;
        explicit instance(uintptr_t addr) : Address(addr) {}

        std::string name() const;
        std::string text() const;
        std::string kind() const;

        instance parent() const;
        std::vector<instance> children() const;
        instance character() const;

        instance child(const std::string& name) const;
        instance childclass(const std::string& Class_Name) const;

        template <typename T>
        std::vector<T> childrenas() const {
            std::vector<T> list;
            if (!Address) return list;
            for (sdk::instance child : children()) {
                if (!child.Address) continue;
                list.emplace_back(T(child.Address));
            }
            return list;
        }
    };

    struct model : public instance {
        using instance::instance;
        std::uint64_t place() const;
        std::uint64_t game() const;
        std::uint64_t creator() const;
        std::uint64_t server() const;
    };

    struct render : public instance {
        using instance::instance;
        sdk::vector2 size() const;
        sdk::matrix4 matrix() const;
        sdk::vector2 screen(const sdk::vector3& world) const;
    };

    struct actor : public instance {
        using instance::instance;
        std::uint64_t userid() const;
        std::uint64_t teamid() const;
        std::string display() const;
        actor local() const;
    };

    struct camera : public instance {
        using instance::instance;
        sdk::vector3 position() const;
        sdk::matrix3 rotation() const;
        void position(const sdk::vector3& pos) const;
        void rotation(const sdk::matrix3& rot) const;
        vector2 viewportat(vector2 target_screen_pos, vector2 screen_size);
        void viewport(sdk::viewport Vp) const;
    };

    struct humanoid : public instance {
        using instance::instance;
        float health() const;
        float maxhealth() const;
        void kill() const;
        int rig() const;
    };

    struct part : public instance {
        using instance::instance;
        sdk::vector3 position() const;
        sdk::matrix3 rotation() const;
        sdk::vector3 size() const;
        sdk::vector3 movedir() const;
        sdk::vector3 velocity() const;
        sdk::vector3 cframe() const;
        sdk::vector3 partposition() const;
        part primitive() const;
        float transparency() const;
        bool anchored() const;

        void velocity(const sdk::vector3& Velocity) const;
        void partposition(const sdk::vector3& Position) const;
        void rotation(const sdk::matrix3& Rotation) const;
        void mesh(const std::string& MeshID) const;
        void transparency(float Transparency) const;
        void movedir(const sdk::vector3& MoveDir) const;
    };

    struct view : public instance {
        using instance::instance;
        std::uint64_t baseview();
        static void invalidate();
        static void skybox();
    };

    struct light : public instance {
        using instance::instance;
        static void ambient(uintptr_t LightingAddress, sdk::vector3 tocolor);
        static void fog(uintptr_t LightingAddress, float end, sdk::vector3 tocolor);
        static void brightness(uintptr_t LightingAddress, float Value);
        static void exposure(uintptr_t LightingAddress, float Value);
        static void fov(uintptr_t CameraAddress, float Degrees);
    };
}
