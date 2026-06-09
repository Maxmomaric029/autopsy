#include <dwmapi.h>
#include <cstdio>
#include <chrono>
#include <thread>
#include <d3d11.h>
#include <vector>
#include <string>
#include <cmath>
#include <cfloat>
#include <cstring>

#include "ui/graphic.h"
#include "global.h"
#include "config.h"

#include <imgui/imgui_internal.h>
#include <imgui/misc/imgui_freetype.h>
#include "imgui/imgui_internal.h"

#include "ui/font/regular.h"
#include "ui/font/bold.h"
#include "../features/esp.h"
#include "../features/ball.h"
#include "../features/silent.h"

// New modular UI includes
#include "ui/core/theme.h"
#include "ui/core/animation.h"
#include "ui/core/fonts.h"
#include "ui/core/icons.h"
#include "ui/core/notifications.h"
#include "ui/widgets/controls.h"
#include "ui/layout/menulayout.h"
#include "ui/pages/pages.h"

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

// ---- Constructor / Destructor ------------------------------------------
graphic::graphic() { Detail = std::make_unique<detail>(); }
graphic::~graphic() { dropimgui(); dropwindow(); dropdevice(); }

// ---- Window creation ---------------------------------------------------
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

// ---- D3D11 Device / SwapChain ------------------------------------------
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

// ---- ImGui initialization --------------------------------------------
bool graphic::imgui() {
    ImGui::CreateContext();

    float dpiScale = ImGui_ImplWin32_GetDpiScaleForMonitor(
        ::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    ImGuiIO& IO = ImGui::GetIO();
    IO.IniFilename = nullptr;

    font::load(dpiScale);

    ImGui::StyleColorsDark();
    ImGuiStyle& S = ImGui::GetStyle();
    S.WindowRounding = theme::r_window;
    S.ChildRounding = theme::r_card;
    S.FrameRounding = theme::r_frame;
    S.PopupRounding = 10.f;
    S.ScrollbarRounding = 8.f;
    S.GrabRounding = 6.f;
    S.ItemSpacing = { 8.f, 7.f };
    S.FramePadding = { 8.f, 5.f };
    S.WindowPadding = { 0.f, 0.f };
    S.PopupBorderSize = 1.f;
    S.FrameBorderSize = 0.f;
    S.ScrollbarSize = 4.f;
    S.Colors[ImGuiCol_WindowBg] = theme::c_bg;
    S.Colors[ImGuiCol_ChildBg] = theme::c_card;
    S.Colors[ImGuiCol_Border] = theme::c_border;
    S.Colors[ImGuiCol_Text] = theme::c_text;

    if (!ImGui_ImplWin32_Init(Detail->Window)) return false;
    if (!Detail->Device || !Detail->DeviceContext) return false;
    if (!ImGui_ImplDX11_Init(Detail->Device, Detail->DeviceContext)) return false;
    return true;
}

// ---- Cleanup -----------------------------------------------------------
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
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

// ---- Key to VK mapping ------------------------------------------------
static int menukey(ImGuiKey key) {
    if (key >= ImGuiKey_0 && key <= ImGuiKey_9)   return '0' + (key - ImGuiKey_0);
    if (key >= ImGuiKey_A && key <= ImGuiKey_Z)    return 'A' + (key - ImGuiKey_A);
    if (key >= ImGuiKey_F1 && key <= ImGuiKey_F24) return VK_F1 + (key - ImGuiKey_F1);
    if (key >= ImGuiKey_Keypad0 && key <= ImGuiKey_Keypad9)
        return VK_NUMPAD0 + (key - ImGuiKey_Keypad0);
    switch (key) {
    case ImGuiKey_Tab: return VK_TAB;
    case ImGuiKey_LeftArrow: return VK_LEFT;
    case ImGuiKey_RightArrow: return VK_RIGHT;
    case ImGuiKey_UpArrow: return VK_UP;
    case ImGuiKey_DownArrow: return VK_DOWN;
    case ImGuiKey_PageUp: return VK_PRIOR;
    case ImGuiKey_PageDown: return VK_NEXT;
    case ImGuiKey_Home: return VK_HOME;
    case ImGuiKey_End: return VK_END;
    case ImGuiKey_Insert: return VK_INSERT;
    case ImGuiKey_Delete: return VK_DELETE;
    case ImGuiKey_Backspace: return VK_BACK;
    case ImGuiKey_Space: return VK_SPACE;
    case ImGuiKey_Enter: return VK_RETURN;
    case ImGuiKey_Escape: return VK_ESCAPE;
    case ImGuiKey_LeftCtrl: return VK_LCONTROL;
    case ImGuiKey_LeftShift: return VK_LSHIFT;
    case ImGuiKey_LeftAlt: return VK_LMENU;
    case ImGuiKey_RightCtrl: return VK_RCONTROL;
    case ImGuiKey_RightShift: return VK_RSHIFT;
    case ImGuiKey_RightAlt: return VK_RMENU;
    case ImGuiKey_Menu: return VK_APPS;
    case ImGuiKey_Apostrophe: return VK_OEM_7;
    case ImGuiKey_Comma: return VK_OEM_COMMA;
    case ImGuiKey_Minus: return VK_OEM_MINUS;
    case ImGuiKey_Period: return VK_OEM_PERIOD;
    case ImGuiKey_Slash: return VK_OEM_2;
    case ImGuiKey_Semicolon: return VK_OEM_1;
    case ImGuiKey_Equal: return VK_OEM_PLUS;
    case ImGuiKey_LeftBracket: return VK_OEM_4;
    case ImGuiKey_Backslash: return VK_OEM_5;
    case ImGuiKey_RightBracket: return VK_OEM_6;
    case ImGuiKey_GraveAccent: return VK_OEM_3;
    case ImGuiKey_CapsLock: return VK_CAPITAL;
    case ImGuiKey_ScrollLock: return VK_SCROLL;
    case ImGuiKey_NumLock: return VK_NUMLOCK;
    case ImGuiKey_PrintScreen: return VK_SNAPSHOT;
    case ImGuiKey_Pause: return VK_PAUSE;
    case ImGuiKey_KeypadDecimal: return VK_DECIMAL;
    case ImGuiKey_KeypadDivide: return VK_DIVIDE;
    case ImGuiKey_KeypadMultiply: return VK_MULTIPLY;
    case ImGuiKey_KeypadSubtract: return VK_SUBTRACT;
    case ImGuiKey_KeypadAdd: return VK_ADD;
    case ImGuiKey_KeypadEnter: return VK_RETURN;
    case ImGuiKey_MouseLeft: return VK_LBUTTON;
    case ImGuiKey_MouseRight: return VK_RBUTTON;
    case ImGuiKey_MouseMiddle: return VK_MBUTTON;
    case ImGuiKey_MouseX1: return VK_XBUTTON1;
    case ImGuiKey_MouseX2: return VK_XBUTTON2;
    default: return 0;
    }
}

// ---- Begin frame -------------------------------------------------------
void graphic::begin() {
    MSG Msg;
    while (PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE)) {
        if (Msg.message == WM_QUIT) ExitProcess(0);
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    static bool lastStreamproof = !global::setting::Streamproof;
    if (lastStreamproof != global::setting::Streamproof) {
        SetWindowDisplayAffinity(Detail->Window,
            global::setting::Streamproof ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);
        lastStreamproof = global::setting::Streamproof;
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    {
        HWND _roblox = FindWindowA(0, "Roblox");
        HWND _fg = GetForegroundWindow();
        static double lastToggle = -1.0;
        const double now = ImGui::GetTime();
        const int menuVk = menukey(global::setting::Menu_Key);
        if (menuVk != 0 && (GetAsyncKeyState(menuVk) & 1) &&
            (_fg == _roblox || _fg == Detail->Window) && now - lastToggle >= .18) {
            lastToggle = now;
            Running = !Running;
            LONG exStyle = WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED;
            if (!Running) exStyle |= WS_EX_TRANSPARENT;
            SetWindowLong(Detail->Window, GWL_EXSTYLE, exStyle);
        }
    }
}

// ---- Silent aim cursor -------------------------------------------------
static void cursor() {
    if (!SilentAimInstance.Address) return;
    if (!drive->read<bool>(SilentAimInstance.Address + offset::gui::Visible)) return;
    POINT pt;
    if (!GetCursorPos(&pt)) return;
    const bool rmbHeld = GetAsyncKeyState(VK_RBUTTON) & 0x8000;
    const float gap = rmbHeld ? 4.f : 10.f;
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    const ImU32 col = IM_COL32(255, 255, 255, 255);
    const float dot = 4.f, lw = 2.f, ll = 10.f;
    const ImVec2 c = { (float)pt.x, (float)pt.y };
    dl->AddRectFilled(c - ImVec2(dot * .5f, dot * .5f), c + ImVec2(dot * .5f, dot * .5f), col);
    dl->AddRectFilled({ c.x - lw * .5f, c.y - gap - ll }, { c.x + lw * .5f, c.y - gap }, col);
    dl->AddRectFilled({ c.x - lw * .5f, c.y + gap }, { c.x + lw * .5f, c.y + gap + ll }, col);
    dl->AddRectFilled({ c.x - gap - ll, c.y - lw * .5f }, { c.x - gap,    c.y + lw * .5f }, col);
    dl->AddRectFilled({ c.x + gap,    c.y - lw * .5f }, { c.x + gap + ll, c.y + lw * .5f }, col);
}

// ---- Roblox focus helpers ----------------------------------------------
static bool focusroblox() {
    HWND roblox = FindWindowA(nullptr, "Roblox");
    if (!roblox || !IsWindow(roblox)) return false;
    if (GetForegroundWindow() == roblox) return true;
    if (IsIconic(roblox)) ShowWindow(roblox, SW_RESTORE);
    const DWORD currentThread = GetCurrentThreadId();
    const DWORD targetThread = GetWindowThreadProcessId(roblox, nullptr);
    HWND foreground = GetForegroundWindow();
    const DWORD foregroundThread = foreground ? GetWindowThreadProcessId(foreground, nullptr) : 0;
    const bool attachForeground = foregroundThread != 0 && foregroundThread != currentThread;
    const bool attachTarget = targetThread != 0 && targetThread != currentThread && targetThread != foregroundThread;
    if (attachForeground) AttachThreadInput(currentThread, foregroundThread, TRUE);
    if (attachTarget) AttachThreadInput(currentThread, targetThread, TRUE);
    BringWindowToTop(roblox);
    const bool focused = SetForegroundWindow(roblox) != FALSE;
    if (attachTarget) AttachThreadInput(currentThread, targetThread, FALSE);
    if (attachForeground) AttachThreadInput(currentThread, foregroundThread, FALSE);
    return focused || GetForegroundWindow() == roblox;
}

static void keepfocus(float elapsed) {
    static double lastFocusAttempt = -1.0;
    const double now = ImGui::GetTime();
    if (elapsed > 4.0f || (lastFocusAttempt >= 0.0 && now - lastFocusAttempt < .35)) return;
    lastFocusAttempt = now;
    focusroblox();
}

// ---- Welcome splash ----------------------------------------------------
static void welcome(bool menuOpen) {
    static double startTime = -1.0;
    if (startTime < 0.0) startTime = ImGui::GetTime();
    const float elapsed = (float)(ImGui::GetTime() - startTime);
    if (elapsed > 4.85f) return;
    if (!menuOpen) keepfocus(elapsed);

    const float inT = anim::ease_out_cubic(ImClamp(elapsed / .55f, 0.f, 1.f));
    const float outT = elapsed <= 4.0f ? 1.f
        : 1.f - anim::ease_out_cubic(ImClamp((elapsed - 4.0f) / .85f, 0.f, 1.f));
    const float alpha = inT * outT;
    if (alpha <= .01f) return;

    ImDrawList* dl = ImGui::GetForegroundDrawList();
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const ImVec2 center = display * .5f;
    const float pulse = (sinf(elapsed * 4.2f) + 1.f) * .5f;

    dl->AddRectFilled({ 0.f, 0.f }, display, IM_COL32(2, 5, 9, (int)(232.f * alpha)));

    for (int i = 0; i < 5; ++i) {
        const float r = 54.f + i * 34.f + pulse * 12.f;
        dl->AddCircle(center, r, IM_COL32(0, 174, 255, (int)((42.f - i * 6.f) * alpha)), 96, 1.2f);
    }

    ImFont* logoF = font::logo();
    const float logoSize = logoF->LegacySize * 1.65f;
    const ImVec2 ts = logoF->CalcTextSizeA(logoSize, FLT_MAX, 0.f, "AUTOPSY");
    const ImVec2 tp(center.x - ts.x * .5f, center.y - 34.f);
    dl->AddText(logoF, logoSize, tp + ImVec2(0.f, 3.f),
        IM_COL32(0, 0, 0, (int)(190.f * alpha)), "AUTOPSY");
    dl->AddText(logoF, logoSize, tp, theme::col_text(), "AUTOPSY");
    dl->AddText(tp + ImVec2(ts.x + 8.f, 6.f), theme::col_accent(), "BETA");

    const char* sub = "loading overlay";
    const ImVec2 ss = ImGui::CalcTextSize(sub);
    dl->AddText(ImVec2(center.x - ss.x * .5f, center.y + 14.f), theme::col_muted(), sub);

    const float barW = ImMin(360.f, display.x * .42f);
    const float progress = ImClamp(elapsed / 4.0f, 0.f, 1.f);
    const ImVec2 bm(center.x - barW * .5f, center.y + 48.f);
    const ImVec2 bM(center.x + barW * .5f, center.y + 54.f);
    dl->AddRectFilled(bm, bM, IM_COL32(12, 26, 38, (int)(220.f * alpha)), 3.f);
    dl->AddRectFilled(bm, ImVec2(bm.x + barW * progress, bM.y), theme::col_accent(), 3.f);
    dl->AddRect(bm, bM, IM_COL32(210, 247, 255, (int)(82.f * alpha)), 3.f);
}

// ========================================================================
// Color namespace (refreshed from global config each frame)
// ========================================================================
namespace color {
    static ImU32 fromfloat(const float col[4], float alphaMul = 1.f) {
        return ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], col[3] * alphaMul));
    }
    static void refresh() {
        // Colors are now handled by theme.h; this is kept for hud namespace compatibility
    }
}

// ---- PC Username forward declaration ----------------------------------
static const char* pcuser();

// ========================================================================
// HUD overlay namespace (watermark, hotkeys, radar, aim warning)
// ========================================================================
namespace hud {
    static ImU32 accent(float a = 1.f) { return theme::col_accent(a); }
    static ImU32 accent2(float a = 1.f) { return theme::col_accent2(a); }
    static int hotkeycount() {
        int c = 0;
        if (global::overlay::Hotkey_Aimbot) ++c;
        if (global::overlay::Hotkey_Silent) ++c;
        if (global::overlay::Hotkey_Fly) ++c;
        if (global::overlay::Hotkey_BladeBallSpam) ++c;
        if (global::overlay::Hotkey_Walkspeed) ++c;
        if (global::overlay::Hotkey_HitboxExpander) ++c;
        return c;
    }

    static void clamppanel(ImVec2& pos, ImVec2 size) {
        const ImVec2 display = ImGui::GetIO().DisplaySize;
        const float pad = 8.f;
        pos.x = ImClamp(pos.x, pad, ImMax(pad, display.x - size.x - pad));
        pos.y = ImClamp(pos.y, pad, ImMax(pad, display.y - size.y - pad));
    }

    static void panelbase(ImDrawList* dl, ImVec2 p, ImVec2 s, bool hovered, bool active) {
        const float r = 11.f;
        const float t = hovered || active ? 1.f : 0.f;
        for (int i = 0; i < 5; i++) {
            const float spread = 9.f + i * 7.f;
            const int a = (int)((30.f - i * 4.8f) + t * 8.f);
            dl->AddRectFilled(p - ImVec2(spread * .45f, spread * .25f) + ImVec2(0.f, 9.f + i * 2.f),
                p + s + ImVec2(spread * .45f, spread * .62f),
                IM_COL32(0, 0, 0, a), r + spread);
        }
        dl->AddRectFilledMultiColorRounded(p - ImVec2(15.f, 10.f), p + s + ImVec2(15.f, 18.f),
            IM_COL32(8, 8, 12, (int)((8.f + t * 5.f))),
            IM_COL32(100, 117, 255, (int)((8.f + t * 5.f))),
            IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), r + 15.f);
        dl->AddRectFilled(p, p + s, IM_COL32(2, 9, 15, 185), r);
        dl->AddRectFilledMultiColorRounded(p + ImVec2(1.f, 1.f), p + s - ImVec2(1.f, 1.f),
            IM_COL32(255, 255, 255, 10), IM_COL32(0, 174, 255, 10),
            IM_COL32(0, 0, 0, 70), IM_COL32(100, 117, 255, 10), r - 1.f);
        dl->AddRect(p, p + s,
            theme::lerp_u32(theme::alpha(accent(), 0.42f), theme::alpha(accent(), 0.82f), t), r, 0, 1.f);
    }

    template <typename DrawFn>
    static void panel(const char* id, ImVec2& pos, ImVec2 size, bool movable, DrawFn draw) {
        clamppanel(pos, size);
        constexpr float drawPad = 30.f;
        ImGui::SetNextWindowPos(pos - ImVec2(drawPad, drawPad), ImGuiCond_Always);
        ImGui::SetNextWindowSize(size + ImVec2(drawPad * 2.f, drawPad * 2.f), ImGuiCond_Always);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.f));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus;
        if (!movable) flags |= ImGuiWindowFlags_NoInputs;

        if (ImGui::Begin(id, nullptr, flags)) {
            ImDrawList* dl2 = ImGui::GetWindowDrawList();
            const ImVec2 p2 = ImGui::GetWindowPos() + ImVec2(drawPad, drawPad);
            ImGui::SetCursorScreenPos(p2);
            ImGui::InvisibleButton("##drag", size);
            const bool hovered = movable && ImGui::IsItemHovered();
            const bool active = movable && ImGui::IsItemActive();
            if (active && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.f)) {
                pos += ImGui::GetIO().MouseDelta;
                clamppanel(pos, size);
            }
            draw(dl2, p2, size, hovered, active);
        }
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

    static void watermark(ImDrawList* dl, ImVec2 p, ImVec2 s, bool hovered, bool active) {
        panelbase(dl, p, s, hovered, active);
        ImFont* logo = font::bold();
        const float logoSize = logo->LegacySize;
        const ImVec2 text = p + ImVec2(15.f, 7.f);
        const ImVec2 nameSize = logo->CalcTextSizeA(logoSize, FLT_MAX, 0.f, "AUTOPSY");
        char fps[32]{};
        std::snprintf(fps, sizeof(fps), "%.0f fps", ImGui::GetIO().Framerate);

        dl->AddRectFilled(p + ImVec2(7.f, 9.f), p + ImVec2(10.f, s.y - 9.f), accent(), 2.f);
        dl->AddText(logo, logoSize, text + ImVec2(1.f, 1.f), IM_COL32(0, 0, 0, 180), "AUTOPSY");
        dl->AddText(logo, logoSize, text, theme::col_text(), "AUTOPSY");
        dl->AddText(text + ImVec2(nameSize.x + 8.f, logoSize * .28f), accent(), "BETA");
        dl->AddText(p + ImVec2(16.f, 25.f), theme::alpha(theme::col_muted(), 0.52f), pcuser());

        const ImVec2 fpsSize = ImGui::CalcTextSize(fps);
        const ImVec2 pillMin = ImVec2(p.x + s.x - fpsSize.x - 28.f, p.y + 10.f);
        const ImVec2 pillMax = ImVec2(p.x + s.x - 10.f, p.y + 29.f);
        dl->AddRectFilled(pillMin, pillMax, IM_COL32(8, 18, 29, 185), 6.f);
        dl->AddRect(pillMin, pillMax, IM_COL32(0, 174, 255, 64), 6.f);
        dl->AddText(ImVec2(pillMin.x + 9.f, pillMin.y + 2.f), accent2(), fps);
    }

    static void hotkeyrow(ImDrawList* dl, ImVec2 pp, float w, const char* label, bool live) {
        const float t = anim::ease_out_cubic(anim::g_anim.get(ImGui::GetID(label, label + strlen(label)), live, 10.f));
        const ImVec2 mn = pp;
        const ImVec2 mx = pp + ImVec2(w, 24.f);
        const ImU32 bg = theme::lerp_u32(IM_COL32(6, 14, 23, 88), IM_COL32(0, 78, 116, 112), t);
        const ImU32 brd = theme::lerp_u32(IM_COL32(25, 48, 64, 92), IM_COL32(0, 174, 255, 132), t);

        dl->AddRectFilled(mn, mx, bg, 7.f);
        dl->AddRect(mn, mx, brd, 7.f, 0, 1.f);
        dl->AddRectFilled(mn + ImVec2(7.f, 7.f), mn + ImVec2(10.f, 17.f),
            theme::lerp_u32(theme::col_muted(), theme::col_accent(), t), 2.f);
        dl->AddText(mn + ImVec2(17.f, 5.f),
            theme::lerp_u32(theme::col_muted(), theme::col_text(), t), label);

        const char* kn = ImGui::GetKeyName(ImGuiKey_None);
        if (!kn || !*kn) kn = "NONE";
        const float keyW = ImMax(42.f, ImGui::CalcTextSize(kn).x + 18.f);
        const ImVec2 keyMin = ImVec2(mx.x - keyW - 8.f, mn.y + 4.f);
        dl->AddRectFilled(keyMin, keyMin + ImVec2(keyW, 16.f),
            theme::lerp_u32(IM_COL32(4, 12, 20, 170), IM_COL32(0, 70, 104, 205), t), 5.f);
        dl->AddRect(keyMin, keyMin + ImVec2(keyW, 16.f),
            theme::lerp_u32(theme::col_border(), theme::col_accent(), t * .8f), 5.f);
        dl->AddText(keyMin + ImVec2((keyW - ImGui::CalcTextSize(kn).x) * .5f, 1.f),
            theme::lerp_u32(theme::col_text(), theme::alpha(theme::col_accent(), 0.85f), t), kn);
    }

    static void hotkey(ImDrawList* dl, ImVec2 p, ImVec2 s, bool hovered, bool active) {
        panelbase(dl, p, s, hovered, active);
        ImFont* title = font::bold();
        dl->AddText(title, title->LegacySize, p + ImVec2(14.f, 11.f), theme::col_text(), "HOTKEYS");
        dl->AddLine(p + ImVec2(14.f, 36.f), p + ImVec2(s.x - 14.f, 36.f),
            theme::col_border(), 1.f);

        const float rowW = s.x - 28.f;
        float y = p.y + 47.f;
        auto row = [&](const char* lbl, bool live) {
            hotkeyrow(dl, ImVec2(p.x + 14.f, y), rowW, lbl, live);
            y += 30.f;
        };

        if (global::overlay::Hotkey_Aimbot)           row("Aimbot", global::aim::Enabled);
        if (global::overlay::Hotkey_Silent)           row("Silent", global::silent::Enabled);
        if (global::overlay::Hotkey_Fly)              row("Fly", global::misc::fly);
        if (global::overlay::Hotkey_BladeBallSpam)    row("Blade Spam", global::ball::SpamParry);
        if (global::overlay::Hotkey_Walkspeed)        row("Walkspeed", global::misc::walkspeed);
        if (global::overlay::Hotkey_HitboxExpander)   row("Hitbox", global::misc::hitbox);

        if (y == p.y + 47.f)
            dl->AddText(p + ImVec2(14.f, 50.f), theme::col_muted(), "No hotkeys selected");
    }

    // ---- Radar helpers ----
    static float dot(const sdk::vector3& a, const sdk::vector3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
    static bool normalize(sdk::vector3& v) {
        const float mag = v.magnitude();
        if (mag < .001f || std::isnan(mag)) return false;
        v = v / mag;
        return true;
    }
    static bool partpose(const sdk::instance& inst, sdk::vector3& out, sdk::matrix3& rot) {
        if (!inst.Address) return false;
        sdk::part part(inst.Address);
        sdk::part prim = part.primitive();
        if (!prim.Address) return false;
        out = prim.position();
        rot = prim.rotation();
        return !(std::isnan(out.x) || std::isnan(out.y) || std::isnan(out.z));
    }
    static bool partposition(const sdk::instance& inst, sdk::vector3& out) {
        sdk::matrix3 ign{};
        return partpose(inst, out, ign);
    }
    static bool playerposition(const sdk::player& player, sdk::vector3& out) {
        if (partposition(player.HumanoidRootPart, out)) return true;
        if (partposition(player.LowerTorso, out)) return true;
        if (partposition(player.Torso, out)) return true;
        return partposition(player.Head, out);
    }
    static bool aimray(const sdk::player& player, sdk::vector3& origin, sdk::vector3& dir) {
        sdk::matrix3 rot{};
        if (!partpose(player.Head, origin, rot) && !partpose(player.HumanoidRootPart, origin, rot) && !partpose(player.Torso, origin, rot))
            return false;
        dir = rot * sdk::vector3{ 0.f, 0.f, -1.f };
        return normalize(dir);
    }
    static bool aiminglocal(const sdk::player& player, const sdk::vector3& localPos) {
        sdk::vector3 origin{}, dir{};
        if (!aimray(player, origin, dir)) return false;
        const sdk::vector3 toLocal = localPos - origin;
        const float along = dot(toLocal, dir);
        const float maxLength = ImClamp(global::overlay::AimView_MaxLength, 50.f, 1000.f);
        if (along < 0.f || along > maxLength) return false;
        const sdk::vector3 closest = origin + dir * along;
        const float miss = closest.distance(localPos);
        const float tolerance = ImClamp(toLocal.magnitude() * .018f, 4.0f, 10.0f);
        return miss <= tolerance;
    }

    static ImVec2 radardelta(const sdk::vector3& local, const sdk::vector3& target) {
        const sdk::vector3 delta = target - local;
        float x = delta.x, y = delta.z;
        if (global::overlay::Radar_Rotate && global::camera.Address) {
            const sdk::matrix3 cam = global::camera.rotation();
            const sdk::vector3 fwd{ -cam.data[2], -cam.data[5], -cam.data[8] };
            const float yaw = atan2f(fwd.x, fwd.z);
            const float c = cosf(-yaw), s = sinf(-yaw);
            const float rx = x * c - y * s, ry = x * s + y * c;
            x = rx; y = ry;
        }
        const float zoom = ImMax(.05f, global::overlay::Radar_Zoom);
        return ImVec2(x * zoom, y * zoom);
    }

    static void radarblip(ImDrawList* dl, ImVec2 center, ImVec2 delta, float limit, bool circle, ImU32 col, float rad) {
        if (circle) {
            const float len = sqrtf(delta.x * delta.x + delta.y * delta.y);
            if (len > limit && len > .001f) delta = ImVec2(delta.x / len * limit, delta.y / len * limit);
        } else {
            delta.x = ImClamp(delta.x, -limit, limit);
            delta.y = ImClamp(delta.y, -limit, limit);
        }
        const ImVec2 pt = center + delta;
        dl->AddCircleFilled(pt + ImVec2(0.f, 1.f), rad + 1.6f, IM_COL32(0, 0, 0, 130), 16);
        dl->AddCircleFilled(pt, rad, col, 16);
        dl->AddCircle(pt, rad + 2.2f, theme::alpha(col, .36f), 16, 1.f);
    }

    static void radar(ImDrawList* dl, ImVec2 p, ImVec2 s, bool hovered, bool active) {
        panelbase(dl, p, s, hovered, active);
        const bool circle = global::overlay::Radar_Shape == 0;
        const ImVec2 center = p + s * .5f;
        const float radius = ImMin(s.x, s.y) * .5f - 18.f;
        const ImVec2 rmin = center - ImVec2(radius, radius);
        const ImVec2 rmax = center + ImVec2(radius, radius);

        if (circle) {
            dl->AddCircleFilled(center, radius, IM_COL32(2, 9, 15, 126), 96);
            dl->AddCircle(center, radius, IM_COL32(0, 174, 255, 132), 96, 1.4f);
            dl->AddCircle(center, radius * .66f, theme::col_border(), 96, 1.f);
            dl->AddCircle(center, radius * .33f, theme::col_border(), 96, 1.f);
        } else {
            dl->AddRectFilled(rmin, rmax, IM_COL32(2, 9, 15, 126), 5.f);
            dl->AddRect(rmin, rmax, IM_COL32(0, 174, 255, 132), 5.f, 0, 1.4f);
            dl->AddRect(center - ImVec2(radius * .66f, radius * .66f),
                center + ImVec2(radius * .66f, radius * .66f), theme::col_border(), 3.f);
            dl->AddRect(center - ImVec2(radius * .33f, radius * .33f),
                center + ImVec2(radius * .33f, radius * .33f), theme::col_border(), 3.f);
        }
        dl->AddLine(ImVec2(center.x - radius, center.y), ImVec2(center.x + radius, center.y),
            theme::alpha(theme::col_accent(), .28f), 1.f);
        dl->AddLine(ImVec2(center.x, center.y - radius), ImVec2(center.x, center.y + radius),
            theme::alpha(theme::col_accent(), .28f), 1.f);

        sdk::vector3 localPos{};
        if (playerposition(global::LocalPlayer, localPos)) {
            for (const auto& player : global::Player_Cache) {
                if (!player.character.Address) continue;
                if (global::LocalPlayer.character.Address &&
                    player.character.Address == global::LocalPlayer.character.Address) continue;
                sdk::vector3 pp{};
                if (!playerposition(player, pp)) continue;
                const float dist = localPos.distance(pp);
                const float fade = 1.f - ImClamp(dist / 900.f, 0.f, .55f);
                radarblip(dl, center, radardelta(localPos, pp), radius - 8.f, circle,
                    theme::alpha(theme::col_accent(), fade * 0.85f), 3.6f);
            }
        }
        const ImVec2 tri[3] = { center + ImVec2(0.f, -7.f), center + ImVec2(5.5f, 6.f), center + ImVec2(-5.5f, 6.f) };
        dl->AddTriangleFilled(tri[0] + ImVec2(0.f, 1.f), tri[1] + ImVec2(0.f, 1.f), tri[2] + ImVec2(0.f, 1.f), IM_COL32(0, 0, 0, 150));
        dl->AddTriangleFilled(tri[0], tri[1], tri[2], theme::col_text());
        ImFont* tf = font::bold();
        dl->AddText(tf, tf->LegacySize, p + ImVec2(14.f, 10.f), theme::col_text(), "RADAR");
        char zt[32]{}; std::snprintf(zt, sizeof(zt), "%.2fx", global::overlay::Radar_Zoom);
        const ImVec2 zs = ImGui::CalcTextSize(zt);
        dl->AddText(ImVec2(p.x + s.x - zs.x - 14.f, p.y + 11.f), accent2(), zt);
    }

    struct aimthreat {
        int Count = 0;
        std::string name;
    };

    static aimthreat getthreat() {
        aimthreat threat{};
        sdk::vector3 localPos{};
        if (!playerposition(global::LocalPlayer, localPos)) return threat;
        for (const auto& player : global::Player_Cache) {
            if (!player.character.Address) continue;
            if (global::LocalPlayer.character.Address &&
                player.character.Address == global::LocalPlayer.character.Address) continue;
            if (!aiminglocal(player, localPos)) continue;
            ++threat.Count;
            if (threat.name.empty())
                threat.name = !player.Display_Name.empty() ? player.Display_Name : player.name;
        }
        return threat;
    }

    static void aimwarning(ImDrawList* dl) {
        const aimthreat threat = getthreat();
        if (threat.Count <= 0) return;
        const ImVec2 display = ImGui::GetIO().DisplaySize;
        const float pulse = (sinf((float)ImGui::GetTime() * 6.0f) + 1.f) * .5f;
        const char* title = "AIM WARNING";
        char detail[96]{};
        if (threat.Count == 1)
            std::snprintf(detail, sizeof(detail), "%s is aiming at you", threat.name.c_str());
        else
            std::snprintf(detail, sizeof(detail), "%dx players aiming at you", threat.Count);

        ImFont* font = font::bold();
        const float fontSize = font->LegacySize;
        const ImVec2 ts = font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, title);
        const ImVec2 ds = ImGui::CalcTextSize(detail);
        const float width = ImClamp(ImMax(ts.x, ds.x) + 74.f, 258.f, 420.f);
        const ImVec2 wm(display.x * .5f - width * .5f, 72.f);
        const ImVec2 wM(wm.x + width, wm.y + 58.f);

        for (int i = 0; i < 4; ++i) {
            const float sp = 5.f + i * 5.f;
            dl->AddRectFilled(wm - ImVec2(sp, sp * .45f) + ImVec2(0.f, 5.f + i),
                wM + ImVec2(sp, sp * .7f), IM_COL32(0, 0, 0, 42 - i * 7), 10.f + sp);
        }
        dl->AddRectFilled(wm, wM, IM_COL32(23, 6, 12, 222), 9.f);
        dl->AddRectFilledMultiColorRounded(wm + ImVec2(1.f, 1.f), wM - ImVec2(1.f, 1.f),
            IM_COL32(255, 80, 104, 40 + (int)(pulse * 34.f)),
            IM_COL32(100, 117, 255, 24), IM_COL32(0, 0, 0, 82),
            IM_COL32(255, 80, 104, 18), 9.f);
        dl->AddRect(wm, wM, IM_COL32(255, 80, 104, 170 + (int)(pulse * 60.f)), 9.f, 0, 1.35f);

        const ImVec2 icon(wm.x + 22.f, wm.y + 29.f);
        dl->AddTriangleFilled(icon + ImVec2(0.f, -12.f), icon + ImVec2(12.f, 10.f),
            icon + ImVec2(-12.f, 10.f), IM_COL32(255, 80, 104, 235));
        dl->AddText(font, fontSize, ImVec2(icon.x - 3.f, icon.y - 8.f), IM_COL32(23, 6, 12, 255), "!");
        dl->AddText(font, fontSize, ImVec2(wm.x + 48.f, wm.y + 9.f), IM_COL32(255, 238, 241, 255), title);
        dl->AddText(ImVec2(wm.x + 48.f, wm.y + 33.f), IM_COL32(255, 178, 190, 255), detail);
    }
} // namespace hud

// ---- Menu (main) --------------------------------------------------------
void graphic::menu() {
    color::refresh();

    static int section = 0;
    static int prevSection = 0;
    static float sectionAlpha = 1.f;
    static float headerAlpha = 1.f;
    static float menuT = 0.f;
    static bool menuPosReady = false;
    static ImVec2 menuPos = {};

    ImGuiIO& IO = ImGui::GetIO();
    const bool compact = global::setting::Compact_UI;
    const float kWinW = compact ? 674.f : theme::kWinW;
    const float kWinH = compact ? 512.f : theme::kWinH;
    constexpr float kR = 14.f;
    const float sbW = compact ? theme::kSidebarCompactW : theme::kSidebarW;
    const float kHeaderH = compact ? 48.f : theme::kHeaderH;

    menuT = anim::damp(menuT, Running ? 1.f : 0.f, Running ? 8.5f : 7.2f);
    if (!Running && menuT <= .01f) return;

    const float menuEase = anim::ease_out_quint(menuT);
    const float menuAlpha = anim::ease_out_cubic(menuT);
    const ImVec2 menuOffset = { 0.f, (1.f - menuEase) * 18.f };

    if (!menuPosReady) { menuPos = IO.DisplaySize / 2.f; menuPosReady = true; }

    const float minX = kWinW * .5f, maxX = IO.DisplaySize.x - kWinW * .5f;
    const float minY = kWinH * .5f, maxY = IO.DisplaySize.y - kWinH * .5f;
    menuPos.x = maxX > minX ? ImClamp(menuPos.x, minX, maxX) : IO.DisplaySize.x * .5f;
    menuPos.y = maxY > minY ? ImClamp(menuPos.y, minY, maxY) : IO.DisplaySize.y * .5f;

    const ImVec2 menuMin = menuPos + menuOffset - ImVec2(kWinW, kWinH) * .5f;
    const ImVec2 menuMax = menuMin + ImVec2(kWinW, kWinH);

    ImDrawList* BDL = ImGui::GetBackgroundDrawList();
    layout::backdrop(BDL, menuMin, menuMax, menuT, kR);
    layout::dissolve(BDL, menuMin, menuMax, menuT, (float)ImGui::GetTime());

    ImGui::SetNextWindowSize({ kWinW, kWinH }, ImGuiCond_Always);
    ImGui::SetNextWindowPos(menuPos + menuOffset, ImGuiCond_Always, { 0.5f, 0.5f });

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.f, 0.f });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, menuAlpha);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, theme::to_u32(theme::c_bg));

    const bool open = ImGui::Begin("##autopsy.lol", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoBackground);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
    if (!open) { ImGui::End(); return; }

    ImVec2 WP = ImGui::GetWindowPos();
    const ImVec2 WS = ImGui::GetWindowSize();
    ImDrawList* DL = ImGui::GetWindowDrawList();
    const float glow = (sinf((float)ImGui::GetTime() * 2.3f) + 1.f) * .5f;

    auto dragMenu = [&]() {
        if (Running && ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.f)) {
            menuPos += IO.MouseDelta;
            menuPos.x = maxX > minX ? ImClamp(menuPos.x, minX, maxX) : IO.DisplaySize.x * .5f;
            menuPos.y = maxY > minY ? ImClamp(menuPos.y, minY, maxY) : IO.DisplaySize.y * .5f;
            ImGui::SetWindowPos(menuPos + menuOffset - WS * .5f, ImGuiCond_Always);
            WP = ImGui::GetWindowPos();
        }
    };

    ImGui::SetCursorScreenPos(WP);
    ImGui::InvisibleButton("##drag_logo", { sbW, compact ? 8.f : theme::kLogoH });
    dragMenu();
    ImGui::SetCursorScreenPos({ WP.x + sbW, WP.y });
    ImGui::InvisibleButton("##drag_header", { WS.x - sbW, kHeaderH });
    dragMenu();

    DL->PushClipRect(WP - ImVec2(40.f, 40.f), WP + WS + ImVec2(40.f, 40.f), false);
    DL->AddRectFilledMultiColorRounded(WP - ImVec2(12.f, 2.f) + ImVec2(0.f, 20.f),
        WP + WS + ImVec2(12.f, 34.f),
        IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0),
        IM_COL32(0, 0, 0, (int)(menuEase * 24.f)),
        IM_COL32(0, 0, 0, (int)(menuEase * 24.f)), kR + 18.f);
    DL->AddRectFilledMultiColorRounded(WP - ImVec2(5.f, 1.f) + ImVec2(0.f, 10.f),
        WP + WS + ImVec2(5.f, 18.f),
        IM_COL32(0, 0, 0, (int)(menuEase * 8.f)),
        IM_COL32(0, 0, 0, (int)(menuEase * 8.f)),
        IM_COL32(0, 0, 0, (int)(menuEase * 48.f)),
        IM_COL32(0, 0, 0, (int)(menuEase * 48.f)), kR + 8.f);
    DL->AddRectFilled(WP + ImVec2(0.f, 5.f), WP + WS + ImVec2(0.f, 5.f),
        IM_COL32(0, 0, 0, (int)(menuEase * 72.f)), kR + 2.f);
    DL->AddRectFilled(WP, WP + WS, IM_COL32(3, 6, 10, 58), kR);
    DL->AddRectFilled(WP + ImVec2(1.f, 1.f), WP + WS - ImVec2(1.f, 1.f),
        IM_COL32(0, 174, 255, 7), kR - 1.f);
    DL->AddRect(WP, WP + WS, theme::col_border(), kR, 0, 1.f);
    DL->AddRect(WP + ImVec2(1.f, 1.f), WP + WS - ImVec2(1.f, 1.f),
        theme::lerp_u32(IM_COL32(0, 174, 255, 28),
            IM_COL32(100, 117, 255, 24), glow), kR, 0, 1.f);
    DL->PopClipRect();

    layout::sidebar(DL, WP, WS, compact, section, pcuser());

    if (prevSection != section) {
        prevSection = section;
        sectionAlpha = 0.f;
        headerAlpha = 0.f;
    }
    sectionAlpha = anim::damp(sectionAlpha, 1.f, 8.5f);
    const float sectionEase = anim::ease_out_cubic(sectionAlpha);
    headerAlpha = anim::damp(headerAlpha, 1.f, 9.5f);
    const float headerEase = anim::ease_out_cubic(headerAlpha);

    layout::header(DL, WP, WS, sbW, section, prevSection, headerEase, headerAlpha, compact);

    const float contentX = WP.x + sbW + 1.f;
    const float contentW = WS.x - sbW - 1.f;
    const float bodyY = WP.y + kHeaderH + 1.f;
    DL->AddRectFilled({ contentX, bodyY },
        { contentX + contentW - 1.f, WP.y + WS.y - 1.f },
        IM_COL32(5, 9, 15, 255), kR, ImDrawFlags_RoundCornersBottomRight);

    constexpr float kPad = 12.f;
    ImGui::SetCursorScreenPos({ contentX + kPad + (1.f - sectionEase) * 18.f, bodyY + kPad });
    ImGui::PushClipRect({ contentX, bodyY }, { contentX + contentW, WP.y + WS.y }, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, sectionEase);
    ImGui::BeginGroup();

    const float bInW = contentW - kPad * 2.f;
    const float bInH = bodyY + WS.y - bodyY - kPad * 2.f;

    switch (section) {
    case 0: page::aimbot(bInW, bInH); break;
    case 1: page::visuals(bInW, bInH); break;
    case 2: page::world(bInW, bInH); break;
    case 3: page::misc(bInW, bInH); break;
    case 4: page::bladeball(bInW, bInH); break;
    case 5: page::settings(bInW, bInH); break;
    }

    ImGui::EndGroup();
    ImGui::PopStyleVar();
    ImGui::PopClipRect();
    ImGui::End();
}

// ---- Visual (overlay rendering) -----------------------------------------
void graphic::visual() {
    ball::render();
    esp::run();
    cursor();
    notify::render();

    const bool movable = Running;
    if (global::overlay::AimWarning)
        hud::aimwarning(ImGui::GetBackgroundDrawList());
    if (global::overlay::watermark)
        hud::panel("##autopsy_watermark", global::overlay::Watermark_Pos,
            ImVec2(258.f, 42.f), movable,
            [](ImDrawList* dl, ImVec2 p, ImVec2 s, bool h, bool a) { hud::watermark(dl, p, s, h, a); });
    if (global::overlay::hotkey) {
        const int rows = hud::hotkeycount();
        hud::panel("##autopsy_hotkeys", global::overlay::Hotkeys_Pos,
            ImVec2(258.f, 52.f + ImMax(1, rows) * 30.f), movable,
            [](ImDrawList* dl, ImVec2 p, ImVec2 s, bool h, bool a) { hud::hotkey(dl, p, s, h, a); });
    }
    if (global::overlay::radar) {
        const float radarSize = ImClamp(global::overlay::Radar_Size, 130.f, 280.f);
        hud::panel("##autopsy_radar", global::overlay::Radar_Pos,
            ImVec2(radarSize, radarSize), movable,
            [](ImDrawList* dl, ImVec2 p, ImVec2 s, bool h, bool a) { hud::radar(dl, p, s, h, a); });
    }

    welcome(Running);
}

// ---- End frame ----------------------------------------------------------
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
