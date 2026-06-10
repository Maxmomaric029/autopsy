#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <memory>
#include <d3d11.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/backends/imgui_impl_dx11.h>
#include <string>
#include <vector>

class ModernUI;

// ========================================================================
// graphic — DirectX11 + Win32 overlay manager
// ========================================================================
// Handles:
//   - Overlay window creation / destruction
//   - D3D11 device / swap chain creation / destruction
//   - ImGui init (delegated to ModernUI)
//   - Frame begin (delegated to ModernUI::BeginFrame)
//   - Render + Present
// ========================================================================

struct detail {
    HWND Window = nullptr;
    WNDCLASSEX WindowClass = {};
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* DeviceContext = nullptr;
    ID3D11RenderTargetView* GraphicsTargetView = nullptr;
    IDXGISwapChain* SwapChain = nullptr;
};

class graphic {
public:
    graphic();
    ~graphic();

    bool Running = false;
    void begin();
    void menu();
    void visual();
    void end();
    bool device();
    bool window();
    bool imgui();

    std::unique_ptr<detail> Detail = std::make_unique<detail>();

private:
    void dropdevice();
    void dropwindow();
    void dropimgui();

    std::unique_ptr<ModernUI> m_ui;
};

inline std::unique_ptr<graphic> screen = std::make_unique<graphic>();
