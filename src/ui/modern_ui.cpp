#include "modern_ui.h"
#include <dwmapi.h>
#include <cstdio>
#include <chrono>
#include <thread>
#include "global.h"

#include "../features/esp.h"
#include "../features/ball.h"
#include "../features/silent.h"

#include "ui/core/theme.h"
#include "ui/core/animation.h"
#include "ui/core/fonts.h"
#include "ui/core/icons.h"
#include "ui/core/notifications.h"
#include "ui/core/texture.h"
#include "ui/widgets/controls.h"
#include "ui/layout/menulayout.h"
#include "ui/pages/pages.h"

// ========================================================================
// ModernUI — public API implementation
// ========================================================================

ModernUI::ModernUI() = default;
ModernUI::~ModernUI() { Destroy(); }

bool ModernUI::Create(HWND window, ID3D11Device* device, ID3D11DeviceContext* context) {
    if (m_initialized) return true;

    ImGui::CreateContext();

    // ---- DPI-aware font loading ----
    float dpiScale = ImGui_ImplWin32_GetDpiScaleForMonitor(
        ::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    ImGuiIO& IO = ImGui::GetIO();
    IO.IniFilename = nullptr;

    font::load(dpiScale);

    // ---- Dark style ----
    ImGui::StyleColorsDark();
    ImGuiStyle& S = ImGui::GetStyle();
    S.WindowRounding = theme::r_window;
    S.ChildRounding = theme::r_card;
    S.FrameRounding = theme::r_frame;
    S.PopupRounding = 10.f;
    S.ScrollbarRounding = 8.f;
    S.GrabRounding = 8.f;
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

    // ---- Init backends ----
    if (!ImGui_ImplWin32_Init(window)) return false;
    if (!device || !context) return false;
    if (!ImGui_ImplDX11_Init(device, context)) return false;

    m_initialized = true;
    return true;
}

void ModernUI::Destroy() {
    if (!m_initialized) return;
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    m_initialized = false;
}

// ========================================================================
// Key -> VK conversion (internal helper)
// ========================================================================
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

// ========================================================================
// BeginFrame — MSG pump, toggle detection, ImGui::NewFrame()
// ========================================================================
void ModernUI::BeginFrame(HWND overlayWindow) {
    // ---- MSG pump ----
    MSG Msg;
    while (PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE)) {
        if (Msg.message == WM_QUIT) ExitProcess(0);
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    // ---- Streamproof ----
    static bool lastStreamproof = !global::setting::Streamproof;
    if (lastStreamproof != global::setting::Streamproof) {
        SetWindowDisplayAffinity(overlayWindow,
            global::setting::Streamproof ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);
        lastStreamproof = global::setting::Streamproof;
    }

    // ---- ImGui new frame ----
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // ---- Menu toggle (Insert key) ----
    HWND roblox = FindWindowA(0, "Roblox");
    HWND fg = GetForegroundWindow();
    static double lastToggle = -1.0;
    double now = ImGui::GetTime();
    int menuVk = menukey(global::setting::Menu_Key);
    if (menuVk != 0 && (GetAsyncKeyState(menuVk) & 1) &&
        (fg == roblox || fg == overlayWindow) && now - lastToggle >= .18) {
        lastToggle = now;
        Toggle();
        LONG exStyle = WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED;
        if (!m_open) exStyle |= WS_EX_TRANSPARENT;
        SetWindowLong(overlayWindow, GWL_EXSTYLE, exStyle);
    }
}

// ========================================================================
// RenderMenu — Config menu with glassmorphism gray sidebar
// ========================================================================
void ModernUI::RenderMenu() {
    if (!m_open) {
        static float menuT = 0.f;
        menuT = anim::damp(menuT, 0.f, 7.2f);
        if (menuT <= .01f) return;
    }

    static int section = 0;
    static int prevSection = 0;
    static float sectionAlpha = 1.f;
    static float menuT = 0.f;
    static bool menuPosReady = false;
    static ImVec2 menuPos = {};

    const float kWinW = theme::kWinW;
    const float kWinH = theme::kWinH;
    const float kR = theme::r_window;

    ImGuiIO& IO = ImGui::GetIO();

    menuT = anim::damp(menuT, m_open ? 1.f : 0.f, m_open ? 8.5f : 7.2f);
    if (!m_open && menuT <= .01f) return;

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

    // ---- Backdrop ----
    ImDrawList* BDL = ImGui::GetBackgroundDrawList();
    layout::backdrop(BDL, menuMin, menuMax, menuT, kR);

    // ---- Window ----
    ImGui::SetNextWindowSize({ kWinW, kWinH }, ImGuiCond_Always);
    ImGui::SetNextWindowPos(menuPos + menuOffset, ImGuiCond_Always, { 0.5f, 0.5f });

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.f, 0.f });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, menuAlpha);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, theme::to_u32(theme::c_bg));

    bool open = ImGui::Begin("##miserable", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoBackground);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
    if (!open) { ImGui::End(); return; }

    ImVec2 WP = ImGui::GetWindowPos();
    const ImVec2 WS = ImGui::GetWindowSize();
    ImDrawList* DL = ImGui::GetWindowDrawList();

    // ---- Drag handle (full window top bar) ----
    constexpr float kDragH = 36.f;
    ImGui::SetCursorScreenPos(WP);
    ImGui::InvisibleButton("##drag", { WS.x, kDragH });
    if (m_open && ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.f)) {
        menuPos += IO.MouseDelta;
        menuPos.x = maxX > minX ? ImClamp(menuPos.x, minX, maxX) : IO.DisplaySize.x * .5f;
        menuPos.y = maxY > minY ? ImClamp(menuPos.y, minY, maxY) : IO.DisplaySize.y * .5f;
        ImGui::SetWindowPos(menuPos + menuOffset - WS * .5f, ImGuiCond_Always);
        WP = ImGui::GetWindowPos();
    }

    // ---- Web-style shadow (like an Electron app modal) ----
    const int shadowAlpha = (int)(32.f * menuEase);
    for (int i = 0; i < 3; i++) {
        float spread = 3.f + i * 2.5f;
        int a = (shadowAlpha - i * 8) * (i + 1) / 3;
        DL->AddRectFilled(WP - ImVec2(spread * 0.5f, spread * 0.2f) + ImVec2(0.f, 5.f + i),
            WP + WS + ImVec2(spread * 0.5f, spread * 0.6f),
            IM_COL32(0, 0, 0, ImMax(0, a)), kR + spread);
    }

    // ---- Sidebar ----
    layout::sidebar(DL, WP, WS, section, menuEase);

    // ---- Section transition tracking ----
    if (prevSection != section) {
        prevSection = section;
        sectionAlpha = 0.f;
    }
    sectionAlpha = anim::damp(sectionAlpha, 1.f, 8.5f);
    float sectionEase = anim::ease_out_cubic(sectionAlpha);

    // ---- Content area ----
    layout::contentBg(DL, WP, WS);

    // ---- Vignette overlay on content area ----
    {
        const ImVec2 cMin = WP + ImVec2(theme::kSidebarW, 0.f);
        const ImVec2 cMax = WP + WS;
        // Darker edges — 6 concentric rings fading out
        for (int i = 0; i < 6; i++) {
            float expand = 10.f + i * 15.f;
            int a = (int)((5 - i) * 4);
            if (a <= 0) continue;
            DL->AddRect(
                cMin + ImVec2(expand, expand),
                cMax - ImVec2(expand, expand),
                IM_COL32(0, 0, 0, a),
                theme::r_window * (1.f - (float)i * 0.15f), 0, expand * 0.7f);
        }
        // Subtle scanline overlay (very faint)
        if (menuEase > 0.5f) {
            float scanAlpha = (menuEase - 0.5f) * 2.f * 3.f;
            for (float sy = cMin.y; sy < cMax.y; sy += 4.f) {
                DL->AddLine(
                    { cMin.x, sy }, { cMax.x, sy },
                    IM_COL32(0, 0, 0, (int)scanAlpha), 1.f);
            }
        }
    }

    constexpr float kSideW = 190.f;
    constexpr float kPad = 14.f;
    float contentX = WP.x + kSideW + 1.f;
    float contentY = WP.y + 1.f;
    float contentW = WS.x - kSideW - 2.f;
    float contentH = WS.y - 2.f;

    // Clip content to content area
    DL->PushClipRect({ contentX, contentY }, { contentX + contentW, contentY + contentH }, true);

    ImGui::SetCursorScreenPos({ contentX + kPad + (1.f - sectionEase) * 12.f, contentY + kPad });
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, sectionEase);
    ImGui::BeginGroup();

    float bInW = contentW - kPad * 2.f;
    float bInH = contentH - kPad * 2.f;

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
    DL->PopClipRect();
    ImGui::End();
}

// ========================================================================
// HUD helpers
// ========================================================================
namespace {

    static const char* pcuser() {
        static std::string name = []() {
            char buffer[257]{};
            DWORD len = GetEnvironmentVariableA("USERNAME", buffer, (DWORD)sizeof(buffer));
            if (!len || len >= sizeof(buffer)) return std::string("Windows");
            return std::string(buffer, buffer + len);
        }();
        return name.c_str();
    }

    // ---- Cursor render ----
    static void cursor() {
        if (!SilentAimInstance.Address) return;
        if (!drive->read<bool>(SilentAimInstance.Address + offset::gui::Visible)) return;
        POINT pt;
        if (!GetCursorPos(&pt)) return;
        bool rmbHeld = GetAsyncKeyState(VK_RBUTTON) & 0x8000;
        float gap = rmbHeld ? 4.f : 10.f;
        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        ImU32 col = IM_COL32(255, 255, 255, 255);
        ImVec2 c = { (float)pt.x, (float)pt.y };
        dl->AddRectFilled(c - ImVec2(2.f, 2.f), c + ImVec2(2.f, 2.f), col);
        dl->AddRectFilled({ c.x - 1.f, c.y - gap - 10.f }, { c.x + 1.f, c.y - gap }, col);
        dl->AddRectFilled({ c.x - 1.f, c.y + gap }, { c.x + 1.f, c.y + gap + 10.f }, col);
        dl->AddRectFilled({ c.x - gap - 10.f, c.y - 1.f }, { c.x - gap, c.y + 1.f }, col);
        dl->AddRectFilled({ c.x + gap, c.y - 1.f }, { c.x + gap + 10.f, c.y + 1.f }, col);
    }

    // ---- HUD panel helpers ----
    namespace hud {
        static ImU32 accent(float a = 1.f) { return IM_COL32(220, 60, 70, (int)(255 * a)); }
        static ImU32 accent2(float a = 1.f) { return IM_COL32(180, 80, 90, (int)(255 * a)); }
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
            ImVec2 display = ImGui::GetIO().DisplaySize;
            float pad = 8.f;
            pos.x = ImClamp(pos.x, pad, ImMax(pad, display.x - size.x - pad));
            pos.y = ImClamp(pos.y, pad, ImMax(pad, display.y - size.y - pad));
        }

        static void panelbase(ImDrawList* dl, ImVec2 p, ImVec2 s, bool hovered, bool active) {
            float r = 11.f;
            float t = hovered || active ? 1.f : 0.f;
            for (int i = 0; i < 5; i++) {
                float spread = 9.f + i * 7.f;
                int a = (int)((30.f - i * 4.8f) + t * 8.f);
                dl->AddRectFilled(p - ImVec2(spread * .45f, spread * .25f) + ImVec2(0.f, 9.f + i * 2.f),
                    p + s + ImVec2(spread * .45f, spread * .62f),
                    IM_COL32(0, 0, 0, a), r + spread);
            }
            dl->AddRectFilledMultiColorRounded(p - ImVec2(15.f, 10.f), p + s + ImVec2(15.f, 18.f),
                IM_COL32(8, 8, 12, (int)((8.f + t * 5.f))),
                IM_COL32(140, 50, 60, (int)((8.f + t * 5.f))),
                IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), r + 15.f);
            dl->AddRectFilled(p, p + s, IM_COL32(2, 6, 10, 185), r);
            dl->AddRectFilledMultiColorRounded(p + ImVec2(1.f, 1.f), p + s - ImVec2(1.f, 1.f),
                IM_COL32(255, 255, 255, 10), IM_COL32(200, 60, 70, 10),
                IM_COL32(0, 0, 0, 70), IM_COL32(140, 50, 60, 10), r - 1.f);
            dl->AddRect(p, p + s,
                theme::lerp_u32(IM_COL32(180, 60, 70, 107), IM_COL32(220, 60, 70, 209), t), r, 0, 1.f);
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
                ImVec2 p2 = ImGui::GetWindowPos() + ImVec2(drawPad, drawPad);
                ImGui::SetCursorScreenPos(p2);
                ImGui::InvisibleButton("##drag", size);
                bool hovered = movable && ImGui::IsItemHovered();
                bool active = movable && ImGui::IsItemActive();
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

        // Real FPS counter (500ms update)
        static float realfps() {
            static double lastTime = 0.0;
            static int    frameCount = 0;
            static float  cached = 0.f;
            const double now = ImGui::GetTime();
            ++frameCount;
            if (now - lastTime >= 0.5) {
                cached = (float)(frameCount / (now - lastTime));
                frameCount = 0;
                lastTime = now;
            }
            return cached;
        }

        // CPU usage (1s update via GetSystemTimes)
        static float cpuusage() {
            static FILETIME idleLast{}, kernelLast{}, userLast{};
            static float cached = 0.f;
            static double lastCheck = 0.0;
            const double now = ImGui::GetTime();
            if (now - lastCheck < 1.0) return cached;
            lastCheck = now;
            FILETIME idleNew, kernelNew, userNew;
            if (!GetSystemTimes(&idleNew, &kernelNew, &userNew)) return cached;
            auto sub = [](FILETIME a, FILETIME b) -> uint64_t {
                uint64_t ua = ((uint64_t)a.dwHighDateTime << 32) | a.dwLowDateTime;
                uint64_t ub = ((uint64_t)b.dwHighDateTime << 32) | b.dwLowDateTime;
                return ua - ub;
            };
            uint64_t idle   = sub(idleNew,   idleLast);
            uint64_t kernel = sub(kernelNew, kernelLast);
            uint64_t user   = sub(userNew,   userLast);
            idleLast = idleNew; kernelLast = kernelNew; userLast = userNew;
            uint64_t total = kernel + user;
            if (total == 0) return cached;
            cached = 100.f * (1.f - (float)idle / (float)total);
            return cached;
        }

        static void watermark(ImDrawList* dl, ImVec2 p, ImVec2 s, bool hovered, bool active) {
            panelbase(dl, p, s, hovered, active);
            ImFont* logo = font::bold();
            float logoSize = logo->LegacySize;
            ImVec2 text = p + ImVec2(15.f, 7.f);
            ImVec2 nameSize = logo->CalcTextSizeA(logoSize, FLT_MAX, 0.f, "MISERABLE");
            char fps[32]{}; std::snprintf(fps, sizeof(fps), "%.0ffps  %.0f%%", realfps(), cpuusage());
            dl->AddRectFilled(p + ImVec2(7.f, 9.f), p + ImVec2(10.f, s.y - 9.f), accent(), 2.f);
            dl->AddText(logo, logoSize, text + ImVec2(1.f, 1.f), IM_COL32(0, 0, 0, 180), "MISERABLE");
            dl->AddText(logo, logoSize, text, IM_COL32(230, 60, 70, 245), "MISERABLE");
            dl->AddText(text + ImVec2(nameSize.x + 8.f, logoSize * .28f), accent(), "BETA");
            dl->AddText(p + ImVec2(16.f, 25.f), IM_COL32(140, 150, 170, 132), pcuser());
            float keyW = ImMax(72.f, ImGui::CalcTextSize(fps).x + 20.f);
            ImVec2 pillMin = ImVec2(p.x + s.x - keyW - 18.f, p.y + 10.f);
            ImVec2 pillMax = ImVec2(p.x + s.x - 10.f, p.y + 29.f);
            dl->AddRectFilled(pillMin, pillMax, IM_COL32(8, 12, 18, 185), 6.f);
            dl->AddRect(pillMin, pillMax, IM_COL32(200, 60, 70, 64), 6.f);
            dl->AddText(ImVec2(pillMin.x + 9.f, pillMin.y + 2.f), accent2(), fps);
        }

        static void hotkeyrow(ImDrawList* dl, ImVec2 pp, float w, const char* label, bool live) {
            float t = anim::ease_out_cubic(anim::g_anim.get(ImGui::GetID(label, label + strlen(label)), live, 10.f));
            ImVec2 mn = pp;
            ImVec2 mx = pp + ImVec2(w, 24.f);
            ImU32 bg = theme::lerp_u32(IM_COL32(6, 10, 16, 88), IM_COL32(80, 20, 28, 112), t);
            ImU32 brd = theme::lerp_u32(IM_COL32(20, 30, 40, 92), IM_COL32(200, 60, 70, 132), t);
            dl->AddRectFilled(mn, mx, bg, 7.f);
            dl->AddRect(mn, mx, brd, 7.f, 0, 1.f);
            dl->AddRectFilled(mn + ImVec2(7.f, 7.f), mn + ImVec2(10.f, 17.f),
                theme::lerp_u32(IM_COL32(160, 170, 190, 200), accent(), t), 2.f);
            dl->AddText(mn + ImVec2(17.f, 5.f), theme::lerp_u32(IM_COL32(160, 170, 190, 200), IM_COL32(220, 230, 245, 255), t), label);
            const char* kn = ImGui::GetKeyName(ImGuiKey_None);
            if (!kn || !*kn) kn = "NONE";
            float keyW = ImMax(42.f, ImGui::CalcTextSize(kn).x + 18.f);
            ImVec2 keyMin = ImVec2(mx.x - keyW - 8.f, mn.y + 4.f);
            dl->AddRectFilled(keyMin, keyMin + ImVec2(keyW, 16.f),
                theme::lerp_u32(IM_COL32(4, 8, 14, 170), IM_COL32(70, 18, 24, 205), t), 5.f);
            dl->AddRect(keyMin, keyMin + ImVec2(keyW, 16.f),
                theme::lerp_u32(IM_COL32(25, 40, 55, 200), accent(), t * .8f), 5.f);
            dl->AddText(keyMin + ImVec2((keyW - ImGui::CalcTextSize(kn).x) * .5f, 1.f),
                theme::lerp_u32(IM_COL32(220, 230, 245, 255), accent(), t), kn);
        }

        static void hotkey(ImDrawList* dl, ImVec2 p, ImVec2 s, bool hovered, bool active) {
            panelbase(dl, p, s, hovered, active);
            ImFont* title = font::bold();
            dl->AddText(title, title->LegacySize, p + ImVec2(14.f, 11.f), IM_COL32(220, 230, 245, 255), "HOTKEYS");
            dl->AddLine(p + ImVec2(14.f, 36.f), p + ImVec2(s.x - 14.f, 36.f), IM_COL32(25, 40, 55, 200), 1.f);
            float rowW = s.x - 28.f;
            float y = p.y + 47.f;
            auto row = [&](const char* lbl, bool live) { hotkeyrow(dl, ImVec2(p.x + 14.f, y), rowW, lbl, live); y += 30.f; };
            if (global::overlay::Hotkey_Aimbot) row("Aimbot", global::aim::Enabled);
            if (global::overlay::Hotkey_Silent) row("Silent", global::silent::Enabled);
            if (global::overlay::Hotkey_Fly) row("Fly", global::misc::fly);
            if (global::overlay::Hotkey_BladeBallSpam) row("Blade Spam", global::ball::SpamParry);
            if (global::overlay::Hotkey_Walkspeed) row("Walkspeed", global::misc::walkspeed);
            if (global::overlay::Hotkey_HitboxExpander) row("Hitbox", global::misc::hitbox);
            if (y == p.y + 47.f) dl->AddText(p + ImVec2(14.f, 50.f), IM_COL32(140, 150, 170, 200), "No hotkeys selected");
        }

        // ---- Radar ----
        static float dot(const sdk::vector3& a, const sdk::vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
        static bool normalize(sdk::vector3& v) {
            float mag = v.magnitude();
            if (mag < .001f || std::isnan(mag)) return false;
            v = v / mag; return true;
        }
        static bool partpose(const sdk::instance& inst, sdk::vector3& out, sdk::matrix3& rot) {
            if (!inst.Address) return false;
            sdk::part part(inst.Address); sdk::part prim = part.primitive();
            if (!prim.Address) return false;
            out = prim.position(); rot = prim.rotation();
            return !(std::isnan(out.x) || std::isnan(out.y) || std::isnan(out.z));
        }
        static bool partposition(const sdk::instance& inst, sdk::vector3& out) {
            sdk::matrix3 ign{}; return partpose(inst, out, ign);
        }
        static bool playerposition(const sdk::player& player, sdk::vector3& out) {
            if (partposition(player.HumanoidRootPart, out)) return true;
            if (partposition(player.LowerTorso, out)) return true;
            if (partposition(player.Torso, out)) return true;
            return partposition(player.Head, out);
        }
        static ImVec2 radardelta(const sdk::vector3& local, const sdk::vector3& target) {
            sdk::vector3 delta = target - local;
            float x = delta.x, y = delta.z;
            if (global::overlay::Radar_Rotate && global::camera.Address) {
                sdk::matrix3 cam = global::camera.rotation();
                sdk::vector3 fwd{ -cam.data[2], -cam.data[5], -cam.data[8] };
                float yaw = atan2f(fwd.x, fwd.z);
                float c = cosf(-yaw), s = sinf(-yaw);
                float rx = x * c - y * s, ry = x * s + y * c;
                x = rx; y = ry;
            }
            float zoom = ImMax(.05f, global::overlay::Radar_Zoom);
            return ImVec2(x * zoom, y * zoom);
        }

        static void radarblip(ImDrawList* dl, ImVec2 center, ImVec2 delta, float limit, bool circle, ImU32 col, float rad) {
            if (circle) { float len = sqrtf(delta.x * delta.x + delta.y * delta.y); if (len > limit && len > .001f) delta = ImVec2(delta.x / len * limit, delta.y / len * limit); }
            else { delta.x = ImClamp(delta.x, -limit, limit); delta.y = ImClamp(delta.y, -limit, limit); }
            ImVec2 pt = center + delta;
            dl->AddCircleFilled(pt + ImVec2(0.f, 1.f), rad + 1.6f, IM_COL32(0, 0, 0, 130), 16);
            dl->AddCircleFilled(pt, rad, col, 16);
            dl->AddCircle(pt, rad + 2.2f, theme::alpha(col, .36f), 16, 1.f);
        }

        static void radar(ImDrawList* dl, ImVec2 p, ImVec2 s, bool hovered, bool active) {
            panelbase(dl, p, s, hovered, active);
            bool circle = global::overlay::Radar_Shape == 0;
            ImVec2 center = p + s * .5f;
            float radius = ImMin(s.x, s.y) * .5f - 18.f;
            ImVec2 rmin = center - ImVec2(radius, radius);
            ImVec2 rmax = center + ImVec2(radius, radius);
            if (circle) {
                dl->AddCircleFilled(center, radius, IM_COL32(2, 6, 10, 126), 96);
                dl->AddCircle(center, radius, IM_COL32(200, 60, 70, 132), 96, 1.4f);
                dl->AddCircle(center, radius * .66f, IM_COL32(25, 40, 55, 200), 96, 1.f);
                dl->AddCircle(center, radius * .33f, IM_COL32(25, 40, 55, 200), 96, 1.f);
            } else {
                dl->AddRectFilled(rmin, rmax, IM_COL32(2, 6, 10, 126), 5.f);
                dl->AddRect(rmin, rmax, IM_COL32(200, 60, 70, 132), 5.f, 0, 1.4f);
                dl->AddRect(center - ImVec2(radius * .66f, radius * .66f), center + ImVec2(radius * .66f, radius * .66f), IM_COL32(25, 40, 55, 200), 3.f);
                dl->AddRect(center - ImVec2(radius * .33f, radius * .33f), center + ImVec2(radius * .33f, radius * .33f), IM_COL32(25, 40, 55, 200), 3.f);
            }
            dl->AddLine(ImVec2(center.x - radius, center.y), ImVec2(center.x + radius, center.y), IM_COL32(200, 60, 70, 71), 1.f);
            dl->AddLine(ImVec2(center.x, center.y - radius), ImVec2(center.x, center.y + radius), IM_COL32(200, 60, 70, 71), 1.f);
            sdk::vector3 localPos{};
            if (playerposition(global::LocalPlayer, localPos)) {
                for (const auto& player : global::Player_Cache) {
                    if (!player.character.Address) continue;
                    if (global::LocalPlayer.character.Address && player.character.Address == global::LocalPlayer.character.Address) continue;
                    sdk::vector3 pp{}; if (!playerposition(player, pp)) continue;
                    float dist = localPos.distance(pp);
                    float fade = 1.f - ImClamp(dist / 900.f, 0.f, .55f);
                    radarblip(dl, center, radardelta(localPos, pp), radius - 8.f, circle, IM_COL32(200, 60, 70, (int)(fade * 217)), 3.6f);
                }
            }
            ImVec2 tri[3] = { center + ImVec2(0.f, -7.f), center + ImVec2(5.5f, 6.f), center + ImVec2(-5.5f, 6.f) };
            dl->AddTriangleFilled(tri[0] + ImVec2(0.f, 1.f), tri[1] + ImVec2(0.f, 1.f), tri[2] + ImVec2(0.f, 1.f), IM_COL32(0, 0, 0, 150));
            dl->AddTriangleFilled(tri[0], tri[1], tri[2], IM_COL32(220, 230, 245, 255));
            ImFont* tf = font::bold();
            dl->AddText(tf, tf->LegacySize, p + ImVec2(14.f, 10.f), IM_COL32(220, 230, 245, 255), "RADAR");
            char zt[32]{}; std::snprintf(zt, sizeof(zt), "%.2fx", global::overlay::Radar_Zoom);
            ImVec2 zs = ImGui::CalcTextSize(zt);
            dl->AddText(ImVec2(p.x + s.x - zs.x - 14.f, p.y + 11.f), accent2(), zt);
        }

        // ---- Aim threat detection ----
        struct aimthreat { int Count = 0; std::string name; };

        static aimthreat getthreat() {
            aimthreat threat{};
            sdk::vector3 localPos{};
            if (!playerposition(global::LocalPlayer, localPos)) return threat;
            for (const auto& player : global::Player_Cache) {
                if (!player.character.Address) continue;
                if (global::LocalPlayer.character.Address && player.character.Address == global::LocalPlayer.character.Address) continue;
                sdk::vector3 origin{}, dir{};
                sdk::matrix3 rot{};
                if (!partpose(player.Head, origin, rot) && !partpose(player.HumanoidRootPart, origin, rot) && !partpose(player.Torso, origin, rot)) continue;
                dir = rot * sdk::vector3{ 0.f, 0.f, -1.f };
                if (!normalize(dir)) continue;
                sdk::vector3 toLocal = localPos - origin;
                float along = dot(toLocal, dir);
                float maxLength = ImClamp(global::overlay::AimView_MaxLength, 50.f, 1000.f);
                if (along < 0.f || along > maxLength) continue;
                sdk::vector3 closest = origin + dir * along;
                float miss = closest.distance(localPos);
                float tolerance = ImClamp(toLocal.magnitude() * .018f, 4.0f, 10.0f);
                if (miss > tolerance) continue;
                ++threat.Count;
                if (threat.name.empty()) threat.name = !player.Display_Name.empty() ? player.Display_Name : player.name;
            }
            return threat;
        }

        static void aimwarning(ImDrawList* dl) {
            aimthreat threat = getthreat();
            if (threat.Count <= 0) return;
            ImVec2 display = ImGui::GetIO().DisplaySize;
            float pulse = (sinf((float)ImGui::GetTime() * 6.0f) + 1.f) * .5f;
            const char* title = "AIM WARNING";
            char detail[96]{};
            if (threat.Count == 1) std::snprintf(detail, sizeof(detail), "%s is aiming at you", threat.name.c_str());
            else std::snprintf(detail, sizeof(detail), "%dx players aiming at you", threat.Count);
            ImFont* font = font::bold();
            float fontSize = font->LegacySize;
            ImVec2 ts = font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, title);
            ImVec2 ds = ImGui::CalcTextSize(detail);
            float width = ImClamp(ImMax(ts.x, ds.x) + 74.f, 258.f, 420.f);
            ImVec2 wm(display.x * .5f - width * .5f, 72.f);
            ImVec2 wM(wm.x + width, wm.y + 58.f);
            for (int i = 0; i < 4; ++i) {
                float sp = 5.f + i * 5.f;
                dl->AddRectFilled(wm - ImVec2(sp, sp * .45f) + ImVec2(0.f, 5.f + i), wM + ImVec2(sp, sp * .7f), IM_COL32(0, 0, 0, 42 - i * 7), 10.f + sp);
            }
            dl->AddRectFilled(wm, wM, IM_COL32(23, 6, 12, 222), 9.f);
            dl->AddRectFilledMultiColorRounded(wm + ImVec2(1.f, 1.f), wM - ImVec2(1.f, 1.f),
                IM_COL32(255, 80, 104, 40 + (int)(pulse * 34.f)), IM_COL32(140, 50, 60, 24),
                IM_COL32(0, 0, 0, 82), IM_COL32(255, 80, 104, 18), 9.f);
            dl->AddRect(wm, wM, IM_COL32(255, 80, 104, 170 + (int)(pulse * 60.f)), 9.f, 0, 1.35f);
            ImVec2 icon(wm.x + 22.f, wm.y + 29.f);
            dl->AddTriangleFilled(icon + ImVec2(0.f, -12.f), icon + ImVec2(12.f, 10.f), icon + ImVec2(-12.f, 10.f), IM_COL32(255, 80, 104, 235));
            dl->AddText(font, fontSize, ImVec2(icon.x - 3.f, icon.y - 8.f), IM_COL32(23, 6, 12, 255), "!");
            dl->AddText(font, fontSize, ImVec2(wm.x + 48.f, wm.y + 9.f), IM_COL32(255, 238, 241, 255), title);
            dl->AddText(ImVec2(wm.x + 48.f, wm.y + 33.f), IM_COL32(255, 178, 190, 255), detail);
        }
    } // namespace hud
} // anonymous namespace

// ========================================================================
// RenderESP — HUD overlay
// ========================================================================
void ModernUI::RenderESP() {
    ball::render();
    esp::run();
    cursor();
    notify::render();

    bool movable = m_open;
    if (global::overlay::AimWarning)
        hud::aimwarning(ImGui::GetBackgroundDrawList());
    if (global::overlay::watermark)
        hud::panel("##miserable_watermark", global::overlay::Watermark_Pos,
            ImVec2(258.f, 42.f), movable,
            [](ImDrawList* dl, ImVec2 p, ImVec2 s, bool h, bool a) { hud::watermark(dl, p, s, h, a); });
    if (global::overlay::radar) {
        float radarSize = ImClamp(global::overlay::Radar_Size, 130.f, 280.f);
        hud::panel("##miserable_radar", global::overlay::Radar_Pos,
            ImVec2(radarSize, radarSize), movable,
            [](ImDrawList* dl, ImVec2 p, ImVec2 s, bool h, bool a) { hud::radar(dl, p, s, h, a); });
    }

}

// ========================================================================
// EndFrame — Render + Present
// ========================================================================
void ModernUI::EndFrame(IDXGISwapChain* swapChain) {
    if (!swapChain) return;
    ImGui::Render();
    // Rendering + Present handled by parent (graphic class)
}
