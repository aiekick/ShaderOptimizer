#pragma once
#include <abstracts/ASingleton.hpp>

#include <imguipack.h>
#include <cstdint>
#include <memory>
#include <string>
#include <frontend/codeEditor.h>

class ProjectFile;
class SourcePane : public AbstractPane {
    IMPLEMENT_SHARED_SINGLETON(SourcePane)
private:
    CodeEditor m_codeEditor;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    void LoadShader(const std::string& vFilePathName);
    std::string getCode() { return m_codeEditor.GetCode(); }

public:
    SourcePane();                            // Prevent construction
    SourcePane(const SourcePane&) = delete;  // Prevent construction by copying
    SourcePane& operator=(const SourcePane&) {
        return *this;
    };                      // Prevent assignment
    virtual ~SourcePane();  // Prevent unwanted destruction};
};
