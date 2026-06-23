#define NOMINMAX
#include <Windows.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <cfloat>
#include "pf.h"
#include "global.h"
#include "../sdk/sdk.h"
#include "../core/cache.h"

// ── Helpers ────────────────────────────────────────────────────────────────

static inline bool validscreen(float x, float y)
{
    return x > -0.5f && y > -0.5f;
}

static inline float dot3(const sdk::vector3& a, const sdk::vector3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static bool validvector(const sdk::vector3& v)
{
    return !std::isnan(v.x) && !std::isnan(v.y) && !std::isnan(v.z) &&
        !(v.x == 0.f && v.y == 0.f && v.z == 0.f);
}

// ── Simple 3D bounding box ─────────────────────────────────────────────
// Draws a 3D-oriented bounding box around a part then projects corners

struct Box3D {
    sdk::vector3 corners[8];
    bool valid = false;
};

static Box3D compute_box(const sdk::instance& part)
{
    Box3D box{};
    if (!part.Address) return box;

    sdk::part p(part.Address);
    sdk::primitive_data pd;
    if (!p.get_primitive_data(pd)) return box;

    // Padding factor for the box
    sdk::vector3 size = pd.size;
    if (part.Address == p.Address) {
        // Default size scaling per part type
    }

    // 8 corners of the oriented bounding box
    sdk::vector3 local_corners[8] = {
        {-1,-1,-1}, {1,-1,-1}, {-1,1,-1}, {1,1,-1},
        {-1,-1,1},  {1,-1,1},  {-1,1,1},  {1,1,1}
    };

    for (int i = 0; i < 8; i++) {
        sdk::vector3 local = {
            local_corners[i].x * size.x * 0.5f,
            local_corners[i].y * size.y * 0.5f,
            local_corners[i].z * size.z * 0.5f
        };
        // Transform by rotation matrix
        box.corners[i] = pd.position + pd.rotation * local;
    }

    box.valid = true;
    return box;
}

// ── Silent aim logic for PF ────────────────────────────────────────────
// Handles camera rotation on left-click for Phantom Forces
namespace pf_silent {

    static sdk::matrix3 saved_rot{};
    static bool has_saved_rot = false;
    static bool last_lbutton = false;

    void run()
    {
        if (!global::pf::Enabled || !global::pf::SilentAim)
            return;

        // Only run in PF
        if (global::GameID != global::pf::PlaceId)
            return;

        if (!global::camera.Address)
            return;

        // Hide head for better visibility
        if (global::LocalPlayer.Head.Address) {
            sdk::part head(global::LocalPlayer.Head.Address);
            head.transparency(1.f);
        }

        bool currentState = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

        if (!currentState && last_lbutton) {
            // Left button released → restore saved rotation
            if (has_saved_rot) {
                sdk::camera cam(global::camera.Address);
                cam.rotation(saved_rot);
            }
        }

        if (currentState && !last_lbutton) {
            // Left button pressed → save current rotation, then aim
            sdk::camera cam(global::camera.Address);
            sdk::matrix3 camRot = cam.rotation();

            // Save current rotation for restoration
            saved_rot = camRot;
            has_saved_rot = true;

            // Find closest enemy to aim at
            sdk::player closest{};
            float closestDist = std::numeric_limits<float>::max();

            std::vector<sdk::player> snapshot;
            {
                std::lock_guard<std::mutex> lock(cache::Mutex);
                snapshot = global::Player_Cache;
            }

            for (auto& player : snapshot) {
                if (!player.character.Address) continue;
                if (player.character.Address == global::LocalPlayer.character.Address) continue;

                sdk::vector3 pos{};
                if (player.Head.Address) {
                    sdk::part head(player.Head.Address);
                    pos = head.partposition();
                } else if (player.HumanoidRootPart.Address) {
                    sdk::part root(player.HumanoidRootPart.Address);
                    pos = root.partposition();
                } else continue;

                if (!validvector(pos)) continue;

                sdk::camera camPos(global::camera.Address);
                float dist = camPos.position().distance(pos);
                if (dist < closestDist) {
                    closestDist = dist;
                    closest = player;
                }
            }

            if (closest.character.Address) {
                // Get target position
                sdk::vector3 targetPos{};
                if (closest.Head.Address) {
                    sdk::part head(closest.Head.Address);
                    targetPos = head.partposition();
                } else if (closest.HumanoidRootPart.Address) {
                    sdk::part root(closest.HumanoidRootPart.Address);
                    targetPos = root.partposition();
                }

                if (validvector(targetPos)) {
                    sdk::camera camPos(global::camera.Address);
                    sdk::vector3 from = camPos.position();

                    // Compute look-at rotation: only modify Y and Z rotation (pitch/yaw)
                    sdk::vector3 dir = targetPos - from;
                    if (validvector(dir)) {
                        // Just set the camera rotation to look at target
                        // We preserve the Z component (roll) from saved rotation
                        sdk::vector3 forward = dir.normalize();
                        sdk::vector3 worldUp{ 0.f, 1.f, 0.f };
                        sdk::vector3 right = worldUp.cross(forward);
                        right = right.normalize();
                        sdk::vector3 up = forward.cross(right);

                        sdk::matrix3 aimRot{};
                        aimRot.data[0] = right.x;   aimRot.data[1] = up.x;   aimRot.data[2] = forward.x;
                        aimRot.data[3] = right.y;   aimRot.data[4] = up.y;   aimRot.data[5] = forward.y;
                        aimRot.data[6] = right.z;   aimRot.data[7] = up.z;   aimRot.data[8] = forward.z;

                        // Apply rotation preserving roll from original
                        sdk::matrix3 newRot = camRot;
                        newRot.data[2] = aimRot.data[2];
                        newRot.data[5] = aimRot.data[5];
                        newRot.data[8] = aimRot.data[8];

                        sdk::camera camSet(global::camera.Address);
                        camSet.rotation(newRot);

                        // Update saved_rot to the new rotation for next frame
                        saved_rot = camSet.rotation();
                    }
                }
            }
        }

        last_lbutton = currentState;
    }
}

// ── PF ESP ─────────────────────────────────────────────────────────────

namespace pf_esp {

    static void draw_skeleton(ImDrawList* dl, const sdk::player& player,
        const sdk::vector2& dims, const sdk::matrix4& viewMatrix)
    {
        auto draw_bone = [&](const sdk::instance& part1, const sdk::instance& part2, ImU32 col) {
            if (!part1.Address || !part2.Address) return;
            sdk::part p1(part1.Address);
            sdk::part p2(part2.Address);
            sdk::vector3 pos1 = p1.partposition();
            sdk::vector3 pos2 = p2.partposition();
            if (!validvector(pos1) || !validvector(pos2)) return;
            sdk::vector2 s1 = global::render.screen(pos1);
            sdk::vector2 s2 = global::render.screen(pos2);
            if (validscreen(s1.x, s1.y) && validscreen(s2.x, s2.y)) {
                dl->AddLine(ImVec2(s1.x, s1.y), ImVec2(s2.x, s2.y), IM_COL32(0, 0, 0, 120), 3.f);
                dl->AddLine(ImVec2(s1.x, s1.y), ImVec2(s2.x, s2.y), col, 1.5f);
            }
        };

        if (player.Head.Address && player.UpperTorso.Address) {
            ImU32 col = IM_COL32(
                (int)(global::pf::SkeletonColor[0] * 255),
                (int)(global::pf::SkeletonColor[1] * 255),
                (int)(global::pf::SkeletonColor[2] * 255),
                (int)(global::pf::SkeletonColor[3] * 255)
            );

            draw_bone(player.Head, player.UpperTorso, col);

            if (player.pfLimbs1.Address) draw_bone(player.UpperTorso, player.pfLimbs1, col);
            if (player.pfLimbs2.Address) draw_bone(player.UpperTorso, player.pfLimbs2, col);
            if (player.pfLimbs3.Address) draw_bone(player.UpperTorso, player.pfLimbs3, col);
            if (player.pfLimbs4.Address) draw_bone(player.UpperTorso, player.pfLimbs4, col);
            if (player.pfLimbs5.Address) draw_bone(player.UpperTorso, player.pfLimbs5, col);
        }
    }

    static void render_player(ImDrawList* dl, const sdk::player& player,
        const sdk::vector2& dims, const sdk::matrix4& viewMatrix)
    {
        // Collect parts for bounding box
        std::vector<sdk::instance> parts;
        if (player.Head.Address) parts.push_back(player.Head);
        if (player.UpperTorso.Address) parts.push_back(player.UpperTorso);
        if (player.pfLimbs1.Address) parts.push_back(player.pfLimbs1);
        if (player.pfLimbs2.Address) parts.push_back(player.pfLimbs2);
        if (player.pfLimbs3.Address) parts.push_back(player.pfLimbs3);
        if (player.pfLimbs4.Address) parts.push_back(player.pfLimbs4);
        if (player.pfLimbs5.Address) parts.push_back(player.pfLimbs5);
        if (player.HumanoidRootPart.Address) parts.push_back(player.HumanoidRootPart);

        // Compute 2D bounding box from projected part positions
        float left = FLT_MAX, top = FLT_MAX, right = -FLT_MAX, bottom = -FLT_MAX;
        bool valid = false;

        for (auto& part : parts) {
            if (!part.Address) continue;
            Box3D box = compute_box(part);
            if (!box.valid) continue;

            for (int i = 0; i < 8; i++) {
                sdk::vector2 screen = global::render.screen(box.corners[i]);
                if (validscreen(screen.x, screen.y)) {
                    valid = true;
                    if (screen.x < left) left = screen.x;
                    if (screen.y < top) top = screen.y;
                    if (screen.x > right) right = screen.x;
                    if (screen.y > bottom) bottom = screen.y;
                }
            }
        }

        if (!valid || left >= right || top >= bottom) return;

        // Box color
        ImU32 boxCol = IM_COL32(
            (int)(global::pf::BoxColor[0] * 255),
            (int)(global::pf::BoxColor[1] * 255),
            (int)(global::pf::BoxColor[2] * 255),
            (int)(global::pf::BoxColor[3] * 255)
        );

        // Draw bounding box
        if (global::pf::Box) {
            dl->AddRect(ImVec2(left, top), ImVec2(right, bottom), boxCol, 0.f, 0, 1.5f);
        }

        // Draw health bar
        if (global::pf::HealthBar && player.MaxHealth > 0.f) {
            float ratio = player.Health / player.MaxHealth;
            ratio = std::clamp(ratio, 0.f, 1.f);
            float barHeight = bottom - top;
            float barWidth = 4.f;
            float barX = left - barWidth - 3.f;

            // Background
            dl->AddRectFilled(ImVec2(barX, top), ImVec2(barX + barWidth, bottom), IM_COL32(0, 0, 0, 180), 2.f);
            // Health fill
            float fillH = barHeight * ratio;
            if (fillH > 0.f) {
                ImU32 healthCol = IM_COL32(
                    (int)(global::pf::HealthColor[0] * 255),
                    (int)(global::pf::HealthColor[1] * 255),
                    (int)(global::pf::HealthColor[2] * 255),
                    (int)(global::pf::HealthColor[3] * 255)
                );
                dl->AddRectFilled(ImVec2(barX + 1.f, bottom - fillH + 1.f),
                    ImVec2(barX + barWidth - 1.f, bottom - 1.f), healthCol, 1.f);
            }
        }

        // Draw skeleton
        if (global::pf::Skeleton) {
            draw_skeleton(dl, player, dims, viewMatrix);
        }
    }

    void run()
    {
        if (!global::pf::Enabled || !global::pf::Esp)
            return;
        if (global::GameID != global::pf::PlaceId)
            return;
        if (global::render.Address == 0 || global::model.Address == 0)
            return;

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        sdk::vector2 dims = global::render.size();
        sdk::matrix4 viewMatrix = global::render.matrix();

        // Snapshot
        std::vector<sdk::player> snapshot;
        {
            std::lock_guard<std::mutex> lock(cache::Mutex);
            snapshot = global::Player_Cache;
        }

        for (auto& player : snapshot) {
            if (!player.character.Address || player.character.Address == global::LocalPlayer.character.Address)
                continue;
            render_player(dl, player, dims, viewMatrix);
        }
    }
}

// ── Public API ─────────────────────────────────────────────────────────

void pf::silent_run()
{
    pf_silent::run();
}

void pf::esp_run()
{
    pf_esp::run();
}

void pf::run()
{
    // No-op: both silent_run() and esp_run() are called per-frame
    // from ModernUI::RenderESP() in modern_ui.cpp
}
