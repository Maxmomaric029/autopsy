#pragma once

#include <Windows.h>
#include <ShlObj.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "global.h"
#include "nlohmann_json.hpp"

namespace fs = std::filesystem;

namespace config
{
    inline std::string dir()
    {
        char* appdata = nullptr;
        size_t size = 0;
        if (_dupenv_s(&appdata, &size, "APPDATA") == 0 && appdata)
        {
            fs::path dir = fs::path(appdata) / "discord";
            free(appdata);
            if (!fs::exists(dir))
                fs::create_directories(dir);
            return dir.string() + "\\";
        }
        fs::path fallback = "C:\\discord";
        if (!fs::exists(fallback))
            fs::create_directories(fallback);
        return fallback.string() + "\\";
    }

    inline void refresh(std::vector<std::string>& list)
    {
        list.clear();
        for (auto& entry : fs::directory_iterator(dir()))
            if (entry.path().extension() == ".ini")
                list.push_back(entry.path().stem().string());
    }

    inline void remove(const std::string& name)
    {
        fs::path p = dir() + name + ".ini";
        if (fs::exists(p)) fs::remove(p);
    }

    inline fs::path path(const std::string& name)
    {
        return fs::path(dir()) / (name + ".ini");
    }

    inline std::string encode64(const std::string& input)
    {
        static constexpr char table[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        out.reserve(((input.size() + 2) / 3) * 4);

        int val = 0;
        int valb = -6;
        for (unsigned char c : input)
        {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0)
            {
                out.push_back(table[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6)
            out.push_back(table[((val << 8) >> (valb + 8)) & 0x3F]);
        while (out.size() % 4)
            out.push_back('=');
        return out;
    }

    inline bool decode64(const std::string& input, std::string& output)
    {
        static constexpr unsigned char invalid = 0xFF;
        unsigned char table[256];
        std::fill(std::begin(table), std::end(table), invalid);
        for (int i = 0; i < 26; ++i)
        {
            table[(unsigned char)('A' + i)] = (unsigned char)i;
            table[(unsigned char)('a' + i)] = (unsigned char)(26 + i);
        }
        for (int i = 0; i < 10; ++i)
            table[(unsigned char)('0' + i)] = (unsigned char)(52 + i);
        table[(unsigned char)'+'] = 62;
        table[(unsigned char)'/'] = 63;

        output.clear();
        int val = 0;
        int valb = -8;
        for (unsigned char c : input)
        {
            if (c == '=')
                break;
            if (c == '\r' || c == '\n' || c == '\t' || c == ' ')
                continue;
            if (table[c] == invalid)
                return false;

            val = (val << 6) + table[c];
            valb += 6;
            if (valb >= 0)
            {
                output.push_back((char)((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return !output.empty();
    }

    inline bool export64(const std::string& name, std::string& output)
    {
        const fs::path file_path = path(name);
        std::ifstream file(file_path, std::ios::binary);
        if (!file)
            return false;

        std::ostringstream ss;
        ss << file.rdbuf();
        output = encode64(ss.str());
        return true;
    }

    inline bool import64(const std::string& name, const std::string& encoded)
    {
        std::string decoded;
        if (name.empty() || !decode64(encoded, decoded))
            return false;

        std::ofstream file(path(name), std::ios::binary | std::ios::trunc);
        if (!file)
            return false;
        file.write(decoded.data(), (std::streamsize)decoded.size());
        return file.good();
    }


    static void writes(const char* sec, const char* key, const std::string& val, const char* path)
    {
        WritePrivateProfileStringA(sec, key, val.c_str(), path);
    }
    static void writeb(const char* sec, const char* key, bool v, const char* path)
    {
        WritePrivateProfileStringA(sec, key, v ? "1" : "0", path);
    }
    static void writef(const char* sec, const char* key, float v, const char* path)
    {
        WritePrivateProfileStringA(sec, key, std::to_string(v).c_str(), path);
    }
    static void writei(const char* sec, const char* key, int v, const char* path)
    {
        WritePrivateProfileStringA(sec, key, std::to_string(v).c_str(), path);
    }
    static void writec(const char* sec, const char* key, float col[4], const char* path)
    {
        auto k = [&](int i) { return std::string(key) + std::to_string(i); };
        for (int i = 0; i < 4; i++)
            WritePrivateProfileStringA(sec, k(i).c_str(), std::to_string(col[i]).c_str(), path);
    }

    static bool readb(const char* sec, const char* key, bool def, const char* path)
    {
        return GetPrivateProfileIntA(sec, key, def ? 1 : 0, path) != 0;
    }
    static float readf(const char* sec, const char* key, float def, const char* path)
    {
        char buf[64];
        GetPrivateProfileStringA(sec, key, std::to_string(def).c_str(), buf, sizeof(buf), path);
        try { return std::stof(buf); }
        catch (...) { return def; }
    }
    static int readi(const char* sec, const char* key, int def, const char* path)
    {
        return GetPrivateProfileIntA(sec, key, def, path);
    }
    static void readc(const char* sec, const char* key, float col[4], const char* path)
    {
        for (int i = 0; i < 4; i++)
        {
            auto k = std::string(key) + std::to_string(i);
            col[i] = readf(sec, k.c_str(), col[i], path);
        }
    }


    inline void save(const std::string& name)
    {
        std::string p = dir() + name + ".ini";
        const char* path = p.c_str();

        writeb("Settings", "Team_Check", global::setting::Team_Check, path);
        writeb("Settings", "Client_Check", global::setting::Client_Check, path);
        writeb("Settings", "Streamproof", global::setting::Streamproof, path);
        writei("Settings", "Performance_Mode", global::setting::Performance_Mode, path);
        writeb("Settings", "Compact_UI", global::setting::Compact_UI, path);
        writei("Settings", "Menu_Key", (int)global::setting::Menu_Key, path);
        writeb("Settings", "Sound_Enabled", global::setting::Sound_Enabled, path);
        writef("Settings", "Sound_Volume", global::setting::Sound_Volume, path);
        writec("Settings", "Theme_Accent", global::setting::color::Accent, path);
        writec("Settings", "Theme_Accent2", global::setting::color::Accent2, path);
        writec("Settings", "Theme_Window", global::setting::color::Window, path);
        writec("Settings", "Theme_Card", global::setting::color::card, path);
        writec("Settings", "Theme_Text", global::setting::color::text, path);

        writeb("Overlay", "Watermark", global::overlay::watermark, path);
        writeb("Overlay", "Hotkeys", global::overlay::hotkey, path);
        writeb("Overlay", "Hotkey_Aimbot", global::overlay::Hotkey_Aimbot, path);
        writeb("Overlay", "Hotkey_Silent", global::overlay::Hotkey_Silent, path);
        writeb("Overlay", "Hotkey_Fly", global::overlay::Hotkey_Fly, path);
        writeb("Overlay", "Hotkey_BladeBallSpam", global::overlay::Hotkey_BladeBallSpam, path);
        writeb("Overlay", "Hotkey_Walkspeed", global::overlay::Hotkey_Walkspeed, path);
        writeb("Overlay", "Hotkey_HitboxExpander", global::overlay::Hotkey_HitboxExpander, path);
        writeb("Overlay", "Radar", global::overlay::radar, path);
        writeb("Overlay", "AimWarning", global::overlay::AimWarning, path);
        writef("Overlay", "Watermark_X", global::overlay::Watermark_Pos.x, path);
        writef("Overlay", "Watermark_Y", global::overlay::Watermark_Pos.y, path);
        writef("Overlay", "Hotkeys_X", global::overlay::Hotkeys_Pos.x, path);
        writef("Overlay", "Hotkeys_Y", global::overlay::Hotkeys_Pos.y, path);
        writef("Overlay", "Radar_X", global::overlay::Radar_Pos.x, path);
        writef("Overlay", "Radar_Y", global::overlay::Radar_Pos.y, path);
        writei("Overlay", "Radar_Shape", global::overlay::Radar_Shape, path);
        writeb("Overlay", "Radar_Rotate", global::overlay::Radar_Rotate, path);
        writef("Overlay", "Radar_Zoom", global::overlay::Radar_Zoom, path);
        writef("Overlay", "Radar_Size", global::overlay::Radar_Size, path);
        writef("Overlay", "AimView_MaxLength", global::overlay::AimView_MaxLength, path);
        writec("Overlay", "Hud_Accent", global::overlay::color::Accent, path);
        writec("Overlay", "Hud_Accent2", global::overlay::color::Accent2, path);
        writec("Overlay", "Hud_Panel", global::overlay::color::panel, path);
        writec("Overlay", "Hud_Text", global::overlay::color::text, path);

        writeb("BladeBall", "BallEsp", global::ball::BallEsp, path);
        writeb("BladeBall", "AutoParry", global::ball::AutoParry, path);
        writeb("BladeBall", "DrawParryRange", global::ball::DrawParryRange, path);
        writeb("BladeBall", "ClashMode", global::ball::ClashMode, path);
        writeb("BladeBall", "PressF", global::ball::pressf, path);
        writeb("BladeBall", "PredictTiming", global::ball::PredictTiming, path);
        writeb("BladeBall", "DynamicTiming", global::ball::DynamicTiming, path);
        writeb("BladeBall", "AutoRange", global::ball::AutoRange, path);
        writeb("BladeBall", "AutoTiming", global::ball::AutoTiming, path);
        writeb("BladeBall", "SpamParry", global::ball::SpamParry, path);
        writef("BladeBall", "ParryDistance", global::ball::ParryDistance, path);
        writef("BladeBall", "DistanceBuffer", global::ball::DistanceBuffer, path);
        writef("BladeBall", "ParryCooldownMs", global::ball::ParryCooldownMs, path);
        writef("BladeBall", "ParryTimingMs", global::ball::ParryTimingMs, path);
        writef("BladeBall", "MinBallSpeed", global::ball::MinBallSpeed, path);
        writei("BladeBall", "SpamParry_Key", (int)global::ball::SpamParry_Key, path);
        writei("BladeBall", "SpamParry_Mode", (int)global::ball::SpamParry_Mode, path);
        writec("BladeBall", "BallEspColor", global::ball::BallEspColor, path);
        writec("BladeBall", "ParryRangeColor", global::ball::ParryRangeColor, path);

        writeb("World", "Skybox", global::world::Skybox, path);
        writeb("World", "Rotate", global::world::Rotate, path);
        writeb("World", "Ambience", global::world::Ambience, path);
        writeb("World", "Fog", global::world::Fog, path);
        writeb("World", "Brightness", global::world::Brightness, path);
        writeb("World", "Exposure", global::world::Exposure, path);
        writeb("World", "FOV", global::world::FOV, path);
        writei("World", "Skybox_Type", global::world::Skybox_Type, path);
        writef("World", "Skybox_Rotate_Speed", global::world::Skybox_Rotate_Speed, path);
        writef("World", "Fog_Distance", global::world::Fog_Distance, path);
        writef("World", "FOV_Distance", global::world::FOV_Distance, path);
        writef("World", "ExposureI", global::world::ExposureI, path);
        writef("World", "BrightnessI", global::world::BrightnessI, path);
        writec("World", "Ambience", global::world::color::Ambience, path);
        writec("World", "Fog", global::world::color::Fog, path);

        writeb("Aimbot", "Enabled", global::aim::Enabled, path);
        writeb("Aimbot", "useFov", global::aim::useFov, path);
        writeb("Aimbot", "DrawFov", global::aim::DrawFov, path);
        writeb("Aimbot", "FovSpin", global::aim::FovSpin, path);
        writeb("Aimbot", "FillFov", global::aim::FillFov, path);
        writeb("Aimbot", "AimbotSticky", global::aim::AimbotSticky, path);
        writeb("Aimbot", "Shake", global::aim::Shake, path);
        writeb("Aimbot", "KnockedCheck", global::aim::KnockedCheck, path);
        writeb("Aimbot", "Prediction", global::aim::Prediction, path);
        writeb("Aimbot", "AutoPrediction", global::aim::AutoPrediction, path);
        writeb("Aimbot", "VisibleCheck", global::aim::VisibleCheck, path);
        writeb("Aimbot", "TriggerBot", global::aim::TriggerBot, path);
        writef("Aimbot", "FovSize", global::aim::FovSize, path);
        writef("Aimbot", "ShakeX", global::aim::ShakeX, path);
        writef("Aimbot", "ShakeY", global::aim::ShakeY, path);
        writef("Aimbot", "ShakeZ", global::aim::ShakeZ, path);
        writef("Aimbot", "PredictionX", global::aim::PredictionX, path);
        writef("Aimbot", "PredictionY", global::aim::PredictionY, path);
        writef("Aimbot", "PredictionZ", global::aim::PredictionZ, path);
        writef("Aimbot", "TriggerRadius", global::aim::TriggerRadius, path);
        writei("Aimbot", "HitPart", global::aim::HitPart, path);
        writei("Aimbot", "Aimbot_type", global::aim::Aimbot_type, path);
        writei("Aimbot", "TargetPriority", global::aim::TargetPriority, path);
        writei("Aimbot", "HitChance", global::aim::HitChance, path);
        writei("Aimbot", "TriggerDelayMs", global::aim::TriggerDelayMs, path);
        writei("Aimbot", "FovSpinSpeed", global::aim::FovSpinSpeed, path);
        writei("Aimbot", "Aimbot_Key", (int)global::aim::Aimbot_Key, path);
        writei("Aimbot", "Aimbot_Mode", (int)global::aim::Aimbot_Mode, path);
        writec("Aimbot", "FovColor", global::aim::FovColor, path);
        writef("Aimbot", "CamSmoothX", global::aim::camera::Smoothing_X, path);
        writef("Aimbot", "CamSmoothY", global::aim::camera::Smoothing_Y, path);
        writef("Aimbot", "MouseSmoothX", global::aim::mouse::Smoothing_X, path);
        writef("Aimbot", "MouseSmoothY", global::aim::mouse::Smoothing_Y, path);
        writef("Aimbot", "MouseSens", global::aim::mouse::Mouse_Sensitivty, path);

        writeb("Silent", "Enabled", global::silent::Enabled, path);
        writeb("Silent", "DrawFov", global::silent::DrawFov, path);
        writeb("Silent", "StickyAim", global::silent::StickyAim, path);
        writeb("Silent", "SpoofMouse", global::silent::SpoofMouse, path);
        writeb("Silent", "UseFov", global::silent::UseFov, path);
        writeb("Silent", "KnockedCheck", global::silent::KnockedCheck, path);
        writeb("Silent", "GunBasedFov", global::silent::GunBasedFov, path);
        writeb("Silent", "Prediction", global::silent::Prediction, path);
        writeb("Silent", "AutoPrediction", global::silent::AutoPrediction, path);
        writeb("Silent", "VisibleCheck", global::silent::VisibleCheck, path);
        writeb("Silent", "FovSpin", global::silent::FovSpin, path);
        writeb("Silent", "FillFov", global::silent::FillFov, path);
        writef("Silent", "Fov", global::silent::fov, path);
        writef("Silent", "FovDoubleBarrel", global::silent::FovDoubleBarrel, path);
        writef("Silent", "FovTacticalShotgun", global::silent::FovTacticalShotgun, path);
        writef("Silent", "FovRevolver", global::silent::FovRevolver, path);
        writef("Silent", "PredictionX", global::silent::PredictionX, path);
        writef("Silent", "PredictionY", global::silent::PredictionY, path);
        writef("Silent", "PredictionZ", global::silent::PredictionZ, path);
        writei("Silent", "AimPart", global::silent::AimPart, path);
        writei("Silent", "TargetPriority", global::silent::TargetPriority, path);
        writei("Silent", "FovSpinSpeed", global::silent::FovSpinSpeed, path);
        writei("Silent", "Silent_Key", (int)global::silent::Silent_Key, path);
        writei("Silent", "Silent_Mode", (int)global::silent::Silent_Mode, path);
        writec("Silent", "FovColor", global::silent::FovColor, path);

        writeb("Visuals", "Enabled", global::esp::Enabled, path);
        writeb("Visuals", "Box", global::esp::Box, path);
        writeb("Visuals", "Box_Fill", global::esp::Box_Fill, path);
        writeb("Visuals", "Box_Fill_Gradient", global::esp::Box_Fill_Gradient, path);
        writeb("Visuals", "Box_Fill_Gradient_Rotate", global::esp::Box_Fill_Gradient_Rotate, path);
        writeb("Visuals", "Healthbar", global::esp::Healthbar, path);
        writeb("Visuals", "Health", global::esp::Health, path);
        writeb("Visuals", "Name", global::esp::name, path);
        writeb("Visuals", "Distance", global::esp::Distance, path);
        writeb("Visuals", "Rig_Type", global::esp::Rig_Type, path);
        writeb("Visuals", "Tool", global::esp::tool, path);
        writeb("Visuals", "Skeleton", global::esp::Skeleton, path);
        writeb("Visuals", "Chams", global::esp::Chams, path);
        writeb("Visuals", "ChamsFade", global::esp::ChamsFade, path);
        writeb("Visuals", "Trails", global::esp::Trails, path);
        writeb("Visuals", "Chinese_Hat", global::esp::Chinese_Hat, path);
        writeb("Visuals", "AimLine", global::esp::aimline, path);
        writeb("Visuals", "VisibleCheck", global::esp::VisibleCheck, path);
        writef("Visuals", "Render_Distance", global::esp::Render_Distance, path);
        writei("Visuals", "ChamsFadeSpeed", global::esp::ChamsFadeSpeed, path);
        writei("Visuals", "BoxFillSpeed", global::esp::BoxFillSpeed, path);
        writei("Visuals", "Healthbar_Type", global::esp::Healthbar_Type, path);
        writei("Visuals", "Box_Type", global::esp::Box_Type, path);
        writei("Visuals", "Box_Fill_Type", global::esp::Box_Fill_Type, path);
        writei("Visuals", "Name_Type", global::esp::Name_Type, path);
        writei("Visuals", "Gap", global::esp::gap, path);
        writei("Visuals", "Thickness", global::esp::Thickness, path);
        writec("Visuals", "Box", global::esp::color::Box, path);
        writec("Visuals", "BoxFill_Top", global::esp::color::BoxFill_Top, path);
        writec("Visuals", "BoxFill_Bottom", global::esp::color::BoxFill_Bottom, path);
        writec("Visuals", "Healthbar", global::esp::color::Healthbar, path);
        writec("Visuals", "Name", global::esp::color::name, path);
        writec("Visuals", "Distance", global::esp::color::Distance, path);
        writec("Visuals", "Rig_Type", global::esp::color::Rig_Type, path);
        writec("Visuals", "Tool", global::esp::color::tool, path);
        writec("Visuals", "Health", global::esp::color::Health, path);
        writec("Visuals", "Skeleton", global::esp::color::Skeleton, path);
        writec("Visuals", "Chams", global::esp::color::Chams, path);
        writec("Visuals", "ChamsOutline", global::esp::color::ChamsOutline, path);
        writec("Visuals", "Trails", global::esp::color::Trails, path);
        writec("Visuals", "ChineseHat", global::esp::color::hat, path);
        writec("Visuals", "AimLine", global::esp::color::aimline, path);
        writec("Visuals", "VisibleColor", global::esp::color::Visible, path);
        writec("Visuals", "NotVisibleColor", global::esp::color::NotVisible, path);
        writec("Visuals", "Healthbar_Top", global::esp::color::Healthbar_Top, path);
        writec("Visuals", "Healthbar_Middle", global::esp::color::Healthbar_Middle, path);
        writec("Visuals", "Healthbar_Bottom", global::esp::color::Healthbar_Bottom, path);

        writeb("Misc", "Fly", global::misc::fly, path);
        writef("Misc", "Fly_Speed", global::misc::Fly_Speed, path);
        writei("Misc", "Fly_Key", (int)global::misc::Fly_Key, path);
        writei("Misc", "Fly_Mode", (int)global::misc::Fly_Mode, path);
        writeb("Misc", "Walkspeed", global::misc::walkspeed, path);
        writef("Misc", "Walkspeed_Speed", global::misc::Walkspeed_Speed, path);
        writeb("Misc", "HitboxExpander", global::misc::hitbox, path);
        writef("Misc", "Hitbox_Size_X", global::misc::Hitbox_Size_X, path);
        writef("Misc", "Hitbox_Size_Y", global::misc::Hitbox_Size_Y, path);
        writef("Misc", "Hitbox_Size_Z", global::misc::Hitbox_Size_Z, path);
    }


    inline void load(const std::string& name)
    {
        std::string p = dir() + name + ".ini";
        if (!fs::exists(p)) return;
        const char* path = p.c_str();

        global::setting::Team_Check = readb("Settings", "Team_Check", global::setting::Team_Check, path);
        global::setting::Client_Check = readb("Settings", "Client_Check", global::setting::Client_Check, path);
        global::setting::Streamproof = readb("Settings", "Streamproof", global::setting::Streamproof, path);
        global::setting::Performance_Mode = readi("Settings", "Performance_Mode", global::setting::Performance_Mode, path);
        global::setting::Compact_UI = readb("Settings", "Compact_UI", global::setting::Compact_UI, path);
        global::setting::Sound_Enabled = readb("Settings", "Sound_Enabled", global::setting::Sound_Enabled, path);
        global::setting::Sound_Volume = readf("Settings", "Sound_Volume", global::setting::Sound_Volume, path);
        const int menuKey = readi("Settings", "Menu_Key", (int)global::setting::Menu_Key, path);
        global::setting::Menu_Key = (menuKey > ImGuiKey_None && menuKey < ImGuiKey_COUNT)
            ? (ImGuiKey)menuKey
            : ImGuiKey_Insert;
        readc("Settings", "Theme_Accent", global::setting::color::Accent, path);
        readc("Settings", "Theme_Accent2", global::setting::color::Accent2, path);
        readc("Settings", "Theme_Window", global::setting::color::Window, path);
        readc("Settings", "Theme_Card", global::setting::color::card, path);
        readc("Settings", "Theme_Text", global::setting::color::text, path);

        global::overlay::watermark = readb("Overlay", "Watermark", global::overlay::watermark, path);
        global::overlay::hotkey = readb("Overlay", "Hotkeys", global::overlay::hotkey, path);
        global::overlay::Hotkey_Aimbot = readb("Overlay", "Hotkey_Aimbot", global::overlay::Hotkey_Aimbot, path);
        global::overlay::Hotkey_Silent = readb("Overlay", "Hotkey_Silent", global::overlay::Hotkey_Silent, path);
        global::overlay::Hotkey_Fly = readb("Overlay", "Hotkey_Fly", global::overlay::Hotkey_Fly, path);
        global::overlay::Hotkey_BladeBallSpam = readb("Overlay", "Hotkey_BladeBallSpam", global::overlay::Hotkey_BladeBallSpam, path);
        global::overlay::Hotkey_Walkspeed = readb("Overlay", "Hotkey_Walkspeed", global::overlay::Hotkey_Walkspeed, path);
        global::overlay::Hotkey_HitboxExpander = readb("Overlay", "Hotkey_HitboxExpander", global::overlay::Hotkey_HitboxExpander, path);
        global::overlay::radar = readb("Overlay", "Radar", global::overlay::radar, path);
        global::overlay::AimWarning = readb("Overlay", "AimWarning", global::overlay::AimWarning, path);
        global::overlay::Watermark_Pos.x = readf("Overlay", "Watermark_X", global::overlay::Watermark_Pos.x, path);
        global::overlay::Watermark_Pos.y = readf("Overlay", "Watermark_Y", global::overlay::Watermark_Pos.y, path);
        global::overlay::Hotkeys_Pos.x = readf("Overlay", "Hotkeys_X", global::overlay::Hotkeys_Pos.x, path);
        global::overlay::Hotkeys_Pos.y = readf("Overlay", "Hotkeys_Y", global::overlay::Hotkeys_Pos.y, path);
        global::overlay::Radar_Pos.x = readf("Overlay", "Radar_X", global::overlay::Radar_Pos.x, path);
        global::overlay::Radar_Pos.y = readf("Overlay", "Radar_Y", global::overlay::Radar_Pos.y, path);
        global::overlay::Radar_Shape = readi("Overlay", "Radar_Shape", global::overlay::Radar_Shape, path);
        global::overlay::Radar_Rotate = readb("Overlay", "Radar_Rotate", global::overlay::Radar_Rotate, path);
        global::overlay::Radar_Zoom = readf("Overlay", "Radar_Zoom", global::overlay::Radar_Zoom, path);
        global::overlay::Radar_Size = readf("Overlay", "Radar_Size", global::overlay::Radar_Size, path);
        global::overlay::AimView_MaxLength = readf("Overlay", "AimView_MaxLength", global::overlay::AimView_MaxLength, path);
        readc("Overlay", "Hud_Accent", global::overlay::color::Accent, path);
        readc("Overlay", "Hud_Accent2", global::overlay::color::Accent2, path);
        readc("Overlay", "Hud_Panel", global::overlay::color::panel, path);
        readc("Overlay", "Hud_Text", global::overlay::color::text, path);

        global::ball::BallEsp = readb("BladeBall", "BallEsp", global::ball::BallEsp, path);
        global::ball::AutoParry = readb("BladeBall", "AutoParry", global::ball::AutoParry, path);
        global::ball::DrawParryRange = readb("BladeBall", "DrawParryRange", global::ball::DrawParryRange, path);
        global::ball::ClashMode = readb("BladeBall", "ClashMode", global::ball::ClashMode, path);
        global::ball::pressf = readb("BladeBall", "PressF", global::ball::pressf, path);
        global::ball::PredictTiming = readb("BladeBall", "PredictTiming", global::ball::PredictTiming, path);
        global::ball::DynamicTiming = readb("BladeBall", "DynamicTiming", global::ball::DynamicTiming, path);
        global::ball::AutoRange = readb("BladeBall", "AutoRange", global::ball::AutoRange, path);
        global::ball::AutoTiming = readb("BladeBall", "AutoTiming", global::ball::AutoTiming, path);
        global::ball::SpamParry = readb("BladeBall", "SpamParry", global::ball::SpamParry, path);
        global::ball::ParryDistance = readf("BladeBall", "ParryDistance", global::ball::ParryDistance, path);
        global::ball::DistanceBuffer = readf("BladeBall", "DistanceBuffer", global::ball::DistanceBuffer, path);
        global::ball::ParryCooldownMs = readf("BladeBall", "ParryCooldownMs", global::ball::ParryCooldownMs, path);
        global::ball::ParryTimingMs = readf("BladeBall", "ParryTimingMs", global::ball::ParryTimingMs, path);
        global::ball::MinBallSpeed = readf("BladeBall", "MinBallSpeed", global::ball::MinBallSpeed, path);
        global::ball::SpamParry_Key = (ImGuiKey)readi("BladeBall", "SpamParry_Key", (int)global::ball::SpamParry_Key, path);
        global::ball::SpamParry_Mode = (ImKeyBindMode)readi("BladeBall", "SpamParry_Mode", (int)global::ball::SpamParry_Mode, path);
        readc("BladeBall", "BallEspColor", global::ball::BallEspColor, path);
        readc("BladeBall", "ParryRangeColor", global::ball::ParryRangeColor, path);

        global::world::Skybox = readb("World", "Skybox", global::world::Skybox, path);
        global::world::Rotate = readb("World", "Rotate", global::world::Rotate, path);
        global::world::Ambience = readb("World", "Ambience", global::world::Ambience, path);
        global::world::Fog = readb("World", "Fog", global::world::Fog, path);
        global::world::Brightness = readb("World", "Brightness", global::world::Brightness, path);
        global::world::Exposure = readb("World", "Exposure", global::world::Exposure, path);
        global::world::FOV = readb("World", "FOV", global::world::FOV, path);
        global::world::Skybox_Type = readi("World", "Skybox_Type", global::world::Skybox_Type, path);
        global::world::Skybox_Rotate_Speed = readf("World", "Skybox_Rotate_Speed", global::world::Skybox_Rotate_Speed, path);
        global::world::Fog_Distance = readf("World", "Fog_Distance", global::world::Fog_Distance, path);
        global::world::FOV_Distance = readf("World", "FOV_Distance", global::world::FOV_Distance, path);
        global::world::ExposureI = readf("World", "ExposureI", global::world::ExposureI, path);
        global::world::BrightnessI = readf("World", "BrightnessI", global::world::BrightnessI, path);
        readc("World", "Ambience", global::world::color::Ambience, path);
        readc("World", "Fog", global::world::color::Fog, path);

        global::aim::Enabled = readb("Aimbot", "Enabled", global::aim::Enabled, path);
        global::aim::useFov = readb("Aimbot", "useFov", global::aim::useFov, path);
        global::aim::DrawFov = readb("Aimbot", "DrawFov", global::aim::DrawFov, path);
        global::aim::FovSpin = readb("Aimbot", "FovSpin", global::aim::FovSpin, path);
        global::aim::FillFov = readb("Aimbot", "FillFov", global::aim::FillFov, path);
        global::aim::AimbotSticky = readb("Aimbot", "AimbotSticky", global::aim::AimbotSticky, path);
        global::aim::Shake = readb("Aimbot", "Shake", global::aim::Shake, path);
        global::aim::KnockedCheck = readb("Aimbot", "KnockedCheck", global::aim::KnockedCheck, path);
        global::aim::Prediction = readb("Aimbot", "Prediction", global::aim::Prediction, path);
        global::aim::AutoPrediction = readb("Aimbot", "AutoPrediction", global::aim::AutoPrediction, path);
        global::aim::VisibleCheck = readb("Aimbot", "VisibleCheck", global::aim::VisibleCheck, path);
        global::aim::TriggerBot = readb("Aimbot", "TriggerBot", global::aim::TriggerBot, path);
        global::aim::FovSize = readf("Aimbot", "FovSize", global::aim::FovSize, path);
        global::aim::ShakeX = readf("Aimbot", "ShakeX", global::aim::ShakeX, path);
        global::aim::ShakeY = readf("Aimbot", "ShakeY", global::aim::ShakeY, path);
        global::aim::ShakeZ = readf("Aimbot", "ShakeZ", global::aim::ShakeZ, path);
        global::aim::PredictionX = readf("Aimbot", "PredictionX", global::aim::PredictionX, path);
        global::aim::PredictionY = readf("Aimbot", "PredictionY", global::aim::PredictionY, path);
        global::aim::PredictionZ = readf("Aimbot", "PredictionZ", global::aim::PredictionZ, path);
        global::aim::TriggerRadius = readf("Aimbot", "TriggerRadius", global::aim::TriggerRadius, path);
        global::aim::HitPart = readi("Aimbot", "HitPart", global::aim::HitPart, path);
        global::aim::Aimbot_type = readi("Aimbot", "Aimbot_type", global::aim::Aimbot_type, path);
        global::aim::TargetPriority = readi("Aimbot", "TargetPriority", global::aim::TargetPriority, path);
        global::aim::HitChance = readi("Aimbot", "HitChance", global::aim::HitChance, path);
        global::aim::TriggerDelayMs = readi("Aimbot", "TriggerDelayMs", global::aim::TriggerDelayMs, path);
        global::aim::FovSpinSpeed = readi("Aimbot", "FovSpinSpeed", global::aim::FovSpinSpeed, path);
        global::aim::Aimbot_Key = (ImGuiKey)readi("Aimbot", "Aimbot_Key", (int)global::aim::Aimbot_Key, path);
        global::aim::Aimbot_Mode = (ImKeyBindMode)readi("Aimbot", "Aimbot_Mode", (int)global::aim::Aimbot_Mode, path);
        readc("Aimbot", "FovColor", global::aim::FovColor, path);
        global::aim::camera::Smoothing_X = readf("Aimbot", "CamSmoothX", global::aim::camera::Smoothing_X, path);
        global::aim::camera::Smoothing_Y = readf("Aimbot", "CamSmoothY", global::aim::camera::Smoothing_Y, path);
        global::aim::mouse::Smoothing_X = readf("Aimbot", "MouseSmoothX", global::aim::mouse::Smoothing_X, path);
        global::aim::mouse::Smoothing_Y = readf("Aimbot", "MouseSmoothY", global::aim::mouse::Smoothing_Y, path);
        global::aim::mouse::Mouse_Sensitivty = readf("Aimbot", "MouseSens", global::aim::mouse::Mouse_Sensitivty, path);

        global::silent::Enabled = readb("Silent", "Enabled", global::silent::Enabled, path);
        global::silent::DrawFov = readb("Silent", "DrawFov", global::silent::DrawFov, path);
        global::silent::StickyAim = readb("Silent", "StickyAim", global::silent::StickyAim, path);
        global::silent::SpoofMouse = readb("Silent", "SpoofMouse", global::silent::SpoofMouse, path);
        global::silent::UseFov = readb("Silent", "UseFov", global::silent::UseFov, path);
        global::silent::KnockedCheck = readb("Silent", "KnockedCheck", global::silent::KnockedCheck, path);
        global::silent::GunBasedFov = readb("Silent", "GunBasedFov", global::silent::GunBasedFov, path);
        global::silent::Prediction = readb("Silent", "Prediction", global::silent::Prediction, path);
        global::silent::AutoPrediction = readb("Silent", "AutoPrediction", global::silent::AutoPrediction, path);
        global::silent::VisibleCheck = readb("Silent", "VisibleCheck", global::silent::VisibleCheck, path);
        global::silent::FovSpin = readb("Silent", "FovSpin", global::silent::FovSpin, path);
        global::silent::FillFov = readb("Silent", "FillFov", global::silent::FillFov, path);
        global::silent::fov = readf("Silent", "Fov", global::silent::fov, path);
        global::silent::FovDoubleBarrel = readf("Silent", "FovDoubleBarrel", global::silent::FovDoubleBarrel, path);
        global::silent::FovTacticalShotgun = readf("Silent", "FovTacticalShotgun", global::silent::FovTacticalShotgun, path);
        global::silent::FovRevolver = readf("Silent", "FovRevolver", global::silent::FovRevolver, path);
        global::silent::PredictionX = readf("Silent", "PredictionX", global::silent::PredictionX, path);
        global::silent::PredictionY = readf("Silent", "PredictionY", global::silent::PredictionY, path);
        global::silent::PredictionZ = readf("Silent", "PredictionZ", global::silent::PredictionZ, path);
        global::silent::AimPart = readi("Silent", "AimPart", global::silent::AimPart, path);
        global::silent::TargetPriority = readi("Silent", "TargetPriority", global::silent::TargetPriority, path);
        global::silent::FovSpinSpeed = readi("Silent", "FovSpinSpeed", global::silent::FovSpinSpeed, path);
        global::silent::Silent_Key = (ImGuiKey)readi("Silent", "Silent_Key", (int)global::silent::Silent_Key, path);
        global::silent::Silent_Mode = (ImKeyBindMode)readi("Silent", "Silent_Mode", (int)global::silent::Silent_Mode, path);
        readc("Silent", "FovColor", global::silent::FovColor, path);

        global::esp::Enabled = readb("Visuals", "Enabled", global::esp::Enabled, path);
        global::esp::Box = readb("Visuals", "Box", global::esp::Box, path);
        global::esp::Box_Fill = readb("Visuals", "Box_Fill", global::esp::Box_Fill, path);
        global::esp::Box_Fill_Gradient = readb("Visuals", "Box_Fill_Gradient", global::esp::Box_Fill_Gradient, path);
        global::esp::Box_Fill_Gradient_Rotate = readb("Visuals", "Box_Fill_Gradient_Rotate", global::esp::Box_Fill_Gradient_Rotate, path);
        global::esp::Healthbar = readb("Visuals", "Healthbar", global::esp::Healthbar, path);
        global::esp::Health = readb("Visuals", "Health", global::esp::Health, path);
        global::esp::name = readb("Visuals", "Name", global::esp::name, path);
        global::esp::Distance = readb("Visuals", "Distance", global::esp::Distance, path);
        global::esp::Rig_Type = readb("Visuals", "Rig_Type", global::esp::Rig_Type, path);
        global::esp::tool = readb("Visuals", "Tool", global::esp::tool, path);
        global::esp::Skeleton = readb("Visuals", "Skeleton", global::esp::Skeleton, path);
        global::esp::Chams = readb("Visuals", "Chams", global::esp::Chams, path);
        global::esp::ChamsFade = readb("Visuals", "ChamsFade", global::esp::ChamsFade, path);
        global::esp::Trails = readb("Visuals", "Trails", global::esp::Trails, path);
        global::esp::Chinese_Hat = readb("Visuals", "Chinese_Hat", global::esp::Chinese_Hat, path);
        global::esp::aimline = readb("Visuals", "AimLine", global::esp::aimline, path);
        global::esp::VisibleCheck = readb("Visuals", "VisibleCheck", global::esp::VisibleCheck, path);
        global::esp::Render_Distance = readf("Visuals", "Render_Distance", global::esp::Render_Distance, path);
        global::esp::ChamsFadeSpeed = readi("Visuals", "ChamsFadeSpeed", global::esp::ChamsFadeSpeed, path);
        global::esp::BoxFillSpeed = readi("Visuals", "BoxFillSpeed", global::esp::BoxFillSpeed, path);
        global::esp::Healthbar_Type = readi("Visuals", "Healthbar_Type", global::esp::Healthbar_Type, path);
        global::esp::Box_Type = readi("Visuals", "Box_Type", global::esp::Box_Type, path);
        global::esp::Box_Fill_Type = readi("Visuals", "Box_Fill_Type", global::esp::Box_Fill_Type, path);
        global::esp::Name_Type = readi("Visuals", "Name_Type", global::esp::Name_Type, path);
        global::esp::gap = readi("Visuals", "Gap", global::esp::gap, path);
        global::esp::Thickness = readi("Visuals", "Thickness", global::esp::Thickness, path);
        readc("Visuals", "Box", global::esp::color::Box, path);
        readc("Visuals", "BoxFill_Top", global::esp::color::BoxFill_Top, path);
        readc("Visuals", "BoxFill_Bottom", global::esp::color::BoxFill_Bottom, path);
        readc("Visuals", "Healthbar", global::esp::color::Healthbar, path);
        readc("Visuals", "Name", global::esp::color::name, path);
        readc("Visuals", "Distance", global::esp::color::Distance, path);
        readc("Visuals", "Rig_Type", global::esp::color::Rig_Type, path);
        readc("Visuals", "Tool", global::esp::color::tool, path);
        readc("Visuals", "Health", global::esp::color::Health, path);
        readc("Visuals", "Skeleton", global::esp::color::Skeleton, path);
        readc("Visuals", "Chams", global::esp::color::Chams, path);
        readc("Visuals", "ChamsOutline", global::esp::color::ChamsOutline, path);
        readc("Visuals", "Trails", global::esp::color::Trails, path);
        readc("Visuals", "ChineseHat", global::esp::color::hat, path);
        readc("Visuals", "AimLine", global::esp::color::aimline, path);
        readc("Visuals", "VisibleColor", global::esp::color::Visible, path);
        readc("Visuals", "NotVisibleColor", global::esp::color::NotVisible, path);
        readc("Visuals", "Healthbar_Top", global::esp::color::Healthbar_Top, path);
        readc("Visuals", "Healthbar_Middle", global::esp::color::Healthbar_Middle, path);
        readc("Visuals", "Healthbar_Bottom", global::esp::color::Healthbar_Bottom, path);

        global::misc::fly = readb("Misc", "Fly", global::misc::fly, path);
        global::misc::Fly_Speed = readf("Misc", "Fly_Speed", global::misc::Fly_Speed, path);
        global::misc::Fly_Key = (ImGuiKey)readi("Misc", "Fly_Key", (int)global::misc::Fly_Key, path);
        global::misc::Fly_Mode = (ImKeyBindMode)readi("Misc", "Fly_Mode", (int)global::misc::Fly_Mode, path);
        global::misc::walkspeed = readb("Misc", "Walkspeed", global::misc::walkspeed, path);
        global::misc::Walkspeed_Speed = readf("Misc", "Walkspeed_Speed", global::misc::Walkspeed_Speed, path);
        global::misc::hitbox = readb("Misc", "HitboxExpander", global::misc::hitbox, path);
        global::misc::Hitbox_Size_X = readf("Misc", "Hitbox_Size_X", global::misc::Hitbox_Size_X, path);
        global::misc::Hitbox_Size_Y = readf("Misc", "Hitbox_Size_Y", global::misc::Hitbox_Size_Y, path);
        global::misc::Hitbox_Size_Z = readf("Misc", "Hitbox_Size_Z", global::misc::Hitbox_Size_Z, path);
    }

    inline void save_json(const std::string& name)
    {
        nlohmann::json j;

        // Settings
        j["Settings"]["Team_Check"] = global::setting::Team_Check;
        j["Settings"]["Client_Check"] = global::setting::Client_Check;
        j["Settings"]["Streamproof"] = global::setting::Streamproof;
        j["Settings"]["Performance_Mode"] = global::setting::Performance_Mode;
        j["Settings"]["Compact_UI"] = global::setting::Compact_UI;
        j["Settings"]["Menu_Key"] = (int)global::setting::Menu_Key;
        j["Settings"]["Sound_Enabled"] = global::setting::Sound_Enabled;
        j["Settings"]["Sound_Volume"] = global::setting::Sound_Volume;
        j["Settings"]["Theme_Accent"] = { global::setting::color::Accent[0], global::setting::color::Accent[1], global::setting::color::Accent[2], global::setting::color::Accent[3] };
        j["Settings"]["Theme_Accent2"] = { global::setting::color::Accent2[0], global::setting::color::Accent2[1], global::setting::color::Accent2[2], global::setting::color::Accent2[3] };
        j["Settings"]["Theme_Window"] = { global::setting::color::Window[0], global::setting::color::Window[1], global::setting::color::Window[2], global::setting::color::Window[3] };
        j["Settings"]["Theme_Card"] = { global::setting::color::card[0], global::setting::color::card[1], global::setting::color::card[2], global::setting::color::card[3] };
        j["Settings"]["Theme_Text"] = { global::setting::color::text[0], global::setting::color::text[1], global::setting::color::text[2], global::setting::color::text[3] };

        // Overlay
        j["Overlay"]["Watermark"] = global::overlay::watermark;
        j["Overlay"]["Hotkeys"] = global::overlay::hotkey;
        j["Overlay"]["Hotkey_Aimbot"] = global::overlay::Hotkey_Aimbot;
        j["Overlay"]["Hotkey_Silent"] = global::overlay::Hotkey_Silent;
        j["Overlay"]["Hotkey_Fly"] = global::overlay::Hotkey_Fly;
        j["Overlay"]["Hotkey_BladeBallSpam"] = global::overlay::Hotkey_BladeBallSpam;
        j["Overlay"]["Hotkey_Walkspeed"] = global::overlay::Hotkey_Walkspeed;
        j["Overlay"]["Hotkey_HitboxExpander"] = global::overlay::Hotkey_HitboxExpander;
        j["Overlay"]["Radar"] = global::overlay::radar;
        j["Overlay"]["AimWarning"] = global::overlay::AimWarning;
        j["Overlay"]["Watermark_X"] = global::overlay::Watermark_Pos.x;
        j["Overlay"]["Watermark_Y"] = global::overlay::Watermark_Pos.y;
        j["Overlay"]["Hotkeys_X"] = global::overlay::Hotkeys_Pos.x;
        j["Overlay"]["Hotkeys_Y"] = global::overlay::Hotkeys_Pos.y;
        j["Overlay"]["Radar_X"] = global::overlay::Radar_Pos.x;
        j["Overlay"]["Radar_Y"] = global::overlay::Radar_Pos.y;
        j["Overlay"]["Radar_Shape"] = global::overlay::Radar_Shape;
        j["Overlay"]["Radar_Rotate"] = global::overlay::Radar_Rotate;
        j["Overlay"]["Radar_Zoom"] = global::overlay::Radar_Zoom;
        j["Overlay"]["Radar_Size"] = global::overlay::Radar_Size;
        j["Overlay"]["AimView_MaxLength"] = global::overlay::AimView_MaxLength;
        j["Overlay"]["Hud_Accent"] = { global::overlay::color::Accent[0], global::overlay::color::Accent[1], global::overlay::color::Accent[2], global::overlay::color::Accent[3] };
        j["Overlay"]["Hud_Accent2"] = { global::overlay::color::Accent2[0], global::overlay::color::Accent2[1], global::overlay::color::Accent2[2], global::overlay::color::Accent2[3] };
        j["Overlay"]["Hud_Panel"] = { global::overlay::color::panel[0], global::overlay::color::panel[1], global::overlay::color::panel[2], global::overlay::color::panel[3] };
        j["Overlay"]["Hud_Text"] = { global::overlay::color::text[0], global::overlay::color::text[1], global::overlay::color::text[2], global::overlay::color::text[3] };

        // BladeBall
        j["BladeBall"]["BallEsp"] = global::ball::BallEsp;
        j["BladeBall"]["AutoParry"] = global::ball::AutoParry;
        j["BladeBall"]["DrawParryRange"] = global::ball::DrawParryRange;
        j["BladeBall"]["ClashMode"] = global::ball::ClashMode;
        j["BladeBall"]["PressF"] = global::ball::pressf;
        j["BladeBall"]["PredictTiming"] = global::ball::PredictTiming;
        j["BladeBall"]["DynamicTiming"] = global::ball::DynamicTiming;
        j["BladeBall"]["AutoRange"] = global::ball::AutoRange;
        j["BladeBall"]["AutoTiming"] = global::ball::AutoTiming;
        j["BladeBall"]["SpamParry"] = global::ball::SpamParry;
        j["BladeBall"]["ParryDistance"] = global::ball::ParryDistance;
        j["BladeBall"]["DistanceBuffer"] = global::ball::DistanceBuffer;
        j["BladeBall"]["ParryCooldownMs"] = global::ball::ParryCooldownMs;
        j["BladeBall"]["ParryTimingMs"] = global::ball::ParryTimingMs;
        j["BladeBall"]["MinBallSpeed"] = global::ball::MinBallSpeed;
        j["BladeBall"]["SpamParry_Key"] = (int)global::ball::SpamParry_Key;
        j["BladeBall"]["SpamParry_Mode"] = (int)global::ball::SpamParry_Mode;
        j["BladeBall"]["BallEspColor"] = { global::ball::BallEspColor[0], global::ball::BallEspColor[1], global::ball::BallEspColor[2], global::ball::BallEspColor[3] };
        j["BladeBall"]["ParryRangeColor"] = { global::ball::ParryRangeColor[0], global::ball::ParryRangeColor[1], global::ball::ParryRangeColor[2], global::ball::ParryRangeColor[3] };

        // Aimbot
        j["Aimbot"]["UseFov"] = global::aim::useFov;
        j["Aimbot"]["Enabled"] = global::aim::Enabled;
        j["Aimbot"]["DrawFov"] = global::aim::DrawFov;
        j["Aimbot"]["FovSpin"] = global::aim::FovSpin;
        j["Aimbot"]["FillFov"] = global::aim::FillFov;
        j["Aimbot"]["AimbotSticky"] = global::aim::AimbotSticky;
        j["Aimbot"]["Shake"] = global::aim::Shake;
        j["Aimbot"]["KnockedCheck"] = global::aim::KnockedCheck;
        j["Aimbot"]["Prediction"] = global::aim::Prediction;
        j["Aimbot"]["AutoPrediction"] = global::aim::AutoPrediction;
        j["Aimbot"]["VisibleCheck"] = global::aim::VisibleCheck;
        j["Aimbot"]["TriggerBot"] = global::aim::TriggerBot;
        j["Aimbot"]["FovSize"] = global::aim::FovSize;
        j["Aimbot"]["ShakeX"] = global::aim::ShakeX;
        j["Aimbot"]["ShakeY"] = global::aim::ShakeY;
        j["Aimbot"]["ShakeZ"] = global::aim::ShakeZ;
        j["Aimbot"]["PredictionX"] = global::aim::PredictionX;
        j["Aimbot"]["PredictionY"] = global::aim::PredictionY;
        j["Aimbot"]["PredictionZ"] = global::aim::PredictionZ;
        j["Aimbot"]["TriggerRadius"] = global::aim::TriggerRadius;
        j["Aimbot"]["HitPart"] = global::aim::HitPart;
        j["Aimbot"]["Aimbot_type"] = global::aim::Aimbot_type;
        j["Aimbot"]["TargetPriority"] = global::aim::TargetPriority;
        j["Aimbot"]["HitChance"] = global::aim::HitChance;
        j["Aimbot"]["TriggerDelayMs"] = global::aim::TriggerDelayMs;
        j["Aimbot"]["FovSpinSpeed"] = global::aim::FovSpinSpeed;
        j["Aimbot"]["Aimbot_Key"] = (int)global::aim::Aimbot_Key;
        j["Aimbot"]["Aimbot_Mode"] = (int)global::aim::Aimbot_Mode;
        j["Aimbot"]["FovColor"] = { global::aim::FovColor[0], global::aim::FovColor[1], global::aim::FovColor[2], global::aim::FovColor[3] };
        j["Aimbot"]["CamSmoothX"] = global::aim::camera::Smoothing_X;
        j["Aimbot"]["CamSmoothY"] = global::aim::camera::Smoothing_Y;
        j["Aimbot"]["MouseSmoothX"] = global::aim::mouse::Smoothing_X;
        j["Aimbot"]["MouseSmoothY"] = global::aim::mouse::Smoothing_Y;
        j["Aimbot"]["MouseSens"] = global::aim::mouse::Mouse_Sensitivty;

        // Silent
        j["Silent"]["Enabled"] = global::silent::Enabled;
        j["Silent"]["DrawFov"] = global::silent::DrawFov;
        j["Silent"]["StickyAim"] = global::silent::StickyAim;
        j["Silent"]["SpoofMouse"] = global::silent::SpoofMouse;
        j["Silent"]["UseFov"] = global::silent::UseFov;
        j["Silent"]["KnockedCheck"] = global::silent::KnockedCheck;
        j["Silent"]["GunBasedFov"] = global::silent::GunBasedFov;
        j["Silent"]["Prediction"] = global::silent::Prediction;
        j["Silent"]["AutoPrediction"] = global::silent::AutoPrediction;
        j["Silent"]["VisibleCheck"] = global::silent::VisibleCheck;
        j["Silent"]["FovSpin"] = global::silent::FovSpin;
        j["Silent"]["FillFov"] = global::silent::FillFov;
        j["Silent"]["Fov"] = global::silent::fov;
        j["Silent"]["FovDoubleBarrel"] = global::silent::FovDoubleBarrel;
        j["Silent"]["FovTacticalShotgun"] = global::silent::FovTacticalShotgun;
        j["Silent"]["FovRevolver"] = global::silent::FovRevolver;
        j["Silent"]["PredictionX"] = global::silent::PredictionX;
        j["Silent"]["PredictionY"] = global::silent::PredictionY;
        j["Silent"]["PredictionZ"] = global::silent::PredictionZ;
        j["Silent"]["AimPart"] = global::silent::AimPart;
        j["Silent"]["TargetPriority"] = global::silent::TargetPriority;
        j["Silent"]["FovSpinSpeed"] = global::silent::FovSpinSpeed;
        j["Silent"]["Silent_Key"] = (int)global::silent::Silent_Key;
        j["Silent"]["Silent_Mode"] = (int)global::silent::Silent_Mode;
        j["Silent"]["FovColor"] = { global::silent::FovColor[0], global::silent::FovColor[1], global::silent::FovColor[2], global::silent::FovColor[3] };

        // Visuals
        j["Visuals"]["Enabled"] = global::esp::Enabled;
        j["Visuals"]["Box"] = global::esp::Box;
        j["Visuals"]["Box_Fill"] = global::esp::Box_Fill;
        j["Visuals"]["Box_Fill_Gradient"] = global::esp::Box_Fill_Gradient;
        j["Visuals"]["Box_Fill_Gradient_Rotate"] = global::esp::Box_Fill_Gradient_Rotate;
        j["Visuals"]["Healthbar"] = global::esp::Healthbar;
        j["Visuals"]["Health"] = global::esp::Health;
        j["Visuals"]["Name"] = global::esp::name;
        j["Visuals"]["Distance"] = global::esp::Distance;
        j["Visuals"]["Rig_Type"] = global::esp::Rig_Type;
        j["Visuals"]["Tool"] = global::esp::tool;
        j["Visuals"]["Skeleton"] = global::esp::Skeleton;
        j["Visuals"]["Chams"] = global::esp::Chams;
        j["Visuals"]["ChamsFade"] = global::esp::ChamsFade;
        j["Visuals"]["Trails"] = global::esp::Trails;
        j["Visuals"]["Chinese_Hat"] = global::esp::Chinese_Hat;
        j["Visuals"]["AimLine"] = global::esp::aimline;
        j["Visuals"]["VisibleCheck"] = global::esp::VisibleCheck;
        j["Visuals"]["Render_Distance"] = global::esp::Render_Distance;
        j["Visuals"]["ChamsFadeSpeed"] = global::esp::ChamsFadeSpeed;
        j["Visuals"]["BoxFillSpeed"] = global::esp::BoxFillSpeed;
        j["Visuals"]["Healthbar_Type"] = global::esp::Healthbar_Type;
        j["Visuals"]["Box_Type"] = global::esp::Box_Type;
        j["Visuals"]["Box_Fill_Type"] = global::esp::Box_Fill_Type;
        j["Visuals"]["Name_Type"] = global::esp::Name_Type;
        j["Visuals"]["Gap"] = global::esp::gap;
        j["Visuals"]["Thickness"] = global::esp::Thickness;
        j["Visuals"]["Color_Box"] = { global::esp::color::Box[0], global::esp::color::Box[1], global::esp::color::Box[2], global::esp::color::Box[3] };
        j["Visuals"]["Color_BoxFill_Top"] = { global::esp::color::BoxFill_Top[0], global::esp::color::BoxFill_Top[1], global::esp::color::BoxFill_Top[2], global::esp::color::BoxFill_Top[3] };
        j["Visuals"]["Color_BoxFill_Bottom"] = { global::esp::color::BoxFill_Bottom[0], global::esp::color::BoxFill_Bottom[1], global::esp::color::BoxFill_Bottom[2], global::esp::color::BoxFill_Bottom[3] };
        j["Visuals"]["Color_Healthbar"] = { global::esp::color::Healthbar[0], global::esp::color::Healthbar[1], global::esp::color::Healthbar[2], global::esp::color::Healthbar[3] };
        j["Visuals"]["Color_Name"] = { global::esp::color::name[0], global::esp::color::name[1], global::esp::color::name[2], global::esp::color::name[3] };
        j["Visuals"]["Color_Distance"] = { global::esp::color::Distance[0], global::esp::color::Distance[1], global::esp::color::Distance[2], global::esp::color::Distance[3] };
        j["Visuals"]["Color_Rig_Type"] = { global::esp::color::Rig_Type[0], global::esp::color::Rig_Type[1], global::esp::color::Rig_Type[2], global::esp::color::Rig_Type[3] };
        j["Visuals"]["Color_Tool"] = { global::esp::color::tool[0], global::esp::color::tool[1], global::esp::color::tool[2], global::esp::color::tool[3] };
        j["Visuals"]["Color_Health"] = { global::esp::color::Health[0], global::esp::color::Health[1], global::esp::color::Health[2], global::esp::color::Health[3] };
        j["Visuals"]["Color_Skeleton"] = { global::esp::color::Skeleton[0], global::esp::color::Skeleton[1], global::esp::color::Skeleton[2], global::esp::color::Skeleton[3] };
        j["Visuals"]["Color_Chams"] = { global::esp::color::Chams[0], global::esp::color::Chams[1], global::esp::color::Chams[2], global::esp::color::Chams[3] };
        j["Visuals"]["Color_ChamsOutline"] = { global::esp::color::ChamsOutline[0], global::esp::color::ChamsOutline[1], global::esp::color::ChamsOutline[2], global::esp::color::ChamsOutline[3] };
        j["Visuals"]["Color_Trails"] = { global::esp::color::Trails[0], global::esp::color::Trails[1], global::esp::color::Trails[2], global::esp::color::Trails[3] };
        j["Visuals"]["Color_ChineseHat"] = { global::esp::color::hat[0], global::esp::color::hat[1], global::esp::color::hat[2], global::esp::color::hat[3] };
        j["Visuals"]["Color_AimLine"] = { global::esp::color::aimline[0], global::esp::color::aimline[1], global::esp::color::aimline[2], global::esp::color::aimline[3] };
        j["Visuals"]["Color_Visible"] = { global::esp::color::Visible[0], global::esp::color::Visible[1], global::esp::color::Visible[2], global::esp::color::Visible[3] };
        j["Visuals"]["Color_NotVisible"] = { global::esp::color::NotVisible[0], global::esp::color::NotVisible[1], global::esp::color::NotVisible[2], global::esp::color::NotVisible[3] };
        j["Visuals"]["Color_Healthbar_Top"] = { global::esp::color::Healthbar_Top[0], global::esp::color::Healthbar_Top[1], global::esp::color::Healthbar_Top[2], global::esp::color::Healthbar_Top[3] };
        j["Visuals"]["Color_Healthbar_Middle"] = { global::esp::color::Healthbar_Middle[0], global::esp::color::Healthbar_Middle[1], global::esp::color::Healthbar_Middle[2], global::esp::color::Healthbar_Middle[3] };
        j["Visuals"]["Color_Healthbar_Bottom"] = { global::esp::color::Healthbar_Bottom[0], global::esp::color::Healthbar_Bottom[1], global::esp::color::Healthbar_Bottom[2], global::esp::color::Healthbar_Bottom[3] };

        // Misc
        j["Misc"]["Fly"] = global::misc::fly;
        j["Misc"]["Fly_Speed"] = global::misc::Fly_Speed;
        j["Misc"]["Fly_Key"] = (int)global::misc::Fly_Key;
        j["Misc"]["Fly_Mode"] = (int)global::misc::Fly_Mode;
        j["Misc"]["Walkspeed"] = global::misc::walkspeed;
        j["Misc"]["Walkspeed_Speed"] = global::misc::Walkspeed_Speed;
        j["Misc"]["HitboxExpander"] = global::misc::hitbox;
        j["Misc"]["Hitbox_Size_X"] = global::misc::Hitbox_Size_X;
        j["Misc"]["Hitbox_Size_Y"] = global::misc::Hitbox_Size_Y;
        j["Misc"]["Hitbox_Size_Z"] = global::misc::Hitbox_Size_Z;

        std::string p = dir() + name + ".json";
        std::ofstream f(p);
        if (f)
        {
            f << j.dump(4);
        }
    }

    inline void load_json(const std::string& name)
    {
        std::string p = dir() + name + ".json";
        std::ifstream f(p);
        if (!f) return;

        nlohmann::json j;
        try {
            f >> j;
        }
        catch (...) {
            return;
        }

        auto read_bool = [&](const std::string& sec, const std::string& key, bool& val) {
            if (j.contains(sec) && j[sec].contains(key) && j[sec][key].is_boolean())
                val = j[sec][key].get<bool>();
        };
        auto read_float = [&](const std::string& sec, const std::string& key, float& val) {
            if (j.contains(sec) && j[sec].contains(key) && j[sec][key].is_number())
                val = j[sec][key].get<float>();
        };
        auto read_int = [&](const std::string& sec, const std::string& key, int& val) {
            if (j.contains(sec) && j[sec].contains(key) && j[sec][key].is_number())
                val = j[sec][key].get<int>();
        };
        auto read_color = [&](const std::string& sec, const std::string& key, float col[4]) {
            if (j.contains(sec) && j[sec].contains(key) && j[sec][key].is_array() && j[sec][key].size() == 4) {
                for (int i = 0; i < 4; i++) {
                    col[i] = j[sec][key][i].get<float>();
                }
            }
        };

        // Settings
        read_bool("Settings", "Team_Check", global::setting::Team_Check);
        read_bool("Settings", "Client_Check", global::setting::Client_Check);
        read_bool("Settings", "Streamproof", global::setting::Streamproof);
        read_int("Settings", "Performance_Mode", global::setting::Performance_Mode);
        read_bool("Settings", "Compact_UI", global::setting::Compact_UI);
        int menu_key_val = (int)global::setting::Menu_Key;
        read_int("Settings", "Menu_Key", menu_key_val);
        global::setting::Menu_Key = (ImGuiKey)menu_key_val;
        read_bool("Settings", "Sound_Enabled", global::setting::Sound_Enabled);
        read_float("Settings", "Sound_Volume", global::setting::Sound_Volume);
        read_color("Settings", "Theme_Accent", global::setting::color::Accent);
        read_color("Settings", "Theme_Accent2", global::setting::color::Accent2);
        read_color("Settings", "Theme_Window", global::setting::color::Window);
        read_color("Settings", "Theme_Card", global::setting::color::card);
        read_color("Settings", "Theme_Text", global::setting::color::text);

        // Overlay
        read_bool("Overlay", "Watermark", global::overlay::watermark);
        read_bool("Overlay", "Hotkeys", global::overlay::hotkey);
        read_bool("Overlay", "Hotkey_Aimbot", global::overlay::Hotkey_Aimbot);
        read_bool("Overlay", "Hotkey_Silent", global::overlay::Hotkey_Silent);
        read_bool("Overlay", "Hotkey_Fly", global::overlay::Hotkey_Fly);
        read_bool("Overlay", "Hotkey_BladeBallSpam", global::overlay::Hotkey_BladeBallSpam);
        read_bool("Overlay", "Hotkey_Walkspeed", global::overlay::Hotkey_Walkspeed);
        read_bool("Overlay", "Hotkey_HitboxExpander", global::overlay::Hotkey_HitboxExpander);
        read_bool("Overlay", "Radar", global::overlay::radar);
        read_bool("Overlay", "AimWarning", global::overlay::AimWarning);
        read_float("Overlay", "Watermark_X", global::overlay::Watermark_Pos.x);
        read_float("Overlay", "Watermark_Y", global::overlay::Watermark_Pos.y);
        read_float("Overlay", "Hotkeys_X", global::overlay::Hotkeys_Pos.x);
        read_float("Overlay", "Hotkeys_Y", global::overlay::Hotkeys_Pos.y);
        read_float("Overlay", "Radar_X", global::overlay::Radar_Pos.x);
        read_float("Overlay", "Radar_Y", global::overlay::Radar_Pos.y);
        read_int("Overlay", "Radar_Shape", global::overlay::Radar_Shape);
        read_bool("Overlay", "Radar_Rotate", global::overlay::Radar_Rotate);
        read_float("Overlay", "Radar_Zoom", global::overlay::Radar_Zoom);
        read_float("Overlay", "Radar_Size", global::overlay::Radar_Size);
        read_float("Overlay", "AimView_MaxLength", global::overlay::AimView_MaxLength);
        read_color("Overlay", "Hud_Accent", global::overlay::color::Accent);
        read_color("Overlay", "Hud_Accent2", global::overlay::color::Accent2);
        read_color("Overlay", "Hud_Panel", global::overlay::color::panel);
        read_color("Overlay", "Hud_Text", global::overlay::color::text);

        // BladeBall
        read_bool("BladeBall", "BallEsp", global::ball::BallEsp);
        read_bool("BladeBall", "AutoParry", global::ball::AutoParry);
        read_bool("BladeBall", "DrawParryRange", global::ball::DrawParryRange);
        read_bool("BladeBall", "ClashMode", global::ball::ClashMode);
        read_bool("BladeBall", "PressF", global::ball::pressf);
        read_bool("BladeBall", "PredictTiming", global::ball::PredictTiming);
        read_bool("BladeBall", "DynamicTiming", global::ball::DynamicTiming);
        read_bool("BladeBall", "AutoRange", global::ball::AutoRange);
        read_bool("BladeBall", "AutoTiming", global::ball::AutoTiming);
        read_bool("BladeBall", "SpamParry", global::ball::SpamParry);
        read_float("BladeBall", "ParryDistance", global::ball::ParryDistance);
        read_float("BladeBall", "DistanceBuffer", global::ball::DistanceBuffer);
        read_float("BladeBall", "ParryCooldownMs", global::ball::ParryCooldownMs);
        read_float("BladeBall", "ParryTimingMs", global::ball::ParryTimingMs);
        read_float("BladeBall", "MinBallSpeed", global::ball::MinBallSpeed);
        int sp_key = (int)global::ball::SpamParry_Key;
        read_int("BladeBall", "SpamParry_Key", sp_key);
        global::ball::SpamParry_Key = (ImGuiKey)sp_key;
        int sp_mode = (int)global::ball::SpamParry_Mode;
        read_int("BladeBall", "SpamParry_Mode", sp_mode);
        global::ball::SpamParry_Mode = (ImKeyBindMode)sp_mode;
        read_color("BladeBall", "BallEspColor", global::ball::BallEspColor);
        read_color("BladeBall", "ParryRangeColor", global::ball::ParryRangeColor);

        // Aimbot
        read_bool("Aimbot", "UseFov", global::aim::useFov);
        read_bool("Aimbot", "Enabled", global::aim::Enabled);
        read_bool("Aimbot", "DrawFov", global::aim::DrawFov);
        read_bool("Aimbot", "FovSpin", global::aim::FovSpin);
        read_bool("Aimbot", "FillFov", global::aim::FillFov);
        read_bool("Aimbot", "AimbotSticky", global::aim::AimbotSticky);
        read_bool("Aimbot", "Shake", global::aim::Shake);
        read_bool("Aimbot", "KnockedCheck", global::aim::KnockedCheck);
        read_bool("Aimbot", "Prediction", global::aim::Prediction);
        read_bool("Aimbot", "AutoPrediction", global::aim::AutoPrediction);
        read_bool("Aimbot", "VisibleCheck", global::aim::VisibleCheck);
        read_bool("Aimbot", "TriggerBot", global::aim::TriggerBot);
        read_float("Aimbot", "FovSize", global::aim::FovSize);
        read_float("Aimbot", "ShakeX", global::aim::ShakeX);
        read_float("Aimbot", "ShakeY", global::aim::ShakeY);
        read_float("Aimbot", "ShakeZ", global::aim::ShakeZ);
        read_float("Aimbot", "PredictionX", global::aim::PredictionX);
        read_float("Aimbot", "PredictionY", global::aim::PredictionY);
        read_float("Aimbot", "PredictionZ", global::aim::PredictionZ);
        read_float("Aimbot", "TriggerRadius", global::aim::TriggerRadius);
        read_int("Aimbot", "HitPart", global::aim::HitPart);
        read_int("Aimbot", "Aimbot_type", global::aim::Aimbot_type);
        read_int("Aimbot", "TargetPriority", global::aim::TargetPriority);
        read_int("Aimbot", "HitChance", global::aim::HitChance);
        read_int("Aimbot", "TriggerDelayMs", global::aim::TriggerDelayMs);
        read_int("Aimbot", "FovSpinSpeed", global::aim::FovSpinSpeed);
        int aim_key = (int)global::aim::Aimbot_Key;
        read_int("Aimbot", "Aimbot_Key", aim_key);
        global::aim::Aimbot_Key = (ImGuiKey)aim_key;
        int aim_mode = (int)global::aim::Aimbot_Mode;
        read_int("Aimbot", "Aimbot_Mode", aim_mode);
        global::aim::Aimbot_Mode = (ImKeyBindMode)aim_mode;
        read_color("Aimbot", "FovColor", global::aim::FovColor);
        read_float("Aimbot", "CamSmoothX", global::aim::camera::Smoothing_X);
        read_float("Aimbot", "CamSmoothY", global::aim::camera::Smoothing_Y);
        read_float("Aimbot", "MouseSmoothX", global::aim::mouse::Smoothing_X);
        read_float("Aimbot", "MouseSmoothY", global::aim::mouse::Smoothing_Y);
        read_float("Aimbot", "MouseSens", global::aim::mouse::Mouse_Sensitivty);

        // Silent
        read_bool("Silent", "Enabled", global::silent::Enabled);
        read_bool("Silent", "DrawFov", global::silent::DrawFov);
        read_bool("Silent", "StickyAim", global::silent::StickyAim);
        read_bool("Silent", "SpoofMouse", global::silent::SpoofMouse);
        read_bool("Silent", "UseFov", global::silent::UseFov);
        read_bool("Silent", "KnockedCheck", global::silent::KnockedCheck);
        read_bool("Silent", "GunBasedFov", global::silent::GunBasedFov);
        read_bool("Silent", "Prediction", global::silent::Prediction);
        read_bool("Silent", "AutoPrediction", global::silent::AutoPrediction);
        read_bool("Silent", "VisibleCheck", global::silent::VisibleCheck);
        read_bool("Silent", "FovSpin", global::silent::FovSpin);
        read_bool("Silent", "FillFov", global::silent::FillFov);
        read_float("Silent", "Fov", global::silent::fov);
        read_float("Silent", "FovDoubleBarrel", global::silent::FovDoubleBarrel);
        read_float("Silent", "FovTacticalShotgun", global::silent::FovTacticalShotgun);
        read_float("Silent", "FovRevolver", global::silent::FovRevolver);
        read_float("Silent", "PredictionX", global::silent::PredictionX);
        read_float("Silent", "PredictionY", global::silent::PredictionY);
        read_float("Silent", "PredictionZ", global::silent::PredictionZ);
        read_int("Silent", "AimPart", global::silent::AimPart);
        read_int("Silent", "TargetPriority", global::silent::TargetPriority);
        read_int("Silent", "FovSpinSpeed", global::silent::FovSpinSpeed);
        int silent_key = (int)global::silent::Silent_Key;
        read_int("Silent", "Silent_Key", silent_key);
        global::silent::Silent_Key = (ImGuiKey)silent_key;
        int silent_mode = (int)global::silent::Silent_Mode;
        read_int("Silent", "Silent_Mode", silent_mode);
        global::silent::Silent_Mode = (ImKeyBindMode)silent_mode;
        read_color("Silent", "FovColor", global::silent::FovColor);

        // Visuals
        read_bool("Visuals", "Enabled", global::esp::Enabled);
        read_bool("Visuals", "Box", global::esp::Box);
        read_bool("Visuals", "Box_Fill", global::esp::Box_Fill);
        read_bool("Visuals", "Box_Fill_Gradient", global::esp::Box_Fill_Gradient);
        read_bool("Visuals", "Box_Fill_Gradient_Rotate", global::esp::Box_Fill_Gradient_Rotate);
        read_bool("Visuals", "Healthbar", global::esp::Healthbar);
        read_bool("Visuals", "Health", global::esp::Health);
        read_bool("Visuals", "Name", global::esp::name);
        read_bool("Visuals", "Distance", global::esp::Distance);
        read_bool("Visuals", "Rig_Type", global::esp::Rig_Type);
        read_bool("Visuals", "Tool", global::esp::tool);
        read_bool("Visuals", "Skeleton", global::esp::Skeleton);
        read_bool("Visuals", "Chams", global::esp::Chams);
        read_bool("Visuals", "ChamsFade", global::esp::ChamsFade);
        read_bool("Visuals", "Trails", global::esp::Trails);
        read_bool("Visuals", "Chinese_Hat", global::esp::Chinese_Hat);
        read_bool("Visuals", "AimLine", global::esp::aimline);
        read_bool("Visuals", "VisibleCheck", global::esp::VisibleCheck);
        read_float("Visuals", "Render_Distance", global::esp::Render_Distance);
        read_int("Visuals", "ChamsFadeSpeed", global::esp::ChamsFadeSpeed);
        read_int("Visuals", "BoxFillSpeed", global::esp::BoxFillSpeed);
        read_int("Visuals", "Healthbar_Type", global::esp::Healthbar_Type);
        read_int("Visuals", "Box_Type", global::esp::Box_Type);
        read_int("Visuals", "Box_Fill_Type", global::esp::Box_Fill_Type);
        read_int("Visuals", "Name_Type", global::esp::Name_Type);
        read_int("Visuals", "Gap", global::esp::gap);
        read_int("Visuals", "Thickness", global::esp::Thickness);
        read_color("Visuals", "Color_Box", global::esp::color::Box);
        read_color("Visuals", "Color_BoxFill_Top", global::esp::color::BoxFill_Top);
        read_color("Visuals", "Color_BoxFill_Bottom", global::esp::color::BoxFill_Bottom);
        read_color("Visuals", "Color_Healthbar", global::esp::color::Healthbar);
        read_color("Visuals", "Color_Name", global::esp::color::name);
        read_color("Visuals", "Color_Distance", global::esp::color::Distance);
        read_color("Visuals", "Color_Rig_Type", global::esp::color::Rig_Type);
        read_color("Visuals", "Color_Tool", global::esp::color::tool);
        read_color("Visuals", "Color_Health", global::esp::color::Health);
        read_color("Visuals", "Color_Skeleton", global::esp::color::Skeleton);
        read_color("Visuals", "Color_Chams", global::esp::color::Chams);
        read_color("Visuals", "Color_ChamsOutline", global::esp::color::ChamsOutline);
        read_color("Visuals", "Color_Trails", global::esp::color::Trails);
        read_color("Visuals", "Color_ChineseHat", global::esp::color::hat);
        read_color("Visuals", "Color_AimLine", global::esp::color::aimline);
        read_color("Visuals", "Color_Visible", global::esp::color::Visible);
        read_color("Visuals", "Color_NotVisible", global::esp::color::NotVisible);
        read_color("Visuals", "Color_Healthbar_Top", global::esp::color::Healthbar_Top);
        read_color("Visuals", "Color_Healthbar_Middle", global::esp::color::Healthbar_Middle);
        read_color("Visuals", "Color_Healthbar_Bottom", global::esp::color::Healthbar_Bottom);

        // Misc
        read_bool("Misc", "Fly", global::misc::fly);
        read_float("Misc", "Fly_Speed", global::misc::Fly_Speed);
        int fly_key = (int)global::misc::Fly_Key;
        read_int("Misc", "Fly_Key", fly_key);
        global::misc::Fly_Key = (ImGuiKey)fly_key;
        int fly_mode = (int)global::misc::Fly_Mode;
        read_int("Misc", "Fly_Mode", fly_mode);
        global::misc::Fly_Mode = (ImKeyBindMode)fly_mode;
        read_bool("Misc", "Walkspeed", global::misc::walkspeed);
        read_float("Misc", "Walkspeed_Speed", global::misc::Walkspeed_Speed);
        read_bool("Misc", "HitboxExpander", global::misc::hitbox);
        read_float("Misc", "Hitbox_Size_X", global::misc::Hitbox_Size_X);
        read_float("Misc", "Hitbox_Size_Y", global::misc::Hitbox_Size_Y);
        read_float("Misc", "Hitbox_Size_Z", global::misc::Hitbox_Size_Z);
    }
}
