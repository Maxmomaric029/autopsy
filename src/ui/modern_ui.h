#pragma once
#include <d3d11.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/backends/imgui_impl_dx11.h>

// ========================================================================
// ModernUI — Clean encapsulation of the entire UI layer
// ========================================================================
// Responsibilities:
// - Initializes and owns ImGui context + font atlas
// - Provides RenderMenu() and RenderESP() as the ONLY rendering entry points
// - Handles menu toggle, backdrop, HUD panels, welcome splash
// - Owns the animation system clock
// ========================================================================

class ModernUI {
public:
    ModernUI();
    ~ModernUI();

    // ---- Lifecycle ------------------------------------------------------
    bool Create(HWND window, ID3D11Device* device, ID3D11DeviceContext* context);
    void Destroy();

    // ---- Frame (call every frame in order) ------------------------------
    void BeginFrame(HWND window);               // MSG pump, ImGui::NewFrame()
    void RenderMenu();                          // The config menu (pill tabs)
    void RenderESP();                           // Overlay HUD (watermark, radar, ESP, etc.)
    void EndFrame(IDXGISwapChain* swapChain);   // ImGui::Render(), Present()

    // ---- State ----------------------------------------------------------
    bool IsOpen() const { return m_open; }
    void Toggle() { m_open = !m_open; }

    // ---- Logo textures --------------------------------------------------
    ID3D11ShaderResourceView* GetCuervo() const { return m_cuervo; }
    ID3D11ShaderResourceView* GetDescarga() const { return m_descarga; }
    int GetCuervoW() const { return m_cuervoW; }
    int GetCuervoH() const { return m_cuervoH; }

private:
    bool m_open = false;
    bool m_initialized = false;

    bool load_logos(ID3D11Device* device);
    void free_logos();

    // No copy
    ModernUI(const ModernUI&) = delete;
    ModernUI& operator=(const ModernUI&) = delete;
};

// ========================================================================
// Exported logo SRVs for sidebar/hud access (defined in modern_ui.cpp)
// ========================================================================
extern ID3D11ShaderResourceView* g_sidebar_logo;
extern int g_sidebar_logoW;
extern int g_sidebar_logoH;
