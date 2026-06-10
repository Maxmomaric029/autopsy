#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <cmath>
#include <unordered_map>

namespace anim {

    inline float saturate(float v) {
        return ImClamp(v, 0.f, 1.f);
    }

    inline float ease_out_cubic(float t) {
        t = 1.f - saturate(t);
        return 1.f - t * t * t;
    }

    inline float ease_out_quint(float t) {
        t = 1.f - saturate(t);
        return 1.f - t * t * t * t * t;
    }

    inline float ease_in_out_cubic(float t) {
        t = saturate(t);
        return t < 0.5f ? 4.f * t * t * t
                        : 1.f - powf(-2.f * t + 2.f, 3.f) * 0.5f;
    }

    inline float damp(float value, float target, float speed) {
        const float dt = ImClamp(ImGui::GetIO().DeltaTime, 0.f, 1.f / 30.f);
        const float k = 1.f - expf(-speed * dt);
        const float v = ImLerp(value, target, k);
        return fabsf(v - target) < 0.0007f ? target : v;
    }

    struct AnimationState {
        std::unordered_map<ImGuiID, float> values;

        float get(ImGuiID id, bool on, float speed = 10.f) {
            if (values.empty()) values.reserve(128);
            // Read/update current entry FIRST
            float& v = values[id];
            v = damp(v, on ? 1.f : 0.f, speed);
            // Save a copy BEFORE pruning — the pruning loop may erase this entry
            // causing a dangling reference on `return v` (UB).
            float result = v;
            // Prune settled entries if cache exceeds 512 (F1.8)
            if (values.size() > 512) {
                for (auto it = values.begin(); it != values.end(); ) {
                    if (it->second == 0.f || it->second == 1.f)
                        it = values.erase(it);
                    else
                        ++it;
                }
            }
            return result;
        }

        float get_hover(ImGuiID id, float speed = 9.f) {
            bool hov = ImGui::IsItemHovered();
            return ease_out_cubic(get(id, hov, speed));
        }

        float get_toggle(ImGuiID id, bool on) {
            return ease_in_out_cubic(get(id, on, 9.5f));
        }
    };

    inline AnimationState g_anim;

    // Shorthand
    inline float hover(ImGuiID id, float speed = 9.f) {
        return g_anim.get_hover(id, speed);
    }

    inline float toggle(ImGuiID id, bool on) {
        return g_anim.get_toggle(id, on);
    }

    inline float pulse(float speed = 2.5f) {
        return (sinf((float)ImGui::GetTime() * speed) + 1.f) * 0.5f;
    }

} // namespace anim
