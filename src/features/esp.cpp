#include <iostream>
#include "../features/esp.h"
#include "../sdk/sdk.h"
#include <cfloat>
#include <cmath>
#include <imgui/imgui.h>
#include "global.h"
#include <Windows.h>
#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <imgui/imgui_internal.h>
#include "ui/graphic.h"
#include "../features/wallcheck.h"
#include "../features/silent.h"

#define IMGUI_DEFINE_MATH_OPERATORS

// ── Helpers ────────────────────────────────────────────────────────────────

static inline bool validscreen(float x, float y)
{
    return x > -0.5f && y > -0.5f;
}

static inline float dot3(const sdk::vector3& a, const sdk::vector3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// ── Visual frame (per-frame camera & window data) ──────────────────────────

namespace visual_frame
{
    static HWND Window = nullptr;
    static RECT ClientRect{};
    static bool HasClientRect = false;
    static sdk::vector3 CameraPos{};
    static bool HasCamera = false;

    static void begin()
    {
        if (!Window || !IsWindow(Window))
            Window = FindWindowA(nullptr, "Roblox");

        HasClientRect = Window && GetClientRect(Window, &ClientRect);
        HasCamera = false;

        if (global::camera.Address)
        {
            sdk::camera cam(global::camera.Address);
            CameraPos = cam.position();
            HasCamera = !(std::isnan(CameraPos.x) || std::isnan(CameraPos.y) || std::isnan(CameraPos.z));
        }
    }
}

static bool visiblecheck(const sdk::vector3& target, const sdk::vector2& screen)
{
    if (!visual_frame::HasClientRect)
        return false;

    if (!(screen.x >= 0.f && screen.y >= 0.f &&
        screen.x <= (float)visual_frame::ClientRect.right &&
        screen.y <= (float)visual_frame::ClientRect.bottom))
        return false;

    if (!visual_frame::HasCamera)
        return false;

    return wallcheck::can_see(visual_frame::CameraPos, target);
}

// ── Projection helper ──────────────────────────────────────────────────────

static bool project(const sdk::vector3& world, ImVec2& out)
{
    auto screen = global::render.screen(world);
    if (!validscreen(screen.x, screen.y))
        return false;
    out.x = std::roundf(screen.x);
    out.y = std::roundf(screen.y);
    return true;
}

// ── Part pose helper ───────────────────────────────────────────────────────

static bool partpose(const sdk::instance& inst, sdk::vector3& position)
{
    if (!inst.Address)
        return false;
    sdk::part p(inst.Address);
    sdk::primitive_data pdata;
    if (!p.get_primitive_data(pdata))
        return false;
    position = pdata.position;
    return !(std::isnan(position.x) || std::isnan(position.y) || std::isnan(position.z));
}

static bool playerposition(const sdk::player& player, sdk::vector3& out)
{
    if (partpose(player.HumanoidRootPart, out)) return true;
    if (partpose(player.LowerTorso, out)) return true;
    if (partpose(player.Torso, out)) return true;
    return partpose(player.Head, out);
}

// ── Color helpers ──────────────────────────────────────────────────────────

static ImU32 col4_to_u32(const float c[4], float alpha_mul = 1.f)
{
    return IM_COL32(
        (int)(ImClamp(c[0], 0.f, 1.f) * 255.f),
        (int)(ImClamp(c[1], 0.f, 1.f) * 255.f),
        (int)(ImClamp(c[2], 0.f, 1.f) * 255.f),
        (int)(ImClamp(c[3] * alpha_mul, 0.f, 1.f) * 255.f));
}

static ImU32 u32_alpha(ImU32 c, float a)
{
    int alpha = (int)(((c >> 24) & 0xFF) * ImClamp(a, 0.f, 1.f));
    return (c & 0x00FFFFFF) | ((ImU32)alpha << 24);
}

static ImU32 health_color(float ratio)
{
    ratio = ImClamp(ratio, 0.f, 1.f);
    // green → yellow → red
    if (ratio > 0.5f)
    {
        float t = (ratio - 0.5f) * 2.f;
        return IM_COL32(
            (int)(61 + (61) * t),
            (int)(224 + (-32) * t),
            (int)(160 + (-80) * t), 255);
    }
    float t = ratio * 2.f;
    return IM_COL32(
        (int)(224 + (16) * t),
        (int)(60 + (132) * t),
        (int)(70 + (10) * t), 255);
}

// ── Draw primitives ────────────────────────────────────────────────────────

static void draw_text_shadowed(ImDrawList* dl, ImFont* font, float size, ImVec2 pos, ImU32 col, const char* text)
{
    dl->AddText(font, size, pos + ImVec2(1.f, 1.f), IM_COL32(0, 0, 0, 180), text);
    dl->AddText(font, size, pos, col, text);
}

// ── ESP rendering ──────────────────────────────────────────────────────────

namespace esp {

    std::vector<const sdk::instance*> bone(const sdk::player& player) {

        std::vector<const sdk::instance*> Parts;
        Parts.reserve(player.UpperTorso.Address && player.LowerTorso.Address ? 15 : 8);

        const bool R15 = player.UpperTorso.Address && player.LowerTorso.Address;
        const bool R6 = player.Torso.Address;

        if (R15) {
            if (player.Head.Address) Parts.push_back(&player.Head);
            if (player.UpperTorso.Address) Parts.push_back(&player.UpperTorso);
            if (player.LowerTorso.Address) Parts.push_back(&player.LowerTorso);
            if (player.LeftUpperArm.Address) Parts.push_back(&player.LeftUpperArm);
            if (player.LeftLowerArm.Address) Parts.push_back(&player.LeftLowerArm);
            if (player.LeftHand.Address)     Parts.push_back(&player.LeftHand);
            if (player.RightUpperArm.Address) Parts.push_back(&player.RightUpperArm);
            if (player.RightLowerArm.Address) Parts.push_back(&player.RightLowerArm);
            if (player.RightHand.Address)     Parts.push_back(&player.RightHand);
            if (player.LeftUpperLeg.Address) Parts.push_back(&player.LeftUpperLeg);
            if (player.LeftLowerLeg.Address) Parts.push_back(&player.LeftLowerLeg);
            if (player.LeftFoot.Address)     Parts.push_back(&player.LeftFoot);
            if (player.RightUpperLeg.Address) Parts.push_back(&player.RightUpperLeg);
            if (player.RightLowerLeg.Address) Parts.push_back(&player.RightLowerLeg);
            if (player.RightFoot.Address)     Parts.push_back(&player.RightFoot);
        }
        else if (R6) {
            if (player.Head.Address)  Parts.push_back(&player.Head);
            if (player.Torso.Address) Parts.push_back(&player.Torso);
            if (player.LeftArm.Address)  Parts.push_back(&player.LeftArm);
            if (player.RightArm.Address) Parts.push_back(&player.RightArm);
            if (player.LeftLeg.Address)  Parts.push_back(&player.LeftLeg);
            if (player.RightLeg.Address) Parts.push_back(&player.RightLeg);
        }
        else {
            for (const auto& Bone : player.Bones) {
                if (Bone.Address)
                    Parts.push_back(&Bone);
            }
            if (Parts.empty()) {
                if (player.HumanoidRootPart.Address) Parts.push_back(&player.HumanoidRootPart);
                if (player.Head.Address)             Parts.push_back(&player.Head);
                if (player.Torso.Address)            Parts.push_back(&player.Torso);
                if (player.UpperTorso.Address)       Parts.push_back(&player.UpperTorso);
                if (player.LowerTorso.Address)       Parts.push_back(&player.LowerTorso);
            }
        }
        return Parts;
    }

    void run()
    {
        if (global::render.Address == 0 || global::model.Address == 0)
            return;

        visual_frame::begin();

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        dl->Flags |= ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedFill;

        // ── Snapshot (rate-limited) ────────────────────────────────────
        static double lastSnapshotTime = 0.0;
        static std::vector<sdk::player> Snapshot;
        static std::vector<sdk::player> SnapshotSwap;
        double now = ImGui::GetTime();
        if (now - lastSnapshotTime > 0.033) {
            {
                std::lock_guard<std::mutex> lock(cache::Mutex);
                SnapshotSwap = global::Player_Cache;
            }
            Snapshot.swap(SnapshotSwap);
            lastSnapshotTime = now;
        }

        if (Snapshot.empty())
            return;

        const auto& Local = global::LocalPlayer;
        const auto& LocalPlayerRef = Local;
        ImVec2 clipMin = dl->GetClipRectMin();
        ImVec2 clipMax = dl->GetClipRectMax();

        // ── Per-player rendering ───────────────────────────────────────
        for (auto& player : Snapshot)
        {
            if (!player.character.Address)
                continue;
            if (Local.character.Address && player.character.Address == Local.character.Address)
                continue;
            if (global::esp::Render_Distance > 0.f && player.Distance > global::esp::Render_Distance)
                continue;

            // ── Get head position ──────────────────────────────────────
            sdk::part HeadPart(player.Head.Address);
            sdk::primitive_data headData;
            bool hasHead = HeadPart.Address && HeadPart.get_primitive_data(headData);

            sdk::vector3 HeadPos{};
            if (hasHead) {
                HeadPos = headData.position;
            } else {
                sdk::part RootPart(player.HumanoidRootPart.Address);
                sdk::primitive_data rootData;
                if (RootPart.Address && RootPart.get_primitive_data(rootData)) {
                    HeadPos = rootData.position;
                    HeadPos.y += 1.5f;
                } else {
                    continue;
                }
            }

            auto HeadScreen = global::render.screen(HeadPos);
            if (!validscreen(HeadScreen.x, HeadScreen.y))
                continue;

            // ── Visibility ─────────────────────────────────────────────
            const bool PlayerVisible = !global::esp::VisibleCheck || visiblecheck(HeadPos, HeadScreen);
            const bool IsTarget = (global::aim::AimTarget.Address != 0 && player.character.Address == global::aim::AimTarget.Address)
                || (SilentCachedTarget.character.Address != 0 && player.character.Address == SilentCachedTarget.character.Address);

            // ── Project bones & compute bounding box ───────────────────
            auto Bones = esp::bone(player);
            if (Bones.empty()) continue;

            std::unordered_map<std::uintptr_t, ImVec2> projected;
            projected.reserve(Bones.size());

            float L = FLT_MAX, T = FLT_MAX, R = -FLT_MAX, B = -FLT_MAX;
            bool valid = false;

            for (auto* inst : Bones) {
                if (!inst || !inst->Address) continue;
                sdk::part p(inst->Address);
                sdk::primitive_data pd;
                if (!p.get_primitive_data(pd)) continue;
                auto sc = global::render.screen(pd.position);
                if (!validscreen(sc.x, sc.y)) continue;
                ImVec2 pt(sc.x, sc.y);
                projected[inst->Address] = pt;
                valid = true;
                L = min(L, pt.x);
                T = min(T, pt.y);
                R = max(R, pt.x);
                B = max(B, pt.y);
            }

            if (!valid || L >= R || T >= B) continue;

            // ── Bounding box padding ───────────────────────────────────
            float h = B - T;
            float w = R - L;
            T -= h * 0.12f;
            B += h * 0.05f;
            L -= w * 0.20f;
            R += w * 0.20f;

            // ── Clip rect cull ─────────────────────────────────────────
            if (R < clipMin.x || L > clipMax.x || B < clipMin.y || T > clipMax.y)
                continue;

            ImVec2 boxMin(L, T);
            ImVec2 boxMax(R, B);

            // ── Box color ──────────────────────────────────────────────
            ImU32 boxCol = col4_to_u32(global::esp::color::Box);
            if (global::esp::VisibleCheck)
                boxCol = PlayerVisible ? col4_to_u32(global::esp::color::Visible) : col4_to_u32(global::esp::color::NotVisible);
            if (IsTarget)
                boxCol = IM_COL32(255, 140, 0, 255);

            // ── Box ────────────────────────────────────────────────────
            if (global::esp::Box)
            {
                float bx = std::roundf(L);
                float by = std::roundf(T);
                float bw = std::roundf(R - L);
                float bh = std::roundf(B - T);
                ImVec2 rMin(bx, by);
                ImVec2 rMax(bx + bw, by + bh);

                if (global::esp::Box_Type == 0)
                {
                    // Full box
                    dl->AddRect(rMin, rMax, boxCol, 0.f, 0, 1.f);
                }
                else
                {
                    // Corner box
                    float len = ImClamp(ImMin(bw, bh) * 0.22f, 8.f, 40.f);
                    // top-left
                    dl->AddLine(rMin, ImVec2(rMin.x + len, rMin.y), boxCol, 1.f);
                    dl->AddLine(rMin, ImVec2(rMin.x, rMin.y + len), boxCol, 1.f);
                    // top-right
                    dl->AddLine(ImVec2(rMax.x, rMin.y), ImVec2(rMax.x - len, rMin.y), boxCol, 1.f);
                    dl->AddLine(ImVec2(rMax.x, rMin.y), ImVec2(rMax.x, rMin.y + len), boxCol, 1.f);
                    // bottom-left
                    dl->AddLine(ImVec2(rMin.x, rMax.y), ImVec2(rMin.x + len, rMax.y), boxCol, 1.f);
                    dl->AddLine(ImVec2(rMin.x, rMax.y), ImVec2(rMin.x, rMax.y - len), boxCol, 1.f);
                    // bottom-right
                    dl->AddLine(ImVec2(rMax.x, rMax.y), ImVec2(rMax.x - len, rMax.y), boxCol, 1.f);
                    dl->AddLine(ImVec2(rMax.x, rMax.y), ImVec2(rMax.x, rMax.y - len), boxCol, 1.f);
                }
            }

            // ── Head dot ───────────────────────────────────────────────
            if (global::esp::Chinese_Hat && HeadScreen.x > -0.5f && HeadScreen.y > -0.5f)
            {
                dl->AddCircleFilled(ImVec2(HeadScreen.x, HeadScreen.y), 3.f, boxCol, 12);
            }

            // ── Health bar ─────────────────────────────────────────────
            if (global::esp::Healthbar)
            {
                float ratio = (player.MaxHealth > 0.f) ? player.Health / player.MaxHealth : 0.f;
                ratio = ImClamp(ratio, 0.f, 1.f);

                float gap = (float)global::esp::gap;
                float thick = ImMax(2.f, (float)global::esp::Thickness + 1.f);
                float barX = L - gap - thick - 4.f;

                ImVec2 barMin(barX, T);
                ImVec2 barMax(barX + thick, B);

                // Background
                dl->AddRectFilled(barMin, barMax, IM_COL32(0, 0, 0, 140), thick * 0.5f);

                // Fill
                float fillH = (B - T - 2.f) * ratio;
                if (fillH > 0.f)
                {
                    ImVec2 fillMin(barX + 1.f, B - 1.f - fillH);
                    ImVec2 fillMax(barX + thick - 1.f, B - 1.f);
                    ImU32 hCol = health_color(ratio);
                    dl->AddRectFilled(fillMin, fillMax, hCol, (thick - 2.f) * 0.5f);
                }
            }

            // ── Health text ────────────────────────────────────────────
            if (global::esp::Health)
            {
                char buf[16];
                snprintf(buf, sizeof(buf), "%d", (int)player.Health);
                float textX = L - 4.f;
                if (global::esp::Healthbar)
                    textX -= (float)global::esp::Thickness + (float)global::esp::gap + 4.f;
                ImVec2 ts = ImGui::CalcTextSize(buf);
                ImVec2 tp(textX - ts.x, T - 2.f);
                draw_text_shadowed(dl, ImGui::GetFont(), ImGui::GetFontSize(), tp, col4_to_u32(global::esp::color::Health), buf);
            }

            // ── Name ───────────────────────────────────────────────────
            if (global::esp::name)
            {
                const char* nameText = nullptr;
                if (global::esp::Name_Type == 1)
                    nameText = player.Display_Name.c_str();
                else if (global::esp::Name_Type == 2)
                    nameText = player.name.c_str();
                else
                    nameText = player.name.c_str();

                ImVec2 ts = ImGui::CalcTextSize(nameText);
                ImVec2 tp(L + ((R - L) - ts.x) * 0.5f, T - ts.y - 4.f);
                draw_text_shadowed(dl, ImGui::GetFont(), ImGui::GetFontSize(), tp, col4_to_u32(global::esp::color::name), nameText);
            }

            // ── Distance ───────────────────────────────────────────────
            if (global::esp::Distance)
            {
                char buf[16];
                snprintf(buf, sizeof(buf), "%dm", (int)player.Distance);
                ImVec2 ts = ImGui::CalcTextSize(buf);
                ImVec2 tp(L + ((R - L) - ts.x) * 0.5f, B + 3.f);
                draw_text_shadowed(dl, ImGui::GetFont(), ImGui::GetFontSize(), tp, col4_to_u32(global::esp::color::Distance), buf);
            }

            // ── Tool ───────────────────────────────────────────────────
            if (global::esp::tool && !player.Tool_Name.empty())
            {
                std::string tool = "[" + player.Tool_Name + "]";
                ImVec2 ts = ImGui::CalcTextSize(tool.c_str());
                float offset = global::esp::Distance ? 16.f : 3.f;
                ImVec2 tp(L + ((R - L) - ts.x) * 0.5f, B + offset);
                draw_text_shadowed(dl, ImGui::GetFont(), ImGui::GetFontSize(), tp, col4_to_u32(global::esp::color::tool), tool.c_str());
            }

            // ── Rig type ───────────────────────────────────────────────
            if (global::esp::Rig_Type)
            {
                const char* rig = (player.Rig_Type == 1) ? "R15" : (player.Rig_Type == 0) ? "R6" : nullptr;
                if (rig) {
                    ImVec2 ts = ImGui::CalcTextSize(rig);
                    ImVec2 tp(R + 4.f, T + 2.f);
                    draw_text_shadowed(dl, ImGui::GetFont(), ImGui::GetFontSize(), tp, col4_to_u32(global::esp::color::Rig_Type), rig);
                }
            }

            // ── Skeleton ───────────────────────────────────────────────
            if (global::esp::Skeleton)
            {
                ImU32 skelCol = col4_to_u32(global::esp::color::Skeleton, 0.85f);

                auto draw_chain = [&](const ImVec2* pts, int count) {
                    if (count < 2) return;
                    for (int i = 0; i < count - 1; ++i) {
                        if (pts[i].x < -0.5f || pts[i + 1].x < -0.5f) continue;
                        dl->AddLine(pts[i], pts[i + 1], IM_COL32(0, 0, 0, 100), 2.f);
                        dl->AddLine(pts[i], pts[i + 1], skelCol, 1.f);
                    }
                };

                auto get_proj = [&](const sdk::instance& inst, ImVec2& out) -> bool {
                    if (!inst.Address) return false;
                    auto it = projected.find(inst.Address);
                    if (it == projected.end()) return false;
                    out = it->second;
                    return true;
                };

                if (player.UpperTorso.Address && player.LowerTorso.Address)
                {
                    // R15
                    ImVec2 pts[4];
                    int n;

                    // Spine
                    n = 0;
                    if (get_proj(player.Head, pts[n])) n++;
                    if (get_proj(player.UpperTorso, pts[n])) n++;
                    if (get_proj(player.LowerTorso, pts[n])) n++;
                    draw_chain(pts, n);

                    // Left arm
                    n = 0;
                    if (get_proj(player.UpperTorso, pts[n])) n++;
                    if (get_proj(player.LeftUpperArm, pts[n])) n++;
                    if (get_proj(player.LeftLowerArm, pts[n])) n++;
                    if (get_proj(player.LeftHand, pts[n])) n++;
                    draw_chain(pts, n);

                    // Right arm
                    n = 0;
                    if (get_proj(player.UpperTorso, pts[n])) n++;
                    if (get_proj(player.RightUpperArm, pts[n])) n++;
                    if (get_proj(player.RightLowerArm, pts[n])) n++;
                    if (get_proj(player.RightHand, pts[n])) n++;
                    draw_chain(pts, n);

                    // Left leg
                    n = 0;
                    if (get_proj(player.LowerTorso, pts[n])) n++;
                    if (get_proj(player.LeftUpperLeg, pts[n])) n++;
                    if (get_proj(player.LeftLowerLeg, pts[n])) n++;
                    if (get_proj(player.LeftFoot, pts[n])) n++;
                    draw_chain(pts, n);

                    // Right leg
                    n = 0;
                    if (get_proj(player.LowerTorso, pts[n])) n++;
                    if (get_proj(player.RightUpperLeg, pts[n])) n++;
                    if (get_proj(player.RightLowerLeg, pts[n])) n++;
                    if (get_proj(player.RightFoot, pts[n])) n++;
                    draw_chain(pts, n);
                }
                else if (player.Torso.Address && player.Head.Address)
                {
                    // R6
                    sdk::part TorsoPart(player.Torso.Address);
                    sdk::part HeadPart2(player.Head.Address);
                    sdk::primitive_data td, hd;
                    if (TorsoPart.get_primitive_data(td) && HeadPart2.get_primitive_data(hd)) {
                        const sdk::vector3& TorsoPos = td.position;
                        const sdk::vector3& TorsoSize = td.size;
                        const auto& TorsoRot = td.rotation;
                        const sdk::vector3& HeadPos2 = hd.position;
                        const sdk::vector3& HeadSize = hd.size;

                        sdk::vector3 ShoulderCenter = TorsoPos + TorsoRot * sdk::vector3{ 0, TorsoSize.y * 0.2f, 0 };
                        sdk::vector3 HipCenter = TorsoPos - TorsoRot * sdk::vector3{ 0, TorsoSize.y * 0.4f, 0 };
                        sdk::vector3 HeadBottom = HeadPos2 - sdk::vector3{ 0, HeadSize.y * 0.5f, 0 };

                        auto proj_vec = [&](const sdk::vector3& w, ImVec2& out) -> bool {
                            auto s = global::render.screen(w);
                            if (!validscreen(s.x, s.y)) return false;
                            out = ImVec2(std::roundf(s.x), std::roundf(s.y));
                            return true;
                        };

                        ImVec2 pts[4];
                        int n;

                        // Spine
                        n = 0;
                        if (proj_vec(HeadPos2, pts[n])) n++;
                        if (proj_vec(HeadBottom, pts[n])) n++;
                        if (proj_vec(ShoulderCenter, pts[n])) n++;
                        if (proj_vec(HipCenter, pts[n])) n++;
                        draw_chain(pts, n);

                        // Left arm
                        sdk::vector3 ShoulderLeft = ShoulderCenter + TorsoRot * sdk::vector3{ -TorsoSize.x * 0.5f, 0, 0 };
                        n = 0;
                        if (proj_vec(ShoulderCenter, pts[n])) n++;
                        if (proj_vec(ShoulderLeft, pts[n])) n++;
                        if (player.LeftArm.Address) {
                            sdk::part arm(player.LeftArm.Address);
                            sdk::primitive_data ap;
                            if (arm.get_primitive_data(ap)) {
                                sdk::vector3 armTop = ap.position + ap.rotation * sdk::vector3{ 0, ap.size.y * 0.2f, 0 };
                                sdk::vector3 armBot = ap.position - ap.rotation * sdk::vector3{ 0, ap.size.y * 0.5f, 0 };
                                if (proj_vec(armTop, pts[n])) n++;
                                if (proj_vec(armBot, pts[n])) n++;
                            }
                        }
                        draw_chain(pts, n);

                        // Right arm
                        sdk::vector3 ShoulderRight = ShoulderCenter + TorsoRot * sdk::vector3{ TorsoSize.x * 0.5f, 0, 0 };
                        n = 0;
                        if (proj_vec(ShoulderCenter, pts[n])) n++;
                        if (proj_vec(ShoulderRight, pts[n])) n++;
                        if (player.RightArm.Address) {
                            sdk::part arm(player.RightArm.Address);
                            sdk::primitive_data ap;
                            if (arm.get_primitive_data(ap)) {
                                sdk::vector3 armTop = ap.position + ap.rotation * sdk::vector3{ 0, ap.size.y * 0.2f, 0 };
                                sdk::vector3 armBot = ap.position - ap.rotation * sdk::vector3{ 0, ap.size.y * 0.5f, 0 };
                                if (proj_vec(armTop, pts[n])) n++;
                                if (proj_vec(armBot, pts[n])) n++;
                            }
                        }
                        draw_chain(pts, n);

                        // Left leg
                        n = 0;
                        if (proj_vec(HipCenter, pts[n])) n++;
                        if (player.LeftLeg.Address) {
                            sdk::part leg(player.LeftLeg.Address);
                            sdk::primitive_data lp;
                            if (leg.get_primitive_data(lp)) {
                                sdk::vector3 legTop = lp.position + lp.rotation * sdk::vector3{ 0, lp.size.y * 0.5f, 0 };
                                sdk::vector3 legBot = lp.position - lp.rotation * sdk::vector3{ 0, lp.size.y * 0.5f, 0 };
                                if (proj_vec(legTop, pts[n])) n++;
                                if (proj_vec(legBot, pts[n])) n++;
                            }
                        }
                        draw_chain(pts, n);

                        // Right leg
                        n = 0;
                        if (proj_vec(HipCenter, pts[n])) n++;
                        if (player.RightLeg.Address) {
                            sdk::part leg(player.RightLeg.Address);
                            sdk::primitive_data lp;
                            if (leg.get_primitive_data(lp)) {
                                sdk::vector3 legTop = lp.position + lp.rotation * sdk::vector3{ 0, lp.size.y * 0.5f, 0 };
                                sdk::vector3 legBot = lp.position - lp.rotation * sdk::vector3{ 0, lp.size.y * 0.5f, 0 };
                                if (proj_vec(legTop, pts[n])) n++;
                                if (proj_vec(legBot, pts[n])) n++;
                            }
                        }
                        draw_chain(pts, n);
                    }
                }
            }
        }

        // ── FOV circles ────────────────────────────────────────────────
        POINT Cursor{};
        bool HasCursor = false;
        if ((global::aim::DrawFov && global::aim::Enabled) || (global::silent::DrawFov && global::silent::Enabled))
            HasCursor = GetCursorPos(&Cursor);

        auto draw_fov = [&](bool show, float radius, const float color[4], bool fill, bool spin, int speed) {
            if (!show || !HasCursor) return;
            ImVec2 center((float)Cursor.x, (float)Cursor.y);
            ImU32 col = col4_to_u32(color);

            if (fill)
                dl->AddCircleFilled(center, radius, u32_alpha(col, 0.10f), 64);
            dl->AddCircle(center, radius, u32_alpha(col, 0.85f), 64, 1.5f);

            if (spin) {
                static float rotation = 0.f;
                rotation += ImGui::GetIO().DeltaTime * ImMax(1, speed) * 1.4f;
                const float arc = IM_PI * 0.34f;
                for (int i = 0; i < 3; i++) {
                    float start = rotation + i * IM_PI * 0.666f;
                    dl->PathArcTo(center, radius + 3.f, start, start + arc, 16);
                    dl->PathStroke(u32_alpha(col, 0.75f), false, 2.f);
                }
            }
        };

        draw_fov(global::aim::DrawFov && global::aim::Enabled,
            global::aim::FovSize, global::aim::FovColor,
            global::aim::FillFov, global::aim::FovSpin, global::aim::FovSpinSpeed);

        {
            float silentFov = effectivefov();
            draw_fov(global::silent::DrawFov && global::silent::Enabled,
                silentFov, global::silent::FovColor,
                global::silent::FillFov, global::silent::FovSpin, global::silent::FovSpinSpeed);
        }
    }
}
