#include "controls.h"

// ========================================================================
// Card widget implementation — polished 2-layer shadow + gradient (F2.2)
// ========================================================================

bool w::card::begin(const char* id, ImVec2 size, const char* title) {
    const ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImGuiID hoverId = ImGui::GetID(id);
    const bool hovered = ImGui::IsMouseHoveringRect(p, p + size);

    if (size.x > 0.f && size.y > 0.f) {
        // ---- Hover animation ----
        float hoverT = anim::g_anim.get(hoverId, hovered, 8.f);

        // ---- 3-layer shadow system ----
        // Layer 0: tight contact shadow
        dl->AddRectFilled(p + ImVec2(0.f, 1.f), p + size + ImVec2(0.f, 3.f),
            IM_COL32(0, 0, 0, 110), 13.f);
        // Layer 1: ambient
        dl->AddRectFilled(p + ImVec2(0.f, 3.f), p + size + ImVec2(0.f, 8.f),
            IM_COL32(0, 0, 0, 65), 16.f);
        // Layer 2: color tint shadow (gives depth against dark bg)
        dl->AddRectFilled(p + ImVec2(-1.f, 2.f), p + size + ImVec2(1.f, 6.f),
            IM_COL32(180, 40, 50, 12), 13.f);

        // ---- Card background with vertical gradient (increased contrast) ----
        dl->AddRectFilledMultiColorRounded(p, p + size,
            IM_COL32(16, 21, 32, 252),             // top — slightly lighter blue-black
            IM_COL32(7, 10, 15, 252),               // bottom — darker
            IM_COL32(16, 21, 32, 252),
            IM_COL32(7, 10, 15, 252), 12.f);

        // ---- Hairline border (increased alpha: 22 at rest, 40 on hover) ----
        int borderAlpha = (int)(22 + hoverT * 18); // 22 -> 40
        dl->AddRect(p, p + size,
            IM_COL32(255, 255, 255, borderAlpha), 12.f, 0, 1.f);

        // ---- Glass shimmer on top edge ----
        dl->AddRectFilledMultiColor(
            p, p + ImVec2(size.x, 1.f),
            IM_COL32(255, 255, 255, 18), IM_COL32(255, 255, 255, 6),
            IM_COL32(255, 255, 255, 6), IM_COL32(255, 255, 255, 18));

        // ---- Red accent glow — only on hover (alpha animated) ----
        if (hoverT > 0.02f) {
            int glowAlpha = (int)(hoverT * 45);
            dl->AddRectFilledMultiColorRounded(
                p, p + ImVec2(size.x * 0.6f, 2.f),
                IM_COL32(220, 50, 65, glowAlpha),
                IM_COL32(220, 50, 65, 0),
                IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), 12.f);
        }
    }

    ImGui::PushStyleColor(ImGuiCol_ChildBg,  IM_COL32(12, 6, 10, 0)); // transparent — we draw bg
    ImGui::PushStyleColor(ImGuiCol_Border,   IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,    12.f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize,  0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    ImVec2(16.f, 16.f)); // space::lg

    const bool open = ImGui::BeginChild(id, size, false,
        ImGuiWindowFlags_None);
    if (open && title) {
        w::labelsection(title);
        w::gap(theme::space::sm);
    }
    return open;
}

void w::card::end() {
    ImGui::EndChild();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}
