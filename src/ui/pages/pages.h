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
    // Tab 0 — AIMBOT
    // ========================================================================
    inline void aimbot(float contentW, float contentH) {
        const float leftW = (contentW - theme::kRightPanelW - theme::space::md);
        const float halfLeft = (leftW - theme::space::md) * 0.5f;

        // ---- Left card: AIMBOT ----
        if (w::card::begin("##ab_main", { halfLeft, contentH }, "AIMBOT")) {
            w::toggle("Master Enable", &global::aim::Enabled);
            ImGui::SameLine(ImGui::GetContentRegionMax().x -
                w::bindwidth(global::aim::Aimbot_Key, w::ImKeyBindMode::Hold) - 4.f);
            w::bind("##ab_bind", &global::aim::Aimbot_Key, (w::ImKeyBindMode*)&global::aim::Aimbot_Mode);

            w::gap(theme::space::sm);
            w::labelsection("TARGETING");
            w::pill_toolbar("##aim_type", {"Free Aim","Mouse Aim","Cam Lock"}, &global::aim::Aimbot_type);
            w::gap(theme::space::xs);
            w::pill_toolbar("##aim_prio", {"Crosshair","Distance"}, &global::aim::TargetPriority);
            w::gap(theme::space::xs);
            w::pill_toolbar("##aim_part", {"Head","Torso","L.Torso"}, &global::aim::HitPart);
            w::gap(theme::space::xs);
            w::toggle("Knocked Check", &global::aim::KnockedCheck);
            w::toggle("Visible Check", &global::aim::VisibleCheck);
            w::toggle("Sticky Aim", &global::aim::AimbotSticky);
            w::sliderint("Hit Chance", &global::aim::HitChance, 0, 100);

            w::gap(theme::space::sm);
            w::labelsection("SMOOTHING");
            w::toggle("Advanced Smooth", &global::aim::SmoothAdvanced);
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

            w::gap(theme::space::sm);
            w::labelsection("TRIGGER BOT");
            w::toggle("Trigger Bot", &global::aim::TriggerBot);
            if (global::aim::TriggerBot) {
                w::sliderfloat("TB Radius", &global::aim::TriggerRadius, 1.f, 25.f);
                w::sliderint("TB Delay ms", &global::aim::TriggerDelayMs, 0, 250);
            }
        }
        w::card::end();

        ImGui::SameLine(0.f, theme::space::md);

        // ---- Right card: SILENT AIM ----
        if (w::card::begin("##si_main", { halfLeft, contentH }, "SILENT AIM")) {
            w::toggle("Master Enable", &global::silent::Enabled);
            ImGui::SameLine(ImGui::GetContentRegionMax().x -
                w::bindwidth(global::silent::Silent_Key, w::ImKeyBindMode::Hold) - 4.f);
            w::bind("##si_bind", &global::silent::Silent_Key, (w::ImKeyBindMode*)&global::silent::Silent_Mode);

            w::gap(theme::space::sm);
            w::labelsection("TARGETING");
            w::pill_toolbar("##si_prio", {"Crosshair","Distance"}, &global::silent::TargetPriority);
            w::gap(theme::space::xs);
            w::pill_toolbar("##si_part", {"Head","Torso","L.Torso"}, &global::silent::AimPart);
            w::gap(theme::space::xs);
            w::toggle("Knocked Check", &global::silent::KnockedCheck);
            w::toggle("Visible Check", &global::silent::VisibleCheck);
            w::toggle("Sticky Aim", &global::silent::StickyAim);
            w::toggle("Spoof Mouse", &global::silent::SpoofMouse);

            w::gap(theme::space::sm);
            w::labelsection("PREDICTION");
            w::toggle("Prediction", &global::silent::Prediction);
            if (global::silent::Prediction) {
                w::toggle("Auto Prediction", &global::silent::AutoPrediction);
                if (!global::silent::AutoPrediction) {
                    w::sliderfloat("Pred X", &global::silent::PredictionX, 0.f, 0.5f);
                    w::sliderfloat("Pred Y", &global::silent::PredictionY, 0.f, 0.5f);
                    w::sliderfloat("Pred Z", &global::silent::PredictionZ, 0.f, 0.5f);
                }
            }
        }
        w::card::end();

        // ---- Right panel: FOV + STATS ----
        ImGui::SameLine(0.f, theme::space::md);

        if (w::card::begin("##fov_panel", { theme::kRightPanelW, contentH }, "FOV & STATS")) {
            w::labelsection("FOV");
            w::togglecolor("Draw Aimbot FOV", &global::aim::DrawFov, "##ab_fovc", global::aim::FovColor);
            w::togglecolor("Draw Silent FOV", &global::silent::DrawFov, "##si_fovc", global::silent::FovColor);
            w::sliderfloat("FOV Size", &global::aim::FovSize, 1.f, 500.f);
            w::toggle("Spin FOV", &global::aim::FovSpin);
            if (global::aim::FovSpin)
                w::sliderint("Spin Speed", &global::aim::FovSpinSpeed, 1, 5);

            w::gap(theme::space::md);
            w::labelsection("STATS");
            // Stats display as a table
            const float statW = (theme::kRightPanelW - 42.f) * 0.5f;
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Scanned");
            ImGui::SameLine(statW + 10.f);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_accent()), "1,247");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Hit Rate");
            ImGui::SameLine(statW + 10.f);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_accent()), "98.2%%");
        }
        w::card::end();
    }

    // ========================================================================
    // Tab 1 — VISUALS
    // ========================================================================
    inline void visuals(float contentW, float contentH) {
        const float leftW = (contentW - theme::kRightPanelW - theme::space::md);
        const float halfLeft = (leftW - theme::space::md) * 0.5f;

        // ---- Left card: ESP TOGGLES ----
        if (w::card::begin("##esp_toggles", { halfLeft, contentH }, "ESP TOGGLES")) {
            w::toggle("Master Enable", &global::esp::Enabled);
            w::gap(theme::space::sm);

            w::labelsection("BOX");
            w::toggle("Box ESP", &global::esp::Box);
            if (global::esp::Box) {
                w::pill_toolbar("##box_type", {"Bounding","Corner"}, &global::esp::Box_Type);
                w::gap(theme::space::xs);
                w::toggle("Box Fill", &global::esp::Box_Fill);
                if (global::esp::Box_Fill)
                    w::dualcolor("##boxfill_top", global::esp::color::BoxFill_Top,
                        "##boxfill_bot", global::esp::color::BoxFill_Bottom);
                w::toggle("Gradient Fill", &global::esp::Box_Fill_Gradient);
            }

            w::gap(theme::space::sm);
            w::labelsection("HEALTH");
            w::toggle("Health Bar", &global::esp::Healthbar);
            if (global::esp::Healthbar) {
                w::pill_toolbar("##hb_type", {"Static","Gradient"}, &global::esp::Healthbar_Type);
                w::sliderint("Bar Gap", &global::esp::gap, 1, 5);
                w::sliderint("Bar Thickness", &global::esp::Thickness, 1, 5);
            }
            w::toggle("Health Text", &global::esp::Health);

            w::gap(theme::space::sm);
            w::labelsection("LABELS");
            w::toggle("Name", &global::esp::name);
            if (global::esp::name)
                w::combo("Name Format", &global::esp::Name_Type,
                    {"Name","Display","Both"});
            w::toggle("Distance", &global::esp::Distance);
            w::toggle("Rig Type", &global::esp::Rig_Type);
            w::toggle("Tool", &global::esp::tool);

            w::gap(theme::space::sm);
            w::labelsection("3D");
            w::toggle("Skeleton", &global::esp::Skeleton);
            w::toggle("Trails", &global::esp::Trails);
            w::toggle("Chinese Hats", &global::esp::Chinese_Hat);
            w::toggle("Aim Lines", &global::esp::aimline);
            w::toggle("Chams", &global::esp::Chams);
            if (global::esp::Chams)
                w::dualcolor("##chams_fill", global::esp::color::Chams,
                    "##chams_out", global::esp::color::ChamsOutline);
        }
        w::card::end();

        ImGui::SameLine(0.f, theme::space::md);

        // ---- Right card: OPTIONS + COLORS ----
        if (w::card::begin("##vis_opts", { halfLeft, contentH }, "OPTIONS")) {
            w::labelsection("GENERAL");
            w::toggle("Exclude Team", &global::setting::Team_Check);
            w::toggle("Exclude Client", &global::setting::Client_Check);

            w::gap(theme::space::sm);
            w::labelsection("RENDERING");
            w::sliderfloat("Render Distance", &global::esp::Render_Distance, 0.f, 1000.f);

            w::gap(theme::space::sm);
            w::labelsection("COLORS");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Visible");
            w::color4("##vis_col", global::esp::color::Visible);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Not Visible");
            w::color4("##nvis_col", global::esp::color::NotVisible);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Skeleton");
            w::color4("##skel_col", global::esp::color::Skeleton);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Trails");
            w::color4("##trail_col", global::esp::color::Trails);

            w::gap(theme::space::sm);
            w::labelsection("SESSION");
            const float statW = (halfLeft * 0.5f);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Players");
            ImGui::SameLine(statW + 10.f);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_accent()), "247");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "FPS");
            ImGui::SameLine(statW + 10.f);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_accent()), "143");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Latency");
            ImGui::SameLine(statW + 10.f);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_accent()), "12ms");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Uptime");
            ImGui::SameLine(statW + 10.f);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_accent()), "99%%");
        }
        w::card::end();

        // ---- Right panel (240px): color swatches + session ----
        ImGui::SameLine(0.f, theme::space::md);

        if (w::card::begin("##vis_right", { theme::kRightPanelW, contentH }, "VISUAL CONFIG")) {
            w::labelsection("PLAYER COLORS");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Visible");
            ImGui::SameLine(ImGui::GetContentRegionMax().x - 22.f);
            w::color4("##visc", global::esp::color::Visible);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Not Visible");
            ImGui::SameLine(ImGui::GetContentRegionMax().x - 22.f);
            w::color4("##nvc", global::esp::color::NotVisible);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Skeleton");
            ImGui::SameLine(ImGui::GetContentRegionMax().x - 22.f);
            w::color4("##skc", global::esp::color::Skeleton);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Trails");
            ImGui::SameLine(ImGui::GetContentRegionMax().x - 22.f);
            w::color4("##trc", global::esp::color::Trails);

            w::gap(theme::space::md);
            w::labelsection("SESSION");
            const float statW2 = (theme::kRightPanelW - 42.f) * 0.5f;
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Players");
            ImGui::SameLine(statW2 + 10.f);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_accent()), "247");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "FPS");
            ImGui::SameLine(statW2 + 10.f);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_accent()), "143");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Latency");
            ImGui::SameLine(statW2 + 10.f);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_accent()), "12ms");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Uptime");
            ImGui::SameLine(statW2 + 10.f);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_accent()), "99%%");
        }
        w::card::end();
    }

    // ========================================================================
    // Tab 2 — WORLD
    // ========================================================================
    inline void world(float contentW, float contentH) {
        // Single wide card
        if (w::card::begin("##wld", { contentW, contentH }, "WORLD MANIPULATION")) {
            w::labelsection("SKYBOX");
            w::toggle("Skybox Changer", &global::world::Skybox);
            if (global::world::Skybox) {
                w::combo("Preset", &global::world::Skybox_Type, {
                    "Miserable", "Space", "Pink Sky", "Minecraft", "Night Cloudy",
                    "Sparkling Night", "Winterness", "Dark Crimson", "Nebula",
                    "Tropical", "Green Sky" });
                w::toggle("Rotation", &global::world::Rotate);
                if (global::world::Rotate)
                    w::sliderfloat("Rotate Speed", &global::world::Skybox_Rotate_Speed, 0.f, 5.f);
            }

            w::gap(theme::space::md);
            w::labelsection("LIGHTING");
            w::togglecolor("Atmosphere", &global::world::Ambience,
                "##atmc", global::world::color::Ambience);
            w::toggle("Fog", &global::world::Fog);
            if (global::world::Fog) {
                ImGui::SameLine(ImGui::GetContentRegionMax().x - 22.f);
                w::color4("##fogc", global::world::color::Fog);
                w::sliderfloat("Fog Distance", &global::world::Fog_Distance, 0.f, 1000.f);
            }
            w::toggle("Brightness", &global::world::Brightness);
            if (global::world::Brightness)
                w::sliderfloat("Brightness", &global::world::BrightnessI, 0.f, 10.f);
            w::toggle("Exposure", &global::world::Exposure);
            if (global::world::Exposure)
                w::sliderfloat("Exposure", &global::world::ExposureI, -3.f, 3.f);

            w::gap(theme::space::md);
            w::labelsection("CAMERA");
            w::toggle("Custom FOV", &global::world::FOV);
            if (global::world::FOV)
                w::sliderfloat("FOV", &global::world::FOV_Distance, 70.f, 120.f);
        }
        w::card::end();
    }

    // ========================================================================
    // Tab 3 — MISC
    // ========================================================================
    inline void misc(float contentW, float contentH) {
        const float halfW = (contentW - theme::space::md) * 0.5f;

        // ---- Left: EXPLOITS ----
        if (w::card::begin("##exploits", { halfW, contentH }, "EXPLOITS")) {
            w::toggle("Fly", &global::misc::fly);
            ImGui::SameLine(ImGui::GetContentRegionMax().x -
                w::bindwidth(global::misc::Fly_Key, w::ImKeyBindMode::Hold) - 4.f);
            w::bind("##fly_bind", &global::misc::Fly_Key, (w::ImKeyBindMode*)&global::misc::Fly_Mode);
            if (global::misc::fly)
                w::sliderfloat("Fly Speed", &global::misc::Fly_Speed, 0.f, 200.f);

            w::gap(theme::space::sm);
            w::labelsection("ADJUSTMENTS");
            w::toggle("Walkspeed", &global::misc::walkspeed);
            if (global::misc::walkspeed)
                w::sliderfloat("Walkspeed", &global::misc::Walkspeed_Speed, 1.f, 500.f);
            w::toggle("Hitbox Expander", &global::misc::hitbox);
            if (global::misc::hitbox) {
                w::sliderfloat("Size X", &global::misc::Hitbox_Size_X, 1.f, 50.f);
                w::sliderfloat("Size Y", &global::misc::Hitbox_Size_Y, 1.f, 50.f);
                w::sliderfloat("Size Z", &global::misc::Hitbox_Size_Z, 1.f, 50.f);
            }
        }
        w::card::end();

        ImGui::SameLine(0.f, theme::space::md);

        // ---- Right: CONFIG ----
        if (w::card::begin("##cfg", { halfW, contentH }, "CONFIG")) {
            static std::vector<std::string> configs;
            static int configIdx = -1;
            static char newNameBuf[128] = {0};

            config::refresh(configs);

            w::labelsection("SAVED CONFIGS");
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

            w::gap(theme::space::sm);
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

            w::gap(theme::space::sm);
            w::labelsection("CREATE NEW");
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
    // Tab 4 — SETTINGS
    // ========================================================================
    inline void settings(float contentW, float contentH) {
        const float leftW = (contentW - theme::space::md) * 0.5f;

        // ---- Left: GENERAL ----
        if (w::card::begin("##s_gen", { leftW, contentH }, "GENERAL")) {
            w::labelsection("MENU");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Menu Key");
            ImGui::SameLine(ImGui::GetContentRegionMax().x - 60.f);
            w::keyselect("##menukey", &global::setting::Menu_Key);

            w::gap(theme::space::sm);
            w::labelsection("DISPLAY");
            w::toggle("Compact UI", &global::setting::Compact_UI);
            w::pill_toolbar("##perf", {"60fps","144fps","Unlocked"}, &global::setting::Performance_Mode);
            w::gap(theme::space::xs);
            w::toggle("Streamproof", &global::setting::Streamproof);

            w::gap(theme::space::sm);
            w::labelsection("SOUND");
            w::toggle("Sound Effects", &global::setting::Sound_Enabled);
            if (global::setting::Sound_Enabled)
                w::sliderfloat("Volume", &global::setting::Sound_Volume, 0.f, 1.f);

            w::gap(theme::space::sm);
            w::labelsection("OVERLAY");
            w::toggle("Watermark", &global::overlay::watermark);
            w::toggle("Hotkeys Panel", &global::overlay::hotkey);
            if (global::overlay::hotkey) {
                w::toggle("  Aimbot", &global::overlay::Hotkey_Aimbot);
                w::toggle("  Silent", &global::overlay::Hotkey_Silent);
                w::toggle("  Fly", &global::overlay::Hotkey_Fly);
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
            w::toggle("Aim Warning", &global::overlay::AimWarning);
            if (global::overlay::AimWarning)
                w::sliderfloat("Detection Range", &global::overlay::AimView_MaxLength, 50.f, 1000.f);
        }
        w::card::end();

        ImGui::SameLine(0.f, theme::space::md);

        // ---- Right: ACCOUNT & ABOUT ----
        if (w::card::begin("##s_about", { leftW, contentH }, "ACCOUNT & ABOUT")) {
            w::labelsection("USER");

            // User pill container
            ImDrawList* dl = ImGui::GetWindowDrawList();
            const ImVec2 pillPos = ImGui::GetCursorScreenPos();
            float pillW = ImGui::GetContentRegionAvail().x;
            float pillH = 64.f;
            // Check hover
            bool pillHovered = ImGui::IsMouseHoveringRect(pillPos, pillPos + ImVec2(pillW, pillH));

            // Pill background: SURFACE2, border BORDER, border-radius 16px
            dl->AddRectFilled(pillPos, pillPos + ImVec2(pillW, pillH),
                IM_COL32(23, 23, 27, 255), 16.f); // SURFACE2
            dl->AddRect(pillPos, pillPos + ImVec2(pillW, pillH),
                pillHovered ? IM_COL32(200, 241, 53, 55) : IM_COL32(255, 255, 255, 15),
                16.f, 0, 1.f);

            // Avatar (real image if loaded, otherwise initials fallback)
            float avatarR = 22.f;
            ImVec2 avCenter = pillPos + ImVec2(12.f + avatarR, pillH * 0.5f);

            // AddImageRounded trick: draw the image into a circle clip
            ID3D11ShaderResourceView* avatarSrv = avatar::get_srv();
            if (avatarSrv && avatar::is_loaded()) {
                // Circle clip via AddImageRounded with radius = 24 (full circle)
                dl->AddImageRounded(avatarSrv,
                    pillPos + ImVec2(12.f, 10.f),
                    pillPos + ImVec2(60.f, 58.f),
                    ImVec2(0, 0), ImVec2(1, 1),
                    IM_COL32(255, 255, 255, 255), 24.f);
                // Border ring
                dl->AddCircle(avCenter, avatarR + 1.f,
                    IM_COL32(200, 241, 53, 40), 32, 1.5f);
            } else {
                // Circle placeholder with initials fallback
                dl->AddCircleFilled(avCenter, avatarR,
                    IM_COL32(23, 23, 27, 255), 32);
                dl->AddCircle(avCenter, avatarR,
                    IM_COL32(200, 241, 53, 40), 32, 1.5f);

                static const char* username = []() {
                    static char buf[128];
                    DWORD len = GetEnvironmentVariableA("USERNAME", buf, sizeof(buf));
                    return (len && len < sizeof(buf)) ? buf : "user";
                }();
                char initial[2] = { username[0] >= 'a' ? (char)(username[0] - 32) : username[0], '\0' };
                ImVec2 initSize = ImGui::CalcTextSize(initial);
                dl->AddText(font::label(), 14.f,
                    { avCenter.x - initSize.x * 0.5f, avCenter.y - initSize.y * 0.5f },
                    theme::col_accent(), initial);
            }

            // Username
            static std::string username_str = avatar::get_username();
            const char* username = username_str.c_str();
            dl->AddText(font::label(), 14.f,
                pillPos + ImVec2(70.f, 14.f),
                IM_COL32(240, 240, 238, 255), username);

            // Status
            dl->AddText(font::mono(), 10.f,
                pillPos + ImVec2(70.f, 34.f),
                IM_COL32(90, 90, 96, 255), "premium · active");

            // Invisible button for hover detection
            ImGui::SetCursorScreenPos(pillPos);
            ImGui::InvisibleButton("##pill_click", { pillW, pillH });

            w::gap(theme::space::sm);

            // ---- HOTKEYS RÁPIDOS ----
            w::labelsection("HOTKEYS");
            // Grid 2 columns
            struct QuickKey { const char* label; ImGuiKey* key; };
            QuickKey quickKeys[] = {
                {"Aimbot",    &global::aim::Aimbot_Key},
                {"Silent",    &global::silent::Silent_Key},
                {"Fly",       &global::misc::Fly_Key},
                {"Walkspeed", nullptr},
                {"Hitbox",    nullptr},
                {"Menu",      &global::setting::Menu_Key},
            };
            float chipW = (ImGui::GetContentRegionAvail().x - 8.f) * 0.5f;
            for (int i = 0; i < 6; i += 2) {
                // Row 1
                if (i < 6) {
                    ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()),
                        "%s", quickKeys[i].label);
                    ImGui::SameLine(chipW + 8.f);
                    if (quickKeys[i].key) {
                        ImGui::SetCursorPosX(chipW + 8.f + 10.f);
                        w::keyselect(quickKeys[i].label, quickKeys[i].key);
                    }
                }
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.f);
                // Row 2
                if (i + 1 < 6) {
                    ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()),
                        "%s", quickKeys[i + 1].label);
                    ImGui::SameLine(chipW + 8.f);
                    if (quickKeys[i + 1].key) {
                        ImGui::SetCursorPosX(chipW + 8.f + 10.f);
                        w::keyselect(quickKeys[i + 1].label, quickKeys[i + 1].key);
                    }
                }
                w::gap(theme::space::xs);
            }

            w::gap(theme::space::sm);
            w::labelsection("ABOUT");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Version");
            ImGui::SameLine(ImGui::GetContentRegionMax().x - 50.f);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_accent()), "1.0.0");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Build");
            ImGui::SameLine(ImGui::GetContentRegionMax().x - 80.f);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_accent()), "2025.06.12");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Status");
            ImGui::SameLine(ImGui::GetContentRegionMax().x - 70.f);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_accent()), "Premium");
        }
        w::card::end();
    }

} // namespace page
