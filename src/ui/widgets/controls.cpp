#include "controls.h"

// ========================================================================
// Card widget — industrial-minimal with 3-layer shadow (per spec)
// ========================================================================

bool w::card::begin(const char* id, ImVec2 size, const char* title) {
    const ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImGuiID hoverId = ImGui::GetID(id);
    const bool hovered = ImGui::IsMouseHoveringRect(p, p + size);

    if (size.x > 0.f && size.y > 0.f) {
        // ---- Hover animation ----
        float hoverT = anim::g_anim.get(hoverId, hovered, 8.f);

        // ---- 3-layer shadow (per spec) ----
        // Layer 0: offset (0,1) size +3px, black alpha 110, r=13
        dl->AddRectFilled(p + ImVec2(0.f, 1.f) - ImVec2(1.5f, 1.5f),
            p + size + ImVec2(1.5f, 1.5f),
            IM_COL32(0, 0, 0, 110), 13.f);
        // Layer 1: offset (0,3) size +8px, black alpha 65, r=16
        dl->AddRectFilled(p + ImVec2(0.f, 3.f) - ImVec2(4.f, 4.f),
            p + size + ImVec2(4.f, 4.f),
            IM_COL32(0, 0, 0, 65), 16.f);
        // Layer 2: offset (0,2) size +6px, rgba(180,40,50,12), r=13 — tint depth
        dl->AddRectFilled(p + ImVec2(0.f, 2.f) - ImVec2(3.f, 3.f),
            p + size + ImVec2(3.f, 3.f),
            IM_COL32(180, 40, 50, 12), 13.f);

        // ---- Card background: vertical gradient SURFACE → darker bottom ----
        dl->AddRectFilledMultiColorRounded(p, p + size,
            IM_COL32(17, 17, 20, 252),              // top: SURFACE
            IM_COL32(12, 12, 15, 252),               // bottom: slightly darker
            IM_COL32(17, 17, 20, 252),
            IM_COL32(12, 12, 15, 252), 10.f);       // r_card

        // ---- Hairline border ----
        int borderAlpha = (int)(15 + hoverT * 15); // 15 → 30 on hover
        dl->AddRect(p, p + size,
            IM_COL32(255, 255, 255, borderAlpha), 10.f, 0, 1.f);

        // ---- Top shimmer: 1px line with horizontal fade ----
        dl->AddRectFilledMultiColor(
            p + ImVec2(1.f, 1.f), p + ImVec2(size.x - 1.f, 2.f),
            IM_COL32(255, 255, 255, 18), IM_COL32(255, 255, 255, 6),
            IM_COL32(255, 255, 255, 6), IM_COL32(255, 255, 255, 18));

        // ---- Lime-green accent glow on hover (top-left) ----
        if (hoverT > 0.02f) {
            int glowAlpha = (int)(hoverT * 45);
            dl->AddRectFilledMultiColorRounded(
                p, p + ImVec2(size.x * 0.6f, 2.f),
                IM_COL32(200, 241, 53, glowAlpha),
                IM_COL32(200, 241, 53, 0),
                IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), 10.f);
        }
    }

    ImGui::PushStyleColor(ImGuiCol_ChildBg,  IM_COL32(12, 6, 10, 0));
    ImGui::PushStyleColor(ImGuiCol_Border,   IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,    10.f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize,  0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    ImVec2(16.f, 16.f));

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
