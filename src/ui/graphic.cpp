#include <cstdio>
#include <chrono>
#include <thread>
#include <d3d11.h>
#include <d3d11_2.h>
#include <dwmapi.h>
#include <vector>
#include <string>
#include <deque>
#include <mutex>
#include <atomic>
#include <fstream>

#include "ui/graphic.h"
#include "ui/modern_ui.h"
#include "ui/core/sound.h"
#include "global.h"
#include "config.h"

#include <imgui/imgui_internal.h>
#include <imgui/misc/imgui_freetype.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// ========================================================================
// Kenney UI sound effects (CC0 license, UI_SFX_Set from opengameart.org)
// ========================================================================
#include "ui/embedded/sfx_all.h"

// ========================================================================
// Miniaudio implementation — play UI sounds from embedded Kenney WAV data
// ========================================================================
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

// ========================================================================
// Sound system implementation (declared in ui/core/sound.h)
// ========================================================================
namespace sound {

    // ---- Engine state ----
    static ma_engine g_engine = {};
    static bool g_engine_initialized = false;

    // ---- Thread-safe sound queue ----
    static std::mutex g_queue_mutex;
    static std::deque<std::string> g_sound_queue;

    // ---- Raw WAV data reference — points to embedded Kenney sound arrays ----
    struct SoundRef {
        const unsigned char* data;
        unsigned int size;
    };
    static std::unordered_map<std::string, SoundRef> g_sounds;

    // ---- Active (playing) sounds, cleaned up when done ----
    struct ActiveSound {
        ma_decoder decoder;
        ma_sound sound;
    };
    static std::vector<ActiveSound*> g_active;

    // ---- Hover cooldown (50ms) ----
    static double g_last_hover_time = 0.0;
    static constexpr double kHoverCooldown = 0.05;

    // ====================================================================
    // Sound mapping — Kenney WAV filename → sound::id
    // ====================================================================
    // toggle_on  → switch1.wav   (crisp mechanical switch)
    // toggle_off → switch12.wav  (softer, shorter switch)
    // hover      → rollover5.wav (subtle hover feedback)
    // tab_switch → switch26.wav  (short clean transition)
    // bind_start → click2.wav    (quick tick)
    // bind_set   → click4.wav    (confirmation beep)
    // menu_open  → switch33.wav  (longer rising sweep)
    // menu_close → switch38.wav  (descending close)

    // ====================================================================
    // Public API
    // ====================================================================

    bool init() {
        if (g_engine_initialized) return true;

        ma_result result = ma_engine_init(NULL, &g_engine);
        if (result != MA_SUCCESS) {
            OutputDebugStringA("[sound] ma_engine_init failed\n");
            return false;
        }
        g_engine_initialized = true;
        g_active.reserve(32);

        // Register embedded Kenney WAV data
        g_sounds[id::toggle_on]  = { sfx_toggle_on,  sfx_toggle_on_size };
        g_sounds[id::toggle_off] = { sfx_toggle_off, sfx_toggle_off_size };
        g_sounds[id::hover]      = { sfx_hover,      sfx_hover_size };
        g_sounds[id::tab_switch] = { sfx_tab_switch, sfx_tab_switch_size };
        g_sounds[id::bind_start] = { sfx_bind_start, sfx_bind_start_size };
        g_sounds[id::bind_set]   = { sfx_bind_set,   sfx_bind_set_size };
        g_sounds[id::menu_open]  = { sfx_menu_open,  sfx_menu_open_size };
        g_sounds[id::menu_close] = { sfx_menu_close, sfx_menu_close_size };

        OutputDebugStringA("[sound] initialized\n");
        return true;
    }

    void shutdown() {
        if (!g_engine_initialized) return;
        // Clean up any active sounds
        for (auto* a : g_active) {
            ma_sound_uninit(&a->sound);
            ma_decoder_uninit(&a->decoder);
            delete a;
        }
        g_active.clear();
        g_sounds.clear();
        ma_engine_uninit(&g_engine);
        g_engine_initialized = false;
        OutputDebugStringA("[sound] shutdown\n");
    }

    void play(const char* sound_id) {
        if (!g_engine_initialized || !sound_id || !global::setting::Sound_Enabled) return;
        std::lock_guard<std::mutex> lock(g_queue_mutex);
        g_sound_queue.push_back(sound_id);
    }

    void update() {
        if (!g_engine_initialized) return;

        // ---- Clean up finished sounds ----
        for (int i = (int)g_active.size() - 1; i >= 0; i--) {
            if (!ma_sound_is_playing(&g_active[i]->sound)) {
                ma_sound_uninit(&g_active[i]->sound);
                ma_decoder_uninit(&g_active[i]->decoder);
                delete g_active[i];
                g_active[i] = g_active.back();
                g_active.pop_back();
            }
        }

        // ---- Process new play requests ----
        std::lock_guard<std::mutex> lock(g_queue_mutex);
        while (!g_sound_queue.empty()) {
            std::string id = g_sound_queue.front();
            g_sound_queue.pop_front();

            auto it = g_sounds.find(id);
            if (it == g_sounds.end()) continue;

            auto& snd = it->second;

            // Create decoder from embedded Kenney WAV data
            ma_decoder_config dec_cfg = ma_decoder_config_init(ma_format_s16, 1, 44100);
            auto* as = new ActiveSound();
            ma_result r = ma_decoder_init_memory(snd.data, snd.size, &dec_cfg, &as->decoder);
            if (r != MA_SUCCESS) { delete as; continue; }

            // Create sound from decoder (decoder must outlive sound — it does, stored in ActiveSound)
            r = ma_sound_init_from_data_source(&g_engine, &as->decoder,
                MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, &as->sound);
            if (r != MA_SUCCESS) {
                ma_decoder_uninit(&as->decoder);
                delete as;
                continue;
            }

            ma_sound_set_volume(&as->sound, global::setting::Sound_Volume);
            ma_sound_start(&as->sound);
            g_active.push_back(as);
        }
    }

    bool can_play_hover() {
        double now = ImGui::GetTime();
        if (now - g_last_hover_time >= kHoverCooldown) {
            g_last_hover_time = now;
            return true;
        }
        return false;
    }

} // namespace sound

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam);

LRESULT CALLBACK wndproc(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam) {
    if (ImGui_ImplWin32_WndProcHandler(Hwnd, Msg, WParam, LParam))
        return true;
    switch (Msg) {
    case WM_SYSCOMMAND:
        if ((WParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_SYSKEYDOWN:
        if (WParam == VK_F4) { DestroyWindow(Hwnd); return 0; }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_CLOSE:
        DestroyWindow(Hwnd);
        return 0;
    }
    return DefWindowProcA(Hwnd, Msg, WParam, LParam);
}

// ========================================================================
// FPS limiter — 3-mode with adaptive idle (per spec)
// mode 0 = Eco:       60fps cap (idle: 30fps)
// mode 1 = Balanced:  144fps cap (idle: 30fps)
// mode 2 = Unlocked:  240fps safety cap (idle: 30fps)
// ========================================================================
static struct FpsLimiter {
    LARGE_INTEGER freq = {};
    bool ready = false;

    void init() {
        timeBeginPeriod(1);
        ready = QueryPerformanceFrequency(&freq) && freq.QuadPart > 0;
    }
    void shutdown() {
        timeEndPeriod(1);
        ready = false;
    }

    // Idle = menu closed AND no ESP content in the frame
    void wait(int targetFps, bool idle = false) {
        if (!ready || targetFps <= 0) return;
        static LARGE_INTEGER last = {};
        if (!last.QuadPart) {
            QueryPerformanceCounter(&last);
            return;
        }
        int effectiveFps = idle ? ImMin(targetFps, 30) : targetFps;
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        double elapsedSec = double(now.QuadPart - last.QuadPart) / double(freq.QuadPart);
        double targetSec = 1.0 / double(effectiveFps);
        double remainingSec = targetSec - elapsedSec;
        if (remainingSec > 0.001) {
            DWORD sleepMs = (DWORD)((remainingSec - 0.001) * 1000.0);
            if (sleepMs > 0) Sleep(sleepMs);
            // Spin-wait the remaining ~1ms for accuracy
            while (true) {
                QueryPerformanceCounter(&now);
                double elapsed = double(now.QuadPart - last.QuadPart) / double(freq.QuadPart);
                if (elapsed >= targetSec) break;
            }
        }
        QueryPerformanceCounter(&last);
    }
} g_fps;

// Frame content flag
bool g_frameHadContent = false;

// ========================================================================
// graphic — DirectX11 + Win32 overlay manager
// ========================================================================

graphic::graphic() : m_ui(std::make_unique<ModernUI>()) { Detail = std::make_unique<detail>(); }
graphic::~graphic() { g_fps.shutdown(); dropimgui(); dropwindow(); dropdevice(); }

bool graphic::window() {
    Detail->WindowClass.cbSize = sizeof(Detail->WindowClass);
    Detail->WindowClass.style = CS_CLASSDC;
    Detail->WindowClass.lpszClassName = "miserable.lol";
    Detail->WindowClass.hInstance = GetModuleHandleA(0);
    Detail->WindowClass.lpfnWndProc = wndproc;
    RegisterClassExA(&Detail->WindowClass);

    Detail->Window = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        Detail->WindowClass.lpszClassName, "miserable.lol", WS_POPUP,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        0, 0, Detail->WindowClass.hInstance, 0);

    if (!Detail->Window) return false;

    SetLayeredWindowAttributes(Detail->Window, RGB(0, 0, 0), 255, LWA_ALPHA);

    MARGINS margins = { -1, -1, -1, -1 };
    HRESULT dwmResult = DwmExtendFrameIntoClientArea(Detail->Window, &margins);
    if (FAILED(dwmResult)) {
        OutputDebugStringA("[graphic] DwmExtendFrameIntoClientArea failed\n");
    }

    ShowWindow(Detail->Window, SW_SHOW);
    UpdateWindow(Detail->Window);
    return true;
}

// ========================================================================
// Device / Swapchain — DISCARD for layered window alpha compatibility
// ========================================================================
bool graphic::device() {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 1;
    sd.BufferDesc.RefreshRate.Numerator = 0;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.OutputWindow = Detail->Window;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Windowed = 1;
    sd.Flags = 0;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    D3D_FEATURE_LEVEL fl;
    const D3D_FEATURE_LEVEL fll[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        fll, 2, D3D11_SDK_VERSION, &sd, &Detail->SwapChain,
        &Detail->Device, &fl, &Detail->DeviceContext);

    if (FAILED(hr))
        hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
            fll, 2, D3D11_SDK_VERSION, &sd, &Detail->SwapChain,
            &Detail->Device, &fl, &Detail->DeviceContext);

    if (FAILED(hr) || !Detail->SwapChain || !Detail->Device || !Detail->DeviceContext) {
        MessageBoxA(nullptr, "This software can not run on your computer.",
            "Critical Problem", MB_ICONERROR | MB_OK);
        return false;
    }

    ID3D11Texture2D* bb = nullptr;
    if (FAILED(Detail->SwapChain->GetBuffer(0, IID_PPV_ARGS(&bb))) || !bb)
        return false;

    if (bb) {
        HRESULT rtv = Detail->Device->CreateRenderTargetView(bb, nullptr,
            &Detail->GraphicsTargetView);
        bb->Release();
        return SUCCEEDED(rtv) && Detail->GraphicsTargetView;
    }
    return false;
}

bool graphic::imgui() {
    g_fps.init();
    return m_ui->Create(Detail->Window, Detail->Device, Detail->DeviceContext);
}

void graphic::dropdevice() {
    if (Detail->GraphicsTargetView) Detail->GraphicsTargetView->Release();
    if (Detail->SwapChain) Detail->SwapChain->Release();
    if (Detail->DeviceContext) Detail->DeviceContext->Release();
    if (Detail->Device) Detail->Device->Release();
    Detail->GraphicsTargetView = nullptr;
    Detail->SwapChain = nullptr;
    Detail->DeviceContext = nullptr;
    Detail->Device = nullptr;
}

void graphic::dropwindow() {
    if (Detail->Window) {
        DestroyWindow(Detail->Window);
        Detail->Window = nullptr;
    }
    if (Detail->WindowClass.lpszClassName)
        UnregisterClassA(Detail->WindowClass.lpszClassName,
            Detail->WindowClass.hInstance);
}

void graphic::dropimgui() {
    m_ui->Destroy();
}

void graphic::begin() {
    m_ui->BeginFrame(Detail->Window);
    Running = m_ui->IsOpen();
}

void graphic::visual() {
    m_ui->RenderESP();
}

void graphic::menu() {
    m_ui->RenderMenu();
}

// ========================================================================
// end() — FPS-limited Present with adaptive idle (3 modes)
// ========================================================================
void graphic::end() {
    if (!Detail->DeviceContext || !Detail->GraphicsTargetView || !Detail->SwapChain)
        return;

    ImGui::Render();
    const float cc[4]{ 0.f, 0.f, 0.f, 0.f };
    Detail->DeviceContext->OMSetRenderTargets(1, &Detail->GraphicsTargetView, nullptr);
    Detail->DeviceContext->ClearRenderTargetView(Detail->GraphicsTargetView, cc);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // ---- 3-mode FPS limiter ----
    const int mode = ImClamp(global::setting::Performance_Mode, 0, 2);
    bool menuOpen = m_ui->IsOpen();
    bool idle = !menuOpen && !g_frameHadContent;

    if (mode == 0) {
        // Eco: 60fps cap, idle 30fps
        g_fps.wait(idle ? 30 : 60);
    } else if (mode == 1) {
        // Balanced: 144fps cap, idle 30fps
        g_fps.wait(idle ? 30 : 144);
    } else {
        // Unlocked: 240fps safety cap, idle 30fps
        g_fps.wait(idle ? 30 : 240);
    }

    g_frameHadContent = false;

    const HRESULT hr = Detail->SwapChain->Present(0, 0);
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        ExitProcess(0);
}
