#pragma once
#include <string>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <deque>
#include <imgui/imgui.h>

// ========================================================================
// Sound system — miniaudio wrapper for UI sound effects
// ========================================================================
// Uses miniaudio (header-only) for WAV playback.
// All sounds are short UI feedback clips.
// Implementation with MINIAUDIO_IMPLEMENTATION is in graphic.cpp.
// ========================================================================

namespace sound {

    // ========================================================================
    // Sound ID list
    // ========================================================================
    namespace id {
        inline constexpr const char* toggle_on    = "toggle_on";
        inline constexpr const char* toggle_off   = "toggle_off";
        inline constexpr const char* hover        = "hover";
        inline constexpr const char* tab_switch   = "tab_switch";
        inline constexpr const char* bind_start   = "bind_start";
        inline constexpr const char* bind_set     = "bind_set";
        inline constexpr const char* menu_open    = "menu_open";
        inline constexpr const char* menu_close   = "menu_close";
    }

    // ========================================================================
    // Public API
    // ========================================================================
    void play(const char* sound_id);
    bool init();
    void shutdown();
    void update();

    // ========================================================================
    // Hover cooldown: minimum 50ms between hover sounds
    // Must be called BEFORE play("hover") on hover-sensitive controls.
    // Returns true if the hover sound should fire.
    // ========================================================================
    bool can_play_hover();

} // namespace sound
