#pragma once
#include <imgui/imgui.h>
#include "../widgets/controls.h"
#include "../../global.h"
#include "../../config.h"
#include "../core/avatar.h"
#include <vector>
#include <string>

// ========================================================================
// Menu page content — 5 tabs with industrial-minimal layout
// ========================================================================

namespace page {

    // ========================================================================
    // Tab 0 — Aimbot
    // ========================================================================
    inline void aimbot(float contentW, float contentH) {
        const float halfLeft = (contentW - theme::space::md) * 0.5f;

        // ---- Left card: Aimbot ----
        if (w::card::begin("##ab_main", { halfLeft, contentH }, "Aimbot")) {
            w::toggle("Master enable", &global::aim::Enabled);
            ImGui::SameLine(ImGui::GetContentRegionMax().x -
                w::bindwidth(global::aim::Aimbot_Key, w::ImKeyBindMode::Hold) - 4.f);
            w::bind("##ab_bind", &global::aim::Aimbot_Key, (w::ImKeyBindMode*)&global::aim::Aimbot_Mode);

            w::gap(theme::space::md);
            w::labelsection("Targeting");
            w::pill_toolbar("##aim_type", {"Memory","Mouse","Camera"}, &global::aim::Aimbot_type);
            w::gap(theme::space::xs);
            w::pill_toolbar("##aim_prio", {"Crosshair","Distance"}, &global::aim::TargetPriority);
            w::gap(theme::space::xs);
            w::pill_toolbar("##aim_part", {"Head","Torso","L.Torso"}, &global::aim::HitPart);
            w::gap(theme::space::sm);
            w::toggle("Knocked check", &global::aim::KnockedCheck);
            w::toggle("Visible check", &global::aim::VisibleCheck);
            w::toggle("Sticky aim", &global::aim::AimbotSticky);
            w::sliderint("Hit chance", &global::aim::HitChance, 0, 100);

            w::gap(theme::space::md);
            w::labelsection("Smoothing");
            w::toggle("Advanced smooth", &global::aim::SmoothAdvanced);
            if (global::aim::Aimbot_type == 0 || global::aim::Aimbot_type == 1) {
                if (global::aim::SmoothAdvanced) {
                    w::sliderfloat("Smooth X", &global::aim::mouse::Smoothing_X, 0.f, 12.f);
                    w::sliderfloat("Smooth Y", &global::aim::mouse::Smoothing_Y, 0.f, 12.f);
                } else {
                    float avg = (global::aim::mouse::Smoothing_X + global::aim::mouse::Smoothing_Y) * 0.5f;
                    if (w::sliderfloat("Smooth", &avg, 0.f, 12.f)) {
                        global::aim::mouse::Smoothing_X = avg;
                        global::aim::mouse::Smoothing_Y = avg;
                    }
                }
                w::sliderfloat("Sensitivity", &global::aim::mouse::Mouse_Sensitivty, 0.f, 5.f);
            } else {
                if (global::aim::SmoothAdvanced) {
                    w::sliderfloat("Smooth X", &global::aim::camera::Smoothing_X, 0.f, 12.f);
                    w::sliderfloat("Smooth Y", &global::aim::camera::Smoothing_Y, 0.f, 12.f);
                } else {
                    float avg = (global::aim::camera::Smoothing_X + global::aim::camera::Smoothing_Y) * 0.5f;
                    if (w::sliderfloat("Smooth", &avg, 0.f, 12.f)) {
                        global::aim::camera::Smoothing_X = avg;
                        global::aim::camera::Smoothing_Y = avg;
                    }
                }
            }

            w::gap(theme::space::md);
            w::labelsection("Field of view");
            w::togglecolor("Draw aimbot FOV", &global::aim::DrawFov, "##ab_fovc", global::aim::FovColor);
            w::sliderfloat("FOV size", &global::aim::FovSize, 1.f, 500.f);
            w::toggle("Spin FOV", &global::aim::FovSpin);
            if (global::aim::FovSpin)
                w::sliderint("Spin speed", &global::aim::FovSpinSpeed, 1, 5);
        }
        w::card::end();

        ImGui::SameLine(0.f, theme::space::md);

        // ---- Right card: Silent aim ----
        if (w::card::begin("##si_main", { halfLeft, contentH }, "Silent aim")) {
            w::toggle("Master enable", &global::silent::Enabled);
            ImGui::SameLine(ImGui::GetContentRegionMax().x -
                w::bindwidth(global::silent::Silent_Key, w::ImKeyBindMode::Hold) - 4.f);
            w::bind("##si_bind", &global::silent::Silent_Key, (w::ImKeyBindMode*)&global::silent::Silent_Mode);

            w::gap(theme::space::md);
            w::labelsection("Targeting");
            w::pill_toolbar("##si_prio", {"Crosshair","Distance"}, &global::silent::TargetPriority);
            w::gap(theme::space::xs);
            w::pill_toolbar("##si_part", {"Head","Torso","L.Torso"}, &global::silent::AimPart);
            w::gap(theme::space::sm);
            w::toggle("Knocked check", &global::silent::KnockedCheck);
            w::toggle("Visible check", &global::silent::VisibleCheck);
            w::toggle("Sticky aim", &global::silent::StickyAim);
            w::toggle("Spoof mouse", &global::silent::SpoofMouse);

            w::gap(theme::space::md);
            w::labelsection("Prediction");
            w::toggle("Prediction", &global::silent::Prediction);
            if (global::silent::Prediction) {
                w::toggle("Auto prediction", &global::silent::AutoPrediction);
                if (!global::silent::AutoPrediction) {
                    w::sliderfloat("Pred X", &global::silent::PredictionX, 0.f, 0.5f);
                    w::sliderfloat("Pred Y", &global::silent::PredictionY, 0.f, 0.5f);
                    w::sliderfloat("Pred Z", &global::silent::PredictionZ, 0.f, 0.5f);
                }
            }

            w::gap(theme::space::md);
            w::labelsection("Field of view");
            w::togglecolor("Draw silent FOV", &global::silent::DrawFov, "##si_fovc", global::silent::FovColor);
        }
        w::card::end();
    }

    // ========================================================================
    // Tab 1 — Visuals
    // ========================================================================
    inline void visuals(float contentW, float contentH) {
        const float leftW = (contentW - theme::kRightPanelW - theme::space::md);
        const float halfLeft = (leftW - theme::space::md) * 0.5f;

        // ---- Left card: ESP toggles ----
        if (w::card::begin("##esp_toggles", { halfLeft, contentH }, "ESP")) {
            w::toggle("Master enable", &global::esp::Enabled);
            w::gap(theme::space::md);

            w::labelsection("Box");
            w::toggle("Box ESP", &global::esp::Box);
            if (global::esp::Box) {
                w::pill_toolbar("##box_type", {"Bounding","Corner"}, &global::esp::Box_Type);
                w::gap(theme::space::xs);
                w::toggle("Box fill", &global::esp::Box_Fill);
                if (global::esp::Box_Fill)
                    w::dualcolor("##boxfill_top", global::esp::color::BoxFill_Top,
                        "##boxfill_bot", global::esp::color::BoxFill_Bottom);
                w::toggle("Gradient fill", &global::esp::Box_Fill_Gradient);
            }

            w::gap(theme::space::md);
            w::labelsection("Health");
            w::toggle("Health bar", &global::esp::Healthbar);
            if (global::esp::Healthbar) {
                w::pill_toolbar("##hb_type", {"Static","Gradient"}, &global::esp::Healthbar_Type);
                w::sliderint("Bar gap", &global::esp::gap, 1, 5);
                w::sliderint("Bar thickness", &global::esp::Thickness, 1, 5);
            }
            w::toggle("Health text", &global::esp::Health);

            w::gap(theme::space::md);
            w::labelsection("Labels");
            w::toggle("Name", &global::esp::name);
            if (global::esp::name)
                w::combo("Name format", &global::esp::Name_Type,
                    {"Name","Display","Both"});
            w::toggle("Distance", &global::esp::Distance);
            w::toggle("Rig type", &global::esp::Rig_Type);
            w::toggle("Tool", &global::esp::tool);

            w::gap(theme::space::md);
            w::labelsection("3D");
            w::toggle("Skeleton", &global::esp::Skeleton);
            w::toggle("Trails", &global::esp::Trails);
            w::toggle("Chinese hats", &global::esp::Chinese_Hat);
            w::toggle("Aim lines", &global::esp::aimline);
            w::toggle("Chams", &global::esp::Chams);
            if (global::esp::Chams)
                w::dualcolor("##chams_fill", global::esp::color::Chams,
                    "##chams_out", global::esp::color::ChamsOutline);
        }
        w::card::end();

        ImGui::SameLine(0.f, theme::space::md);

        // ---- Right card: Options + colors ----
        if (w::card::begin("##vis_opts", { halfLeft, contentH }, "Options")) {
            w::labelsection("General");
            w::toggle("Exclude team", &global::setting::Team_Check);
            w::toggle("Exclude client", &global::setting::Client_Check);

            w::gap(theme::space::md);
            w::labelsection("Rendering");
            w::sliderfloat("Render distance", &global::esp::Render_Distance, 0.f, 1000.f);
            w::toggle("Visible check", &global::esp::VisibleCheck);

            w::gap(theme::space::md);
            w::labelsection("Colors");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Box");
            w::color4("##box_col", global::esp::color::Box);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Visible");
            w::color4("##vis_col", global::esp::color::Visible);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Not visible");
            w::color4("##nvis_col", global::esp::color::NotVisible);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Skeleton");
            w::color4("##skel_col", global::esp::color::Skeleton);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Trails");
            w::color4("##trail_col", global::esp::color::Trails);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Aim line");
            w::color4("##aimline_col", global::esp::color::aimline);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Name");
            w::color4("##name_col", global::esp::color::name);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Distance");
            w::color4("##dist_col", global::esp::color::Distance);
        }
        w::card::end();
    }

    // ========================================================================
    // Tab 2 — World
    // ========================================================================
    inline void world(float contentW, float contentH) {
        if (w::card::begin("##wld", { contentW, contentH }, "World")) {
            w::labelsection("Skybox");
            w::toggle("Skybox changer", &global::world::Skybox);
            if (global::world::Skybox) {
                w::combo("Preset", &global::world::Skybox_Type, {
                    "Miserable", "Space", "Pink sky", "Minecraft", "Night cloudy",
                    "Sparkling night", "Winterness", "Dark crimson", "Nebula",
                    "Tropical", "Green sky" });
                w::toggle("Rotation", &global::world::Rotate);
                if (global::world::Rotate)
                    w::sliderfloat("Rotate speed", &global::world::Skybox_Rotate_Speed, 0.f, 5.f);
            }

            w::gap(theme::space::lg);
            w::labelsection("Lighting");
            w::togglecolor("Atmosphere", &global::world::Ambience,
                "##atmc", global::world::color::Ambience);
            w::toggle("Fog", &global::world::Fog);
            if (global::world::Fog) {
                ImGui::SameLine(ImGui::GetContentRegionMax().x - 22.f);
                w::color4("##fogc", global::world::color::Fog);
                w::sliderfloat("Fog distance", &global::world::Fog_Distance, 0.f, 1000.f);
            }
            w::toggle("Brightness", &global::world::Brightness);
            if (global::world::Brightness)
                w::sliderfloat("Brightness", &global::world::BrightnessI, 0.f, 10.f);
            w::toggle("Exposure", &global::world::Exposure);
            if (global::world::Exposure)
                w::sliderfloat("Exposure", &global::world::ExposureI, -3.f, 3.f);

            w::gap(theme::space::lg);
            w::labelsection("Camera");
            w::toggle("Custom FOV", &global::world::FOV);
            if (global::world::FOV)
                w::sliderfloat("FOV", &global::world::FOV_Distance, 70.f, 120.f);
        }
        w::card::end();
    }

    // ========================================================================
    // Tab 3 — Misc
    // ========================================================================
    inline void misc(float contentW, float contentH) {
        const float halfW = (contentW - theme::space::md) * 0.5f;

        // ---- Left: Exploits ----
        if (w::card::begin("##exploits", { halfW, contentH }, "Exploits")) {
            w::toggle("Fly", &global::misc::fly);
            ImGui::SameLine(ImGui::GetContentRegionMax().x -
                w::bindwidth(global::misc::Fly_Key, w::ImKeyBindMode::Hold) - 4.f);
            w::bind("##fly_bind", &global::misc::Fly_Key, (w::ImKeyBindMode*)&global::misc::Fly_Mode);
            if (global::misc::fly)
                w::sliderfloat("Fly speed", &global::misc::Fly_Speed, 0.f, 200.f);

            w::gap(theme::space::md);
            w::labelsection("Adjustments");
            w::toggle("Walkspeed", &global::misc::walkspeed);
            if (global::misc::walkspeed)
                w::sliderfloat("Walkspeed", &global::misc::Walkspeed_Speed, 1.f, 500.f);
            w::toggle("Hitbox expander", &global::misc::hitbox);
            if (global::misc::hitbox) {
                w::sliderfloat("Size X", &global::misc::Hitbox_Size_X, 1.f, 50.f);
                w::sliderfloat("Size Y", &global::misc::Hitbox_Size_Y, 1.f, 50.f);
                w::sliderfloat("Size Z", &global::misc::Hitbox_Size_Z, 1.f, 50.f);
            }
        }
        w::card::end();

        ImGui::SameLine(0.f, theme::space::md);

        // ---- Right: Config ----
        if (w::card::begin("##cfg", { halfW, contentH }, "Config")) {
            static std::vector<std::string> configs;
            static int configIdx = -1;
            static char newNameBuf[128] = {0};

            config::refresh(configs);

            w::labelsection("Saved configs");
            if (configs.empty()) {
                w::gap(theme::space::sm);
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()),
                    "No configs found");
            } else {
                std::vector<const char*> items;
                for (auto& c : configs) items.push_back(c.c_str());
                w::combo("Select config", &configIdx, items);
                if (configIdx < 0 || configIdx >= (int)configs.size())
                    configIdx = -1;
            }

            w::gap(theme::space::md);
            if (configIdx >= 0 && configIdx < (int)configs.size()) {
                if (w::accent_button("Load", -1.f, 24.f))
                    config::load(configs[configIdx]);
                w::gap(theme::space::xs);
                if (w::accent_button("Save", -1.f, 24.f))
                    config::save(configs[configIdx]);
                w::gap(theme::space::xs);
                if (w::danger_button("Delete", -1.f, 24.f)) {
                    config::remove(configs[configIdx]);
                    configIdx = -1;
                }
            }

            w::gap(theme::space::md);
            w::labelsection("Create new");
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::InputText("##newcfg", newNameBuf, sizeof(newNameBuf));
            ImGui::PopItemWidth();
            if (w::accent_button("Create", -1.f, 24.f)) {
                if (newNameBuf[0]) {
                    config::save(newNameBuf);
                    newNameBuf[0] = '\0';
                }
            }
        }
        w::card::end();
    }

    // ========================================================================
    // Tab 4 — Settings
    // ========================================================================
    inline void settings(float contentW, float contentH) {
        const float leftW = (contentW - theme::space::md) * 0.5f;

        // ---- Left: General ----
        if (w::card::begin("##s_gen", { leftW, contentH }, "General")) {
            w::labelsection("Menu");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Menu key");
            ImGui::SameLine(ImGui::GetContentRegionMax().x - 60.f);
            w::keyselect("##menukey", &global::setting::Menu_Key);

            w::gap(theme::space::md);
            w::labelsection("Display");
            w::toggle("Compact UI", &global::setting::Compact_UI);
            w::pill_toolbar("##perf", {"60fps","144fps","Unlocked"}, &global::setting::Performance_Mode);
            w::gap(theme::space::xs);
            w::toggle("Streamproof", &global::setting::Streamproof);

            w::gap(theme::space::md);
            w::labelsection("Sound");
            w::toggle("Sound effects", &global::setting::Sound_Enabled);
            if (global::setting::Sound_Enabled)
                w::sliderfloat("Volume", &global::setting::Sound_Volume, 0.f, 1.f);

            w::gap(theme::space::md);
            w::labelsection("Overlay");
            w::toggle("Watermark", &global::overlay::watermark);
            w::toggle("Hotkeys panel", &global::overlay::hotkey);
            if (global::overlay::hotkey) {
                w::toggle("  Aimbot", &global::overlay::Hotkey_Aimbot);
                w::toggle("  Silent", &global::overlay::Hotkey_Silent);
                w::toggle("  Fly", &global::overlay::Hotkey_Fly);
                w::toggle("  Blade spam", &global::overlay::Hotkey_BladeBallSpam);
                w::toggle("  Walkspeed", &global::overlay::Hotkey_Walkspeed);
                w::toggle("  Hitbox", &global::overlay::Hotkey_HitboxExpander);
            }
            w::toggle("Radar", &global::overlay::radar);
            if (global::overlay::radar) {
                w::pill_toolbar("##radar_shape", {"Circle","Square"}, &global::overlay::Radar_Shape);
                w::sliderfloat("Zoom", &global::overlay::Radar_Zoom, 0.3f, 4.f);
                w::sliderfloat("Size", &global::overlay::Radar_Size, 130.f, 280.f);
                w::toggle("Rotate with cam", &global::overlay::Radar_Rotate);
            }
            w::toggle("Aim warning", &global::overlay::AimWarning);
            if (global::overlay::AimWarning)
                w::sliderfloat("Detection range", &global::overlay::AimView_MaxLength, 50.f, 1000.f);
        }
        w::card::end();

        ImGui::SameLine(0.f, theme::space::md);

        // ---- Right: Account ----
        if (w::card::begin("##s_about", { leftW, contentH }, "Account")) {
            w::labelsection("User");

            ImDrawList* dl = ImGui::GetWindowDrawList();
            const ImVec2 pillPos = ImGui::GetCursorScreenPos();
            float pillW = ImGui::GetContentRegionAvail().x;
            float pillH = 64.f;

            dl->AddRectFilled(pillPos, pillPos + ImVec2(pillW, pillH),
                IM_COL32(20, 10, 10, 255), 12.f);
            dl->AddRect(pillPos, pillPos + ImVec2(pillW, pillH),
                IM_COL32(224, 48, 64, 25), 12.f, 0, 1.f);

            float avatarR = 22.f;
            ImVec2 avCenter = pillPos + ImVec2(12.f + avatarR, pillH * 0.5f);

            ID3D11ShaderResourceView* avatarSrv = avatar::get_srv();
            if (avatarSrv && avatar::is_loaded()) {
                dl->AddImageRounded(avatarSrv,
                    pillPos + ImVec2(12.f, 10.f),
                    pillPos + ImVec2(60.f, 58.f),
                    ImVec2(0, 0), ImVec2(1, 1),
                    IM_COL32(255, 255, 255, 255), 24.f);
                dl->AddCircle(avCenter, avatarR + 1.f,
                    IM_COL32(224, 48, 64, 50), 32, 1.5f);
            } else {
                dl->AddCircleFilled(avCenter, avatarR, IM_COL32(30, 12, 12, 255), 32);
                dl->AddCircle(avCenter, avatarR, IM_COL32(224, 48, 64, 50), 32, 1.5f);
                static const char* uname = []() {
                    static char buf[128];
                    DWORD len = GetEnvironmentVariableA("USERNAME", buf, sizeof(buf));
                    return (len && len < sizeof(buf)) ? buf : "?";
                }();
                char init[2] = { (char)toupper(uname[0]), '\0' };
                ImVec2 initSz = ImGui::CalcTextSize(init);
                dl->AddText(font::label(), 14.f,
                    { avCenter.x - initSz.x * 0.5f, avCenter.y - initSz.y * 0.5f },
                    theme::col_accent(), init);
            }

            static std::string username_str = avatar::get_username();
            dl->AddText(font::label(), 13.f,
                pillPos + ImVec2(70.f, 14.f),
                theme::col_text(), username_str.c_str());

            ImGui::SetCursorScreenPos(pillPos + ImVec2(0.f, pillH + 8.f));

            w::gap(theme::space::md);
            w::labelsection("Keybinds");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Aimbot");
            ImGui::SameLine(ImGui::GetContentRegionMax().x - 80.f);
            w::keyselect("##qk_aim", &global::aim::Aimbot_Key);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Silent");
            ImGui::SameLine(ImGui::GetContentRegionMax().x - 80.f);
            w::keyselect("##qk_sil", &global::silent::Silent_Key);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Fly");
            ImGui::SameLine(ImGui::GetContentRegionMax().x - 80.f);
            w::keyselect("##qk_fly", &global::misc::Fly_Key);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Menu");
            ImGui::SameLine(ImGui::GetContentRegionMax().x - 80.f);
            w::keyselect("##qk_menu", &global::setting::Menu_Key);
        }
        w::card::end();
    }

    // ========================================================================
    // Tab 5 — Phantom Forces
    // ========================================================================
    inline void pf(float contentW, float contentH) {
        const float leftW = (contentW - theme::space::md) * 0.5f;

        // ---- Left: Silent aim ----
        if (w::card::begin("##pf_silent", { leftW, contentH }, "Silent aim")) {
            w::toggle("Enable PF mode", &global::pf::Enabled);
            w::gap(theme::space::md);
            w::labelsection("Silent aim");
            w::toggle("Silent aim", &global::pf::SilentAim);
            w::gap(theme::space::sm);
            ImGui::TextWrapped("Auto-aims at closest target on left-click.");
            ImGui::TextWrapped("Camera returns to original position on release.");
        }
        w::card::end();

        ImGui::SameLine(0.f, theme::space::md);

        // ---- Right: ESP ----
        if (w::card::begin("##pf_esp", { leftW, contentH }, "ESP")) {
            w::toggle("ESP", &global::pf::Esp);
            w::gap(theme::space::md);

            w::labelsection("Render");
            w::toggle("Bounding box", &global::pf::Box);
            w::gap(theme::space::xs);
            w::toggle("Skeleton", &global::pf::Skeleton);
            w::gap(theme::space::xs);
            w::toggle("Health bar", &global::pf::HealthBar);

            w::gap(theme::space::md);
            w::labelsection("Colors");
            w::color4("##pf_box_col", global::pf::BoxColor);
            w::color4("##pf_skel_col", global::pf::SkeletonColor);
            w::color4("##pf_heal_col", global::pf::HealthColor);
        }
        w::card::end();
    }

} // namespace page
