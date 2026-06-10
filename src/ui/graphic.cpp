#include <cstdio>
#include <chrono>
#include <thread>
#include <d3d11.h>
#include <d3d11_2.h>
#include <vector>
#include <string>

#include "ui/graphic.h"
#include "ui/modern_ui.h"
#include "global.h"
#include "config.h"

#include <imgui/imgui_internal.h>
#include <imgui/misc/imgui_freetype.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

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
        return 0;
    }
    return DefWindowProcA(Hwnd, Msg, WParam, LParam);
}

// ========================================================================
// FPS limiter — high-resolution timer with Sleep+spin (F1.1)
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

    // Sleep until the next frame boundary at 'targetFps', then return.
    void wait(int targetFps) {
        if (!ready || targetFps <= 0) return;
        static LARGE_INTEGER last = {};
        if (!last.QuadPart) {
            QueryPerformanceCounter(&last);
            return;
        }
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        double elapsedSec = double(now.QuadPart - last.QuadPart) / double(freq.QuadPart);
        double targetSec = 1.0 / double(targetFps);
        double remainingSec = targetSec - elapsedSec;
        if (remainingSec > 0.001) {
            // Sleep most of the wait, spin the last ~1ms for accuracy
            DWORD sleepMs = (DWORD)((remainingSec - 0.001) * 1000.0);
            if (sleepMs > 0) Sleep(sleepMs);
            // Spin-wait the remaining time
            while (true) {
                QueryPerformanceCounter(&now);
                double elapsed = double(now.QuadPart - last.QuadPart) / double(freq.QuadPart);
                if (elapsed >= targetSec) break;
            }
        }
        QueryPerformanceCounter(&last);
    }
} g_fps;

// Frame content flag — set by RenderESP/RenderMenu when they emit draw calls
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
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        Detail->WindowClass.lpszClassName, "miserable.lol", WS_POPUP,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        0, 0, Detail->WindowClass.hInstance, 0);

    if (!Detail->Window) return false;

    // LWA_COLORKEY makes RGB(0,0,0) pixels transparent via chroma key.
    // The D3D clear color is {0,0,0,0} so the background is fully transparent.
    // Do NOT combine with DwmExtendFrameIntoClientArea — that overwrites
    // the layered window attributes and kills transparency.
    SetLayeredWindowAttributes(Detail->Window, RGB(0, 0, 0), BYTE(0), LWA_COLORKEY);

    ShowWindow(Detail->Window, SW_SHOW);
    UpdateWindow(Detail->Window);
    return true;
}

// ========================================================================
// Device / Swapchain — try FLIP_DISCARD with waitable object, fallback DISCARD (F1.2)
// ========================================================================
bool graphic::device() {
    // Try FLIP_DISCARD first (modern, lower latency)
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.RefreshRate.Numerator = 0;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.OutputWindow = Detail->Window;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Windowed = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    D3D_FEATURE_LEVEL fl;
    const D3D_FEATURE_LEVEL fll[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        fll, 2, D3D11_SDK_VERSION, &sd, &Detail->SwapChain,
        &Detail->Device, &fl, &Detail->DeviceContext);

    // Fallback: DISCARD without waitable (layered windows may not support FLIP)
    if (FAILED(hr)) {
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags = 0;
        hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            fll, 2, D3D11_SDK_VERSION, &sd, &Detail->SwapChain,
            &Detail->Device, &fl, &Detail->DeviceContext);
    }

    if (hr == DXGI_ERROR_UNSUPPORTED)
        hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
            fll, 2, D3D11_SDK_VERSION, &sd, &Detail->SwapChain,
            &Detail->Device, &fl, &Detail->DeviceContext);

    if (FAILED(hr) || !Detail->SwapChain || !Detail->Device || !Detail->DeviceContext) {
        MessageBoxA(nullptr, "This software can not run on your computer.",
            "Critical Problem", MB_ICONERROR | MB_OK);
        return false;
    }

    // Waitable object removed — FPS limiter is the sole pacing mechanism

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
// end() — FPS-limited Present with adaptive idle (F1.1)
// ========================================================================
void graphic::end() {
    if (!Detail->DeviceContext || !Detail->GraphicsTargetView || !Detail->SwapChain)
        return;

    ImGui::Render();
    const float cc[4]{ 0.f, 0.f, 0.f, 0.f };
    Detail->DeviceContext->OMSetRenderTargets(1, &Detail->GraphicsTargetView, nullptr);
    Detail->DeviceContext->ClearRenderTargetView(Detail->GraphicsTargetView, cc);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // ---- FPS-only pacing — no waitable object, no VSync (F1.1 fix) ----
    // The FPS limiter is the SOLE pacing mechanism. Present(0,0) avoids triple-wait lag.
    const int mode = ImClamp(global::setting::Performance_Mode, 0, 2);
    bool menuOpen = m_ui->IsOpen();

    if (mode == 0) {
        // Eco: 60fps cap
        if (!menuOpen && !g_frameHadContent)
            g_fps.wait(30);  // idle 30fps
        else
            g_fps.wait(60);
    } else if (mode == 1) {
        // Balanced: 144fps cap
        if (!menuOpen && !g_frameHadContent)
            g_fps.wait(30);
        else
            g_fps.wait(144);
    } else {
        // Uncapped: safety cap at 240fps
        if (!menuOpen && !g_frameHadContent)
            g_fps.wait(30);
        else
            g_fps.wait(240);
    }

    g_frameHadContent = false; // reset for next frame

    // Always Present with 0 sync interval — FPS limiter handles pacing, not VSync
    const HRESULT hr = Detail->SwapChain->Present(0, 0);
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        ExitProcess(0);
}
