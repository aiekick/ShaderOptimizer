#pragma once
#include <abstracts/ASingleton.hpp>

#include <imguipack.h>
#include <cstdint>
#include <memory>
#include <string>
#include <frontend/codeEditor.h>

class ProjectFile;
class TargetPane : public AbstractPane {
    IMPLEMENT_SHARED_SINGLETON(TargetPane)
private:
    CodeEditor m_codeEditor;

public:
    bool Init() override;
    void Unit() override;
    bool DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened, ImGuiContext* vContextPtr, void* vUserDatas) override;
    void setCode(const std::string& vCode) { m_codeEditor.SetCode(vCode, TextEditor::LanguageDefinition::Glsl()); }

public:
    TargetPane();                              // Prevent construction
    TargetPane(const TargetPane&) = default;  // Prevent construction by copying
    TargetPane& operator=(const TargetPane&) {
        return *this;
    };                       // Prevent assignment
    virtual ~TargetPane();  // Prevent unwanted destruction};
};
