#pragma once
#include <abstracts/ASingleton.hpp>
#include <ezlibs/ezXmlConfig.hpp>
#include <imguipack/imguipack.h>
#include <ezlibs/ezActions.hpp>

class Frontend : public ez::xml::Config {
    IMPLEMENT_SINGLETON(Frontend)

private:
    bool m_showImGui = false;
    bool m_showMetric = false;
    bool m_showAboutDialog = false;          // show about dlg
    bool m_saveDialogIfRequired = false;     // open save options dialog (save / save as / continue without saving / cancel)
    bool m_saveDialogActionWasDone = false;  // if action was done by save options dialog
    ez::Actions m_actions;
    ImRect m_displayRect{ImVec2(0, 0), ImVec2(1280, 720)};

public:
    bool init();
    void unit();

public:
    void iWantToCloseTheApp();
    void justDropFiles(int count, const char** paths);
    void actionWindowCloseApp();
    void display(const uint32_t& vCurrentFrame, const ImRect& vRect);
    bool drawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas);
    bool drawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas);
    bool drawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas);
    ez::xml::Nodes getXmlNodes(const std::string& vUserDatas = "") override;
    bool setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) override;

private:
    void m_openAboutDialog();
    // trigger actions
    void m_openUnSavedDialog();   // show a dialog because the project file is not saved
    void m_closeUnSavedDialog();  // show a dialog because the project file is not saved
    bool m_showUnSavedDialog();   // show a dilaog because the project file is not saved
    // actions via menu
    void m_actionMenuNewProject();
    void m_actionMenuOpenProject();
    void m_actionMenuSaveProject();
    void m_actionMenuSaveAsProject();
    void m_actionMenuCloseProject();
    // via the unsaved dialog
    bool m_actionUnSavedDialogSaveProject();
    void m_actionUnSavedDialogSaveAsProject();
    void m_actionUnSavedDialogCancel();
    // others
    void m_actionOpenUnSavedDialogIfNeeded();
    void m_actionCancel();
    // dialog funcs to be in actions
    bool m_displayNewProjectDialog();
    bool m_displayOpenProjectDialog();
    bool m_displaySaveProjectDialog();
    // misc
    bool m_buildThemes();
    void m_drawMainMenuBar();
    void m_drawMainStatusBar();
};