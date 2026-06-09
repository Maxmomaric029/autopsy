#include "controls.h"

// ========================================================================
// Card widget implementation — premium glassmorphism cards
// ========================================================================

bool w::card::begin(const char* id, ImVec2 size, const char* title) {
    const ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    if (size.x > 0.f && size.y > 0.f) {
        // ---- Deep multi-layer shadow ----
        for (int i = 0; i < 5; i++) {
            float spread = 3.f + i * 4.f;
            int alpha = (int)(38.f - i * 6.f);
            dl->AddRectFilled(
                p + ImVec2(-spread * 0.3f, spread * 0.4f),
                p + size + ImVec2(spread * 0.3f, spread * 0.7f),
                IM_COL32(0, 0, 0, alpha), 14.f + spread);
        }

        // ---- Card background (dark maroon/red-black instead of blue-purple) ----
        dl->AddRectFilled(p, p + size, IM_COL32(12, 6, 10, 245), 12.f);

        // ---- Shimmer on top edge (glass feel) ----
        dl->AddRectFilledMultiColor(
            p, p + ImVec2(size.x, 1.f),
            IM_COL32(255, 255, 255, 18), IM_COL32(255, 255, 255, 6),
            IM_COL32(255, 255, 255, 6), IM_COL32(255, 255, 255, 18));

        // ---- Subtle border ----
        dl->AddRect(p, p + size,
            IM_COL32(18, 32, 52, 220), 12.f, 0, 1.f);

        // ---- Accent glow in top-left ----
        dl->AddRectFilledMultiColorRounded(
            p, p + ImVec2(size.x * 0.6f, 2.f),
            IM_COL32(220, 50, 65, 45), IM_COL32(220, 50, 65, 0),
            IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), 12.f);
    }

    ImGui::PushStyleColor(ImGuiCol_ChildBg,  IM_COL32(12, 6, 10, 0)); // transparent — we draw bg
    ImGui::PushStyleColor(ImGuiCol_Border,   IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,    12.f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize,  0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    ImVec2(16.f, 14.f));

    const bool open = ImGui::BeginChild(id, size, false, ImGuiWindowFlags_None);
    if (open && title) {
        w::labelsection(title);
        w::gap(3.f);
    }
    return open;
}

void w::card::end() {
    ImGui::EndChild();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}
