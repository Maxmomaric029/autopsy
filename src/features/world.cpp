#include "../sdk/sdk.h"
#include "global.h"
#include "../core/cache.h"
#include <iostream>

namespace world {

    // Saved original values for restoration when toggling off
    static float s_origBrightness = 0.f;
    static float s_origExposure = 0.f;
    static float s_origFogEnd = 0.f;
    static sdk::vector3 s_origFogColor{};
    static sdk::vector3 s_origAmbient{};
    static sdk::vector3 s_origOutdoorAmbient{};
    static bool s_brightnessSaved = false;
    static bool s_exposureSaved = false;
    static bool s_fogSaved = false;
    static bool s_ambienceSaved = false;

    void skybox() {

        static int lastType = -1;
        static bool lastEnabled = false;

        while (true)
        {
            try {
            // Safety: verify light address is valid
            if (!global::light.Address) { std::this_thread::sleep_for(std::chrono::milliseconds(200)); continue; }

            // Only write skybox strings when type CHANGES, not every frame
            bool needsUpdate = (global::world::Skybox != lastEnabled) ||
                               (global::world::Skybox && global::world::Skybox_Type != lastType);

            if (global::world::Skybox && global::light.Address)
            {
                // Wrap ALL Sky operations in a nested try/catch for safety
                sdk::instance sky;
                try {
                    sky = global::light.child("Sky");
                    if (!sky.Address || sky.Address == 0xFFFFFFFFFFFFFFFF)
                        sky = global::light.childclass("Sky");
                } catch (...) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    continue;
                }

                // Verify Sky address is valid before any read/write
                if (!sky.Address || sky.Address == 0xFFFFFFFFFFFFFFFF)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    continue;
                }

                // Verify Sky object kind is actually "Sky" (avoid writing to wrong instance)
                std::string skyKind;
                try {
                    skyKind = sky.kind();
                } catch (...) { skyKind = ""; }
                if (skyKind != "Sky") {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    continue;
                }

                // Only write textures when selection changes
                if (needsUpdate && sky.Address)
                {
                    lastType = global::world::Skybox_Type;
                    lastEnabled = global::world::Skybox;

                    // Array of skybox presets: {Bk, Dn, Ft, Lf, Rt, Up}
                    struct Preset { const char* bk, *dn, *ft, *lf, *rt, *up; };
                    static const Preset presets[] = {
                        {"rbxassetid://12635309703","rbxassetid://12635311686","rbxassetid://12635312870","rbxassetid://12635313718","rbxassetid://12635315817","rbxassetid://12635316856"},
                        {"rbxassetid://12064107","rbxassetid://12064152","rbxassetid://12064121","rbxassetid://12063984","rbxassetid://12064115","rbxassetid://12064131"},
                        {"rbxassetid://271042516","rbxassetid://271077243","rbxassetid://271042556","rbxassetid://271042310","rbxassetid://271042467","rbxassetid://271077958"},
                        {"rbxassetid://1876545003","rbxassetid://1876544331","rbxassetid://1876542941","rbxassetid://1876543392","rbxassetid://1876543764","rbxassetid://1876544642"},
                        {"rbxassetid://116758234","rbxassetid://116758314","rbxassetid://116758367","rbxassetid://116758446","rbxassetid://116758478","rbxassetid://116758496"},
                        {"rbxassetid://1233158420","rbxassetid://1233158838","rbxassetid://1233157105","rbxassetid://1233157640","rbxassetid://1233157995","rbxassetid://1233159158"},
                        {"rbxassetid://1327358","rbxassetid://1327359","rbxassetid://1327355","rbxassetid://1327357","rbxassetid://1327356","rbxassetid://1327360"},
                        {"rbxassetid://570555736","rbxassetid://570555964","rbxassetid://570555800","rbxassetid://570555840","rbxassetid://570555882","rbxassetid://570555929"},
                        {"rbxassetid://95020137072033","rbxassetid://92862258103959","rbxassetid://107665368823185","rbxassetid://126542804346203","rbxassetid://103716549795832","rbxassetid://131036626982613"},
                        {"rbxassetid://169210090","rbxassetid://169210108","rbxassetid://169210121","rbxassetid://169210133","rbxassetid://169210143","rbxassetid://169210149"},
                        {"rbxassetid://47974894","rbxassetid://47974690","rbxassetid://47974821","rbxassetid://47974776","rbxassetid://47974859","rbxassetid://47974909"}
                    };

                    int idx = global::world::Skybox_Type;
                    if (idx >= 0 && idx < 11) {
                        drive->writestring(sky.Address + offset::sky::SkyboxBk, presets[idx].bk);
                        drive->writestring(sky.Address + offset::sky::SkyboxDn, presets[idx].dn);
                        drive->writestring(sky.Address + offset::sky::SkyboxFt, presets[idx].ft);
                        drive->writestring(sky.Address + offset::sky::SkyboxLf, presets[idx].lf);
                        drive->writestring(sky.Address + offset::sky::SkyboxRt, presets[idx].rt);
                        drive->writestring(sky.Address + offset::sky::SkyboxUp, presets[idx].up);
                    }
                }

                // Rotation: write every 200ms (only if sky address is valid)
                if (global::world::Rotate && sky.Address && sky.Address != 0xFFFFFFFFFFFFFFFF) {
                    try {
                        static float rotY = 0.0f;
                        rotY = (rotY > 360.0f) ? 0.0f : rotY + global::world::Skybox_Rotate_Speed;
                        drive->write<sdk::vector3>(sky.Address + offset::sky::SkyboxOrientation, { 0.0f, rotY, 0.0f });
                    } catch (...) {}
                }
            }

            // Always invalidate view cache after sky changes
            if (needsUpdate && global::view.Address) {
                try {
                    sdk::view::invalidate();
                    sdk::view::skybox();
                } catch (...) {}
            }

            } catch (...) {}
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    void atmosphere() {

        while (true)
        {
            try {
            if (global::world::Ambience)
            {
                if (!global::light.Address) { std::this_thread::sleep_for(std::chrono::milliseconds(200)); continue; }
                if (!s_ambienceSaved) {
                    s_origAmbient = drive->read<sdk::vector3>(global::light.Address + offset::light::Ambient);
                    s_origOutdoorAmbient = drive->read<sdk::vector3>(global::light.Address + offset::light::OutdoorAmbient);
                    s_ambienceSaved = true;
                }
                sdk::light::ambient(global::light.Address, {global::world::color::Ambience[0], global::world::color::Ambience[1], global::world::color::Ambience[2]} );
                sdk::view::invalidate();
            }
            else if (s_ambienceSaved) {
                sdk::light::ambient(global::light.Address, s_origAmbient);
                sdk::view::invalidate();
                s_ambienceSaved = false;
            }
            } catch (...) {}
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    void fog() {

        while (true)
        {
            try {
            if (global::world::Fog)
            {
                if (!global::light.Address) { std::this_thread::sleep_for(std::chrono::milliseconds(200)); continue; }
                if (!s_fogSaved) {
                    s_origFogEnd = drive->read<float>(global::light.Address + offset::light::FogEnd);
                    s_origFogColor = drive->read<sdk::vector3>(global::light.Address + offset::light::FogColor);
                    s_fogSaved = true;
                }
                sdk::light::fog(global::light.Address, global::world::Fog_Distance, {global::world::color::Fog[0], global::world::color::Fog[1], global::world::color::Fog[2]} );
                sdk::view::invalidate();
            }
            else if (s_fogSaved) {
                sdk::light::fog(global::light.Address, s_origFogEnd, s_origFogColor);
                sdk::view::invalidate();
                s_fogSaved = false;
            }
            } catch (...) {}
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    void brightness() {

        while (true)
        {
            try {
            if (global::world::Brightness)
            {
                if (!global::light.Address) { std::this_thread::sleep_for(std::chrono::milliseconds(200)); continue; }
                if (!s_brightnessSaved) {
                    s_origBrightness = drive->read<float>(global::light.Address + offset::light::Brightness);
                    s_brightnessSaved = true;
                }
                sdk::light::brightness(global::light.Address, global::world::BrightnessI);
                sdk::view::invalidate();
            }
            else if (s_brightnessSaved) {
                sdk::light::brightness(global::light.Address, s_origBrightness);
                sdk::view::invalidate();
                s_brightnessSaved = false;
            }
            } catch (...) {}
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    void exposure() {

        while (true)
        {
            try {
            if (global::world::Exposure)
            {
                if (!global::light.Address) { std::this_thread::sleep_for(std::chrono::milliseconds(200)); continue; }
                if (!s_exposureSaved) {
                    s_origExposure = drive->read<float>(global::light.Address + offset::light::ExposureCompensation);
                    s_exposureSaved = true;
                }
                sdk::light::exposure(global::light.Address, global::world::ExposureI);
                sdk::view::invalidate();
            }
            else if (s_exposureSaved) {
                sdk::light::exposure(global::light.Address, s_origExposure);
                sdk::view::invalidate();
                s_exposureSaved = false;
            }
            } catch (...) {}
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    void fov() {

        float lastFovVal = -1.f;

        while (true)
        {
            try {
            if (global::world::FOV && global::camera.Address)
            {
                if (std::abs(global::world::FOV_Distance - lastFovVal) > 0.1f)
                {
                    sdk::light::fov(global::camera.Address, global::world::FOV_Distance);
                    lastFovVal = global::world::FOV_Distance;
                }
                // Write every frame to prevent camera from overriding it
                // But use 16ms interval to avoid saturating memory bus
                sdk::light::fov(global::camera.Address, global::world::FOV_Distance);
            }
            } catch (...) {}
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // every frame at 60fps
        }

    }

    void run() {

        std::thread(skybox).detach();
        std::thread(atmosphere).detach();
        std::thread(fog).detach();
        std::thread(brightness).detach();
        std::thread(exposure).detach();
		std::thread(fov).detach();
    }
}
