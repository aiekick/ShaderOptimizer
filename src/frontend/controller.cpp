#include <frontend/controller.h>
#include <panes/sourcePane.h>
#include <panes/targetPane.h>
#include <ShaderOpt/flopEstimator.h>

bool Controller::init() {
    if (m_controller.init()) {
        m_outputTypeCombo.init(0, {"GLSL", "HLSL", "MSL", "SPIRV", "AST", "CPP"});
        return true;
    }
    return false;
}

void Controller::unit() {
    m_controller.unit();
}

void Controller::drawUI() {
    m_outputTypeCombo.display(200.0f, "Output type");
    if (ImGui::ContrastedButton("Optimize")) {
        m_controller.setSource(SourcePane::ref()->getCode());
        ShaderOpt::Controller::Config config;
        config.debug = false;
        config.outputType = static_cast<ShaderOpt::Controller::Config::OutputType>(m_outputTypeCombo.getIndex());
        m_result = m_controller.optimize(config);
        if (m_result.valid) {
            TargetPane::ref()->setCode(m_controller.getTarget());
        }
    }
    if (m_result.valid && m_result.stats.total > 0.0) {
        ImGui::Text("Total Flops : %.0f", m_result.stats.total);
        static ImGuiTableFlags flags =        //
            ImGuiTableFlags_SizingFixedFit |  //
            ImGuiTableFlags_RowBg |           //
            ImGuiTableFlags_Hideable |        //
            ImGuiTableFlags_ScrollY |         //
            ImGuiTableFlags_NoHostExtendY;
        ImGui::PushID(this);
        if (ImGui::BeginTable("##OpsStats", 3, flags)) {
            ImGui::TableSetupScrollFreeze(0, 1);  // Make header always visible
            ImGui::TableSetupColumn("Ops", ImGuiTableColumnFlags_WidthFixed, -1, 0);
            ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed, -1, 0);
            ImGui::TableSetupColumn("Flops", ImGuiTableColumnFlags_WidthFixed, -1, 0);
            ImGui::TableHeadersRow();
            int32_t idx = 0;
            for (const auto& stat : m_result.stats.ops) {
                ImGui::PushID(idx);
                if (ImGui::TableNextColumn()) {  // Ops
                    if (ImGui::Selectable(stat.second.name.c_str(), m_selectedIndex > -1 && m_selectedIndex == idx, ImGuiSelectableFlags_SpanAllColumns)) {
                        if (m_selectedIndex != -1 && m_selectedIndex == idx) {
                            m_selectedIndex = -1;
                        } else {
                            m_selectedIndex = idx;
                        }
                    }
                }
                if (ImGui::TableNextColumn()) {  // Count
                    ImGui::Text("%u", static_cast<uint32_t>(stat.second.count));
                }

                if (ImGui::TableNextColumn()) {  // Flops
                    ImGui::Text("%.0f", stat.second.flops);
                }
                ImGui::PopID();

                ++idx;
            }
            ImGui::EndTable();
        }
        ImGui::PopID();
    }
}

ShaderOpt::Controller& Controller::getControllerRef() {
    return m_controller;
}
