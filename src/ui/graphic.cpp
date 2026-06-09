#include <dwmapi.h>
#include <cstdio>
#include <chrono>
#include <thread>
#include <d3d11.h>
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
// graphic — DirectX11 + Win32 overlay manager
// Now delegates ALL rendering to ModernUI
// ========================================================================

graphic::graphic() : m_ui(std::make_unique<ModernUI>()) { Detail = std::make_unique<detail>(); }
graphic::~graphic() { dropimgui(); dropwindow(); dropdevice(); }

bool graphic::window() {
    Detail->WindowClass.cbSize = sizeof(Detail->WindowClass);
    Detail->WindowClass.style = CS_CLASSDC;
    Detail->WindowClass.lpszClassName = "autopsy.lol";
    Detail->WindowClass.hInstance = GetModuleHandleA(0);
    Detail->WindowClass.lpfnWndProc = wndproc;
    RegisterClassExA(&Detail->WindowClass);

    Detail->Window = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        Detail->WindowClass.lpszClassName, "autopsy.lol", WS_POPUP,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        0, 0, Detail->WindowClass.hInstance, 0);

    if (!Detail->Window) return false;
    SetLayeredWindowAttributes(Detail->Window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

    RECT ClientArea{}, WindowArea{};
    GetClientRect(Detail->Window, &ClientArea);
    GetWindowRect(Detail->Window, &WindowArea);
    POINT Diff{};
    ClientToScreen(Detail->Window, &Diff);

    MARGINS Margins{
        WindowArea.left + (Diff.x - WindowArea.left),
        WindowArea.top + (Diff.y - WindowArea.top),
        WindowArea.right, WindowArea.bottom
    };
    DwmExtendFrameIntoClientArea(Detail->Window, &Margins);
    ShowWindow(Detail->Window, SW_SHOW);
    UpdateWindow(Detail->Window);
    return true;
}

bool graphic::device() {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.RefreshRate.Numerator = 0;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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

    if (hr == DXGI_ERROR_UNSUPPORTED)
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

void graphic::end() {
    if (!Detail->DeviceContext || !Detail->GraphicsTargetView || !Detail->SwapChain)
        return;
    ImGui::Render();
    const float cc[4]{ 0.f, 0.f, 0.f, 0.f };
    Detail->DeviceContext->OMSetRenderTargets(1, &Detail->GraphicsTargetView, nullptr);
    Detail->DeviceContext->ClearRenderTargetView(Detail->GraphicsTargetView, cc);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    const int mode = ImClamp(global::setting::Performance_Mode, 0, 2);
    if (mode == 0) std::this_thread::sleep_for(std::chrono::milliseconds(2));

    const UINT syncInterval = mode == 2 ? 0u : 1u;
    const HRESULT hr = Detail->SwapChain->Present(syncInterval, 0);
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        ExitProcess(0);
}
