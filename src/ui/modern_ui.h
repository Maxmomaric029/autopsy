#pragma once
#include <d3d11.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/backends/imgui_impl_dx11.h>

// ========================================================================
// ModernUI — Clean encapsulation of the entire UI layer
// ========================================================================

class ModernUI {
public:
    ModernUI();
    ~ModernUI();

    // ---- Lifecycle ------------------------------------------------------
    bool Create(HWND window, ID3D11Device* device, ID3D11DeviceContext* context);
    void Destroy();

    // ---- Frame (call every frame in order) ------------------------------
    void BeginFrame(HWND window);
    void RenderMenu();
    void RenderESP();
    void EndFrame(IDXGISwapChain* swapChain);

    // ---- State ----------------------------------------------------------
    bool IsOpen() const { return m_open; }
    void Toggle() { m_open = !m_open; }

private:
    bool m_open = false;
    bool m_initialized = false;

    bool load_logos(ID3D11Device* device);
    void free_logos();

    ModernUI(const ModernUI&) = delete;
    ModernUI& operator=(const ModernUI&) = delete;
};

// ========================================================================
// Exported logo SRVs for sidebar/hud access
// ========================================================================
extern ID3D11ShaderResourceView* g_sidebar_logo;
extern int g_sidebar_logoW;
extern int g_sidebar_logoH;
