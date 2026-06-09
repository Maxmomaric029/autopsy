#include "controls.h"

// ========================================================================
// Card widget implementation
// ========================================================================

bool w::card::begin(const char* id, ImVec2 size, const char* title) {
    const ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Shadow layers
    if (size.x > 0.f && size.y > 0.f) {
        dl->AddRectFilled(p + ImVec2(0.f, 4.f), p + size + ImVec2(0.f, 4.f),
            IM_COL32(0, 0, 0, 46), 12.f);
        dl->AddRectFilled(p, p + size, IM_COL32(7, 13, 21, 235), 12.f);
    }

    ImGui::PushStyleColor(ImGuiCol_ChildBg, theme::to_u32(theme::c_card));
    ImGui::PushStyleColor(ImGuiCol_Border, theme::to_u32(theme::c_border));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(13.f, 12.f));

    const bool open = ImGui::BeginChild(id, size, true, ImGuiWindowFlags_None);
    if (open && title) {
        w::labelsection(title);
        w::gap(2.f);
    }
    return open;
}

void w::card::end() {
    ImGui::EndChild();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}
