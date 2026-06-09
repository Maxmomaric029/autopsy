#pragma once
#include <imgui/imgui.h>
#include "../widgets/controls.h"
#include "../../global.h"
#include "../../config.h"
#include <vector>
#include <string>

// ========================================================================
// Menu page content — each section's controls using the new widget system
// ========================================================================

namespace page {

    // ---- Aimbot page (Section 0) ---------------------------------------
    inline void aimbot(float bInW, float bInH) {
        const float halfW = (bInW - 8.f) * .5f;
        const float halfH = (bInH - 8.f) * .5f;

        if (w::card::begin("##abt", { halfW, halfH }, "AIMBOT")) {
            w::toggle("Enabled", &global::aim::Enabled);
            ImGui::SameLine(ImGui::GetContentRegionMax().x -
                w::bindwidth(global::aim::Aimbot_Key, global::aim::Aimbot_Mode) - 4.f);
            w::bind("##abk", &global::aim::Aimbot_Key, &global::aim::Aimbot_Mode);
            w::gap(2.f);
            w::toggle("Sticky Aim", &global::aim::AimbotSticky);
            w::toggle("Knocked Check", &global::aim::KnockedCheck);
            w::toggle("Visible Check", &global::aim::VisibleCheck);
            w::sliderint("Hit Chance", &global::aim::HitChance, 0, 100);
            w::gap(4.f);
            w::combo("Type", &global::aim::Aimbot_type, { "Mouse", "Camera" });
            w::gap(2.f);
            w::combo("Priority", &global::aim::TargetPriority, { "Crosshair", "Distance" });
            w::gap(2.f);
            w::combo("HitPart", &global::aim::HitPart, { "Head", "Torso", "LowerTorso" });
            w::gap(6.f);
            w::toggle("Prediction", &global::aim::Prediction);
            if (global::aim::Prediction) {
                w::toggle("Auto Prediction", &global::aim::AutoPrediction);
                if (!global::aim::AutoPrediction) {
                    w::sliderfloat("Pred X", &global::aim::PredictionX, 0.f, .5f);
                    w::sliderfloat("Pred Y", &global::aim::PredictionY, 0.f, .5f);
                    w::sliderfloat("Pred Z", &global::aim::PredictionZ, 0.f, .5f);
                }
            }
            w::gap(4.f);
            if (global::aim::Aimbot_type == 0) {
                w::sliderfloat("Smooth X", &global::aim::mouse::Smoothing_X, 0.f, 12.f);
                w::sliderfloat("Smooth Y", &global::aim::mouse::Smoothing_Y, 0.f, 12.f);
                w::sliderfloat("Sensitivity", &global::aim::mouse::Mouse_Sensitivty, 0.f, 5.f);
            }
            else {
                w::sliderfloat("Smooth X", &global::aim::camera::Smoothing_X, 0.f, 12.f);
                w::sliderfloat("Smooth Y", &global::aim::camera::Smoothing_Y, 0.f, 12.f);
            }
            w::gap(4.f);
            w::toggle("Trigger Bot", &global::aim::TriggerBot);
            if (global::aim::TriggerBot) {
                w::sliderfloat("TB Radius", &global::aim::TriggerRadius, 1.f, 25.f);
                w::sliderint("TB Delay", &global::aim::TriggerDelayMs, 0, 250);
            }
        }
        w::card::end();

        ImGui::SameLine(0.f, 6.f);

        if (w::card::begin("##sil", { halfW, halfH }, "SILENT AIM")) {
            w::toggle("Enabled", &global::silent::Enabled);
            ImGui::SameLine(ImGui::GetContentRegionMax().x -
                w::bindwidth(global::silent::Silent_Key, global::silent::Silent_Mode) - 4.f);
            w::bind("##sik", &global::silent::Silent_Key, &global::silent::Silent_Mode);
            w::gap(2.f);
            w::toggle("Sticky Aim", &global::silent::StickyAim);
            w::toggle("Spoof Mouse", &global::silent::SpoofMouse);
            w::toggle("Knocked Check", &global::silent::KnockedCheck);
            w::toggle("Visible Check", &global::silent::VisibleCheck);
            w::gap(4.f);
            w::combo("Priority", &global::silent::TargetPriority, { "Crosshair", "Distance" });
            w::gap(2.f);
            w::combo("Hit Part", &global::silent::AimPart, { "Head", "Torso", "LowerTorso" });
            w::gap(4.f);
            w::toggle("Prediction", &global::silent::Prediction);
            if (global::silent::Prediction) {
                w::toggle("Auto Prediction", &global::silent::AutoPrediction);
                if (!global::silent::AutoPrediction) {
                    w::sliderfloat("Pred X", &global::silent::PredictionX, 0.f, .5f);
                    w::sliderfloat("Pred Y", &global::silent::PredictionY, 0.f, .5f);
                    w::sliderfloat("Pred Z", &global::silent::PredictionZ, 0.f, .5f);
                }
            }
        }
        w::card::end();

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.f);
        const float r2H = halfH - 6.f;

        if (w::card::begin("##abfov", { halfW, r2H }, "AIMBOT FOV")) {
            if (global::aim::Enabled) {
                w::togglecolor("Draw FOV", &global::aim::DrawFov, "##abfc", global::aim::FovColor);
                if (global::aim::DrawFov) {
                    w::sliderfloat("Radius", &global::aim::FovSize, 1.f, 500.f);
                    w::toggle("Spin", &global::aim::FovSpin);
                    if (global::aim::FovSpin)
                        w::sliderint("Spin Speed", &global::aim::FovSpinSpeed, 1, 5);
                    w::toggle("Constrain to FOV", &global::aim::useFov);
                }
            }
            else {
                w::gap(4.f);
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()),
                    "Enable aimbot first");
            }
        }
        w::card::end();

        ImGui::SameLine(0.f, 6.f);

        if (w::card::begin("##sifov", { halfW, r2H }, "SILENT FOV")) {
            if (global::silent::Enabled) {
                w::togglecolor("Draw FOV", &global::silent::DrawFov, "##sifc", global::silent::FovColor);
                w::gap(2.f);
                w::toggle("Gun-Based FOV", &global::silent::GunBasedFov);
                if (global::silent::GunBasedFov) {
                    w::sliderfloat("Default", &global::silent::fov, 0.f, 300.f);
                    w::sliderfloat("Dbl Barrel", &global::silent::FovDoubleBarrel, 0.f, 300.f);
                    w::sliderfloat("Tactical", &global::silent::FovTacticalShotgun, 0.f, 300.f);
                    w::sliderfloat("Revolver", &global::silent::FovRevolver, 0.f, 300.f);
                }
                else {
                    w::sliderfloat("Static FOV", &global::silent::fov, 0.f, 500.f);
                }
                w::toggle("Spin FOV", &global::silent::FovSpin);
                if (global::silent::FovSpin)
                    w::sliderint("Spin Speed", &global::silent::FovSpinSpeed, 1, 5);
                w::toggle("Constrain to FOV", &global::silent::UseFov);
            }
            else {
                w::gap(4.f);
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()),
                    "Enable silent aim first");
            }
        }
        w::card::end();
    }

    // ---- Visuals page (Section 1) ---------------------------------------
    inline void visuals(float bInW, float bInH) {
        const float halfW = (bInW - 8.f) * .5f;

        if (w::card::begin("##esp", { halfW, bInH }, "ESP TOGGLES")) {
            w::toggle("Master Enable", &global::esp::Enabled);
            w::toggle("Visible Check", &global::esp::VisibleCheck);
            if (global::esp::VisibleCheck) {
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Visible");
                w::color4("##espvisc", global::esp::color::Visible);
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Not Visible");
                w::color4("##espnotvisc", global::esp::color::NotVisible);
            }
            w::gap(4.f);

            w::labelsection("BOX");
            w::togglecolor("Box", &global::esp::Box, "##bxc", global::esp::color::Box);
            if (global::esp::Box) {
                w::combo("Style", &global::esp::Box_Type, { "Bounding", "Corner" });
                w::gap(2.f);
                w::toggle("Box Fill", &global::esp::Box_Fill);
                if (global::esp::Box_Fill) {
                    w::dualcolor("##bft", global::esp::color::BoxFill_Top,
                        "##bfb", global::esp::color::BoxFill_Bottom);
                    w::toggle("Gradient", &global::esp::Box_Fill_Gradient);
                    if (global::esp::Box_Fill_Gradient) {
                        w::toggle("Rotation", &global::esp::Box_Fill_Gradient_Rotate);
                        if (global::esp::Box_Fill_Gradient_Rotate) {
                            w::combo("Rotation Type", &global::esp::Box_Fill_Type,
                                { "Side", "Bottom", "Spin" });
                            w::sliderint("Speed", &global::esp::BoxFillSpeed, 1, 5);
                        }
                    }
                }
            }
            w::gap(4.f);

            w::labelsection("HEALTH");
            w::togglecolor("Health Bar", &global::esp::Healthbar, "##hbc", global::esp::color::Healthbar);
            if (global::esp::Healthbar) {
                w::combo("Bar Style", &global::esp::Healthbar_Type, { "Static", "Gradient" });
                if (global::esp::Healthbar_Type == 1) {
                    ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()),
                        "Gradient (top \xE2\x80\x94 mid \xE2\x80\x94 bottom)");
                    w::tricolor("##hbt", global::esp::color::Healthbar_Top,
                        "##hbm", global::esp::color::Healthbar_Middle,
                        "##hbb", global::esp::color::Healthbar_Bottom);
                }
                w::sliderint("Bar Gap", &global::esp::gap, 1, 5);
                w::sliderint("Bar Thickness", &global::esp::Thickness, 1, 5);
            }
            w::togglecolor("Health Text", &global::esp::Health, "##htc", global::esp::color::Health);
            w::gap(4.f);

            w::labelsection("LABELS");
            w::togglecolor("Name", &global::esp::name, "##nc", global::esp::color::name);
            if (global::esp::name)
                w::combo("Name Format", &global::esp::Name_Type,
                    { "Name", "Display Name", "Name & Display" });
            w::togglecolor("Distance", &global::esp::Distance, "##dc", global::esp::color::Distance);
            w::togglecolor("Rig Type", &global::esp::Rig_Type, "##rc", global::esp::color::Rig_Type);
            w::togglecolor("Tool", &global::esp::tool, "##tc", global::esp::color::tool);
            w::gap(4.f);

            w::labelsection("3D");
            w::togglecolor("Skeleton", &global::esp::Skeleton, "##skc", global::esp::color::Skeleton);
            w::togglecolor("Trails", &global::esp::Trails, "##trc", global::esp::color::Trails);
            w::togglecolor("Chinese Hats", &global::esp::Chinese_Hat, "##chhatc", global::esp::color::hat);
            w::togglecolor("Aim Lines", &global::esp::aimline, "##aimlc", global::esp::color::aimline);
            w::toggle("Chams", &global::esp::Chams);
            if (global::esp::Chams) {
                w::dualcolor("##chc", global::esp::color::Chams,
                    "##choc", global::esp::color::ChamsOutline);
                w::toggle("Fade", &global::esp::ChamsFade);
                if (global::esp::ChamsFade)
                    w::sliderint("Fade Speed", &global::esp::ChamsFadeSpeed, 1, 5);
            }
        }
        w::card::end();

        ImGui::SameLine(0.f, 6.f);

        if (w::card::begin("##vopt", { halfW, bInH }, "OPTIONS")) {
            w::toggle("Exclude Team", &global::setting::Team_Check);
            w::toggle("Exclude Client", &global::setting::Client_Check);
            w::gap(4.f);
            w::labelsection("RENDERING");
            w::sliderfloat("Render Distance", &global::esp::Render_Distance, 0.f, 500.f);
        }
        w::card::end();
    }

    // ---- World page (Section 2) -----------------------------------------
    inline void world(float bInW, float bInH) {
        if (w::card::begin("##wld", { bInW, bInH }, "WORLD MANIPULATION")) {
            w::toggle("Skybox Changer", &global::world::Skybox);
            if (global::world::Skybox) {
                w::combo("Preset", &global::world::Skybox_Type, {
                    "Autopsy", "Space", "Pink Sky", "Minecraft", "Night Cloudy",
                    "Sparkling Night", "Winterness", "Dark Crimson", "Nebula",
                    "Tropical", "Green Sky" });
                w::toggle("Rotation", &global::world::Rotate);
                if (global::world::Rotate)
                    w::sliderfloat("Rotate Speed", &global::world::Skybox_Rotate_Speed, 0.f, 5.f);
            }
            w::gap(4.f);

            w::labelsection("LIGHTING");
            w::togglecolor("Atmosphere", &global::world::Ambience,
                "##atmc", global::world::color::Ambience);
            w::toggle("Fog", &global::world::Fog);
            if (global::world::Fog) {
                ImGui::SameLine(ImGui::GetContentRegionMax().x - 15.f);
                w::color4("##fogc", global::world::color::Fog);
                w::sliderfloat("Fog Distance", &global::world::Fog_Distance, 0.f, 1000.f);
            }
            w::toggle("Brightness", &global::world::Brightness);
            if (global::world::Brightness)
                w::sliderfloat("Brightness", &global::world::BrightnessI, 0.f, 10.f);
            w::toggle("Exposure", &global::world::Exposure);
            if (global::world::Exposure)
                w::sliderfloat("Exposure", &global::world::ExposureI, -3.f, 3.f);
            w::gap(4.f);

            w::labelsection("CAMERA");
            w::toggle("Custom FOV", &global::world::FOV);
            if (global::world::FOV)
                w::sliderfloat("FOV", &global::world::FOV_Distance, 70.f, 120.f);
        }
        w::card::end();
    }

    // ---- Misc page (Section 3) -----------------------------------------
    inline void misc(float bInW, float bInH) {
        const float halfW = (bInW - 8.f) * .5f;

        if (w::card::begin("##msc", { halfW, bInH }, "EXPLOITS")) {
            w::toggle("Fly", &global::misc::fly);
            ImGui::SameLine(ImGui::GetContentRegionMax().x -
                w::bindwidth(global::misc::Fly_Key, global::misc::Fly_Mode) - 4.f);
            w::bind("##flyk", &global::misc::Fly_Key, &global::misc::Fly_Mode);
            if (global::misc::fly)
                w::sliderfloat("Fly Speed", &global::misc::Fly_Speed, 0.f, 200.f);
            w::gap(4.f);

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

        ImGui::SameLine(0.f, 6.f);

        // Right card: config / settings
        if (w::card::begin("##cfg", { halfW, bInH }, "CONFIG")) {
            static std::vector<std::string> configs;
            static int configIdx = -1;
            static char newNameBuf[128] = {0};

            config::refresh(configs);

            // Current config list
            if (configs.empty()) {
                w::gap(4.f);
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()),
                    "No configs found");
            }
            else {
                std::vector<const char*> items;
                for (auto& c : configs) items.push_back(c.c_str());
                w::combo("Config", &configIdx, items);
                if (configIdx < 0 || configIdx >= (int)configs.size())
                    configIdx = -1;
            }

            w::gap(4.f);

            // Save / Load / Delete buttons
            if (configIdx >= 0 && configIdx < (int)configs.size()) {
                if (w::accent_button("Load", -1.f, 24.f))
                    config::load(configs[configIdx]);
                w::gap(2.f);
                if (w::accent_button("Save", -1.f, 24.f))
                    config::save(configs[configIdx]);
                w::gap(2.f);
                if (w::danger_button("Delete", -1.f, 24.f)) {
                    config::remove(configs[configIdx]);
                    configIdx = -1;
                }
                w::gap(4.f);
            }

            w::labelsection("CREATE");
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

    // ---- Blade Ball page (Section 4) ------------------------------------
    inline void bladeball(float bInW, float bInH) {
        const float halfW = (bInW - 8.f) * .5f;
        const bool bladeBallActive = global::GameID == global::ball::PlaceId;

        if (w::card::begin("##bbmain", { halfW, bInH }, "BLADE BALL")) {
            if (!bladeBallActive) {
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_accent()),
                    "User is not playing Blade Ball.");
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()),
                    "Start Blade Ball to enable these features.");
                w::gap(8.f);
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()),
                    "Features locked");
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()),
                    "Required place: 13772394625");
            }
            else {
                w::togglecolor("Ball ESP", &global::ball::BallEsp,
                    "##bbespcol", global::ball::BallEspColor);
                w::togglecolor("Parry Range", &global::ball::DrawParryRange,
                    "##bbrangecol", global::ball::ParryRangeColor);
                w::gap(4.f);
                w::toggle("Auto Parry", &global::ball::AutoParry);
                w::toggle("Clash Mode", &global::ball::ClashMode);
                w::toggle("Press F", &global::ball::pressf);
                w::toggle("Predict Timing", &global::ball::PredictTiming);
                w::toggle("Auto Range", &global::ball::AutoRange);
                w::toggle("Auto Timing", &global::ball::AutoTiming);
                w::toggle("Dynamic Timing", &global::ball::DynamicTiming);
                w::sliderfloat("Parry Distance", &global::ball::ParryDistance, 5.f, 120.f);
                w::sliderfloat("Distance Buffer", &global::ball::DistanceBuffer, 0.f, 100.f);
                w::sliderfloat("Timing MS", &global::ball::ParryTimingMs, 35.f, 260.f);
                w::sliderfloat("Cooldown MS", &global::ball::ParryCooldownMs, 20.f, 220.f);
                w::sliderfloat("Min Speed", &global::ball::MinBallSpeed, 1.f, 80.f);
            }
        }
        w::card::end();

        ImGui::SameLine(0.f, 6.f);

        if (w::card::begin("##bbinfo", { halfW, bInH }, "OPTIONS")) {
            if (!bladeBallActive) {
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()),
                    "Blade Ball options are disabled.");
                w::gap(4.f);
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()),
                    "Join Blade Ball and the controls will unlock.");
            }
            else {
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()),
                    "Ball ESP draws each detected ball.");
                w::gap(4.f);
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()),
                    "Auto Parry automatically parries based on configured");
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()),
                    "predictions, timing, and range settings.");
                w::gap(8.f);
                w::labelsection("SPAM PARRY");
                w::toggle("Spam Parry", &global::ball::SpamParry);
                ImGui::SameLine(ImGui::GetContentRegionMax().x -
                    w::bindwidth(global::ball::SpamParry_Key, global::ball::SpamParry_Mode) - 4.f);
                w::bind("##bbspamk", &global::ball::SpamParry_Key, &global::ball::SpamParry_Mode);
            }
        }
        w::card::end();
    }

    // ---- Settings page (Section 5) --------------------------------------
    inline void settings(float bInW, float bInH) {
        const float halfW = (bInW - 8.f) * .5f;

        if (w::card::begin("##sgen", { halfW, bInH }, "GENERAL")) {
            w::labelsection("MENU");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Menu Key");
            ImGui::SameLine(ImGui::GetContentRegionMax().x - 60.f);
            w::keyselect("##menukey", &global::setting::Menu_Key);
            w::gap(4.f);
            w::labelsection("DISPLAY");
            w::toggle("Compact UI", &global::setting::Compact_UI);
            w::combo("Performance", &global::setting::Performance_Mode,
                { "VSync", "Eco", "Unlocked" });
            w::toggle("Streamproof", &global::setting::Streamproof);
            w::gap(4.f);

            w::labelsection("OVERLAY");
            w::toggle("Watermark", &global::overlay::watermark);
            w::toggle("Hotkeys", &global::overlay::hotkey);
            if (global::overlay::hotkey) {
                w::toggle("  Aimbot", &global::overlay::Hotkey_Aimbot);
                w::toggle("  Silent", &global::overlay::Hotkey_Silent);
                w::toggle("  Fly", &global::overlay::Hotkey_Fly);
                w::toggle("  Blade Ball Spam", &global::overlay::Hotkey_BladeBallSpam);
                w::toggle("  Walkspeed", &global::overlay::Hotkey_Walkspeed);
                w::toggle("  Hitbox", &global::overlay::Hotkey_HitboxExpander);
            }
            w::toggle("Radar", &global::overlay::radar);
            if (global::overlay::radar) {
                w::combo("Shape", &global::overlay::Radar_Shape, { "Circle", "Square" });
                w::sliderfloat("Zoom", &global::overlay::Radar_Zoom, .3f, 4.f);
                w::sliderfloat("Size", &global::overlay::Radar_Size, 130.f, 280.f);
                w::toggle("Rotate", &global::overlay::Radar_Rotate);
            }
            w::toggle("Aim Warning", &global::overlay::AimWarning);
            if (global::overlay::AimWarning)
                w::sliderfloat("AimView Length", &global::overlay::AimView_MaxLength, 50.f, 1000.f);
        }
        w::card::end();

        ImGui::SameLine(0.f, 6.f);

        if (w::card::begin("##stheme", { halfW, bInH }, "THEME")) {
            w::labelsection("COLORS");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Accent");
            w::color4("##acc", global::setting::color::Accent);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Accent 2");
            w::color4("##acc2", global::setting::color::Accent2);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Background");
            w::color4("##bg", global::setting::color::Window);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Card");
            w::color4("##card", global::setting::color::card);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "Text");
            w::color4("##text", global::setting::color::text);
            w::gap(8.f);

            // HUD colors
            w::labelsection("HUD");
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "HUD Accent");
            w::color4("##hudacc", global::overlay::color::Accent);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "HUD Accent 2");
            w::color4("##hudacc2", global::overlay::color::Accent2);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "HUD Panel");
            w::color4("##hudpnl", global::overlay::color::panel);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(theme::col_muted()), "HUD Text");
            w::color4("##hudtxt", global::overlay::color::text);

        }
        w::card::end();
    }

} // namespace page
