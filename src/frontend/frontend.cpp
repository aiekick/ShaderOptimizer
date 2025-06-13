#include <frontend/frontend.h>

#include <res/fontIcons.h>

#include <backend/backend.h>

#include <project/projectFile.h>

#include <panes/consolePane.h>
#include <panes/controlPane.h>
#include <panes/debugPane.h>
#include <panes/sourcePane.h>
#include <panes/targetPane.h>

bool Frontend::init() {
    m_buildThemes();

    LayoutManager::Instance()->Init(ICON_FONT_MONITOR_DASHBOARD " Layouts", "Default Layout");

    LayoutManager::Instance()->SetPaneDisposalRatio("LEFT", 0.25f);
    LayoutManager::Instance()->SetPaneDisposalRatio("RIGHT", 0.25f);
    LayoutManager::Instance()->SetPaneDisposalRatio("BOTTOM", 0.25f);

#ifdef _DEBUG
    LayoutManager::Instance()->AddPane(DebugPane::ref(), "Debug", "", "BOTTOM", 0.25f, false, false);
#endif
    LayoutManager::Instance()->AddPane(ConsolePane::ref(), "Console", "", "BOTTOM", 0.25f, false, false);
    LayoutManager::Instance()->AddPane(ControlPane::ref(), "Control", "", "LEFT", 0.25f, true, false);
    LayoutManager::Instance()->AddPane(SourcePane::ref(), "Source", "", "CENTRAL", 0.0f, true, false);
    LayoutManager::Instance()->AddPane(TargetPane::ref(), "Target", "", "RIGHT", 0.3f, true, false);

    // InitPanes is done in mInitPanes, because a specific order is needed

    return LayoutManager::Instance()->InitPanes();
}

void Frontend::unit() {
    LayoutManager::Instance()->UnitPanes();
}

void Frontend::iWantToCloseTheApp() {
    actionWindowCloseApp();
}

void Frontend::justDropFiles(int count, const char** paths) {
    /*assert(0);

    std::map<std::string, std::string> dicoFont;
    std::string prj;

    for (int i = 0; i < count; ++i) {
        // file
        auto f = std::string(paths[i]);

        // lower case
        auto fopt = f;
        for (auto& c : fopt)
            c = (char)std::tolower((int)c);

        // well known extention
        if (fopt.find(".ttf") != std::string::npos     // truetype (.ttf)
            || fopt.find(".otf") != std::string::npos  // opentype (.otf)
            //||	fopt.find(".ttc") != std::string::npos		// ttf/otf collection for futur (.ttc)
        ) {
            dicoFont[f] = f;
        }
        if (fopt.find(PROJECTEXT) != std::string::npos) {
            prj = f;
        }
    }

    // priority to project file
    if (!prj.empty()) {
        Backend::Instance()->NeedToLoadProject(prj);
    }*/
}

void Frontend::actionWindowCloseApp() {
    if (Backend::ref().isNeedToCloseApp()) {
        return;  // block next call to close app when running
    }

    m_actions.clear();
    m_actionOpenUnSavedDialogIfNeeded();
    m_actions.pushBackConditonalAction([]() {
        Backend::ref().closeApp();
        return true;
    });
}

void Frontend::display(const uint32_t& vCurrentFrame, const ImRect& vRect) {
    const auto contextptr = ImGui::GetCurrentContext();
    if (contextptr != nullptr) {
        const auto& io = ImGui::GetIO();
        m_displayRect = vRect;
        ImGui::CustomStyle::ResetCustomId();
        m_drawMainMenuBar();
        m_drawMainStatusBar();
        if (LayoutManager::Instance()->BeginDockSpace(ImGuiDockNodeFlags_PassthruCentralNode)) {
            /*if (Backend::Instance()->GetBackendDatasRef().canWeTuneGizmo) {
                const auto viewport = ImGui::GetMainViewport();
                ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);
                ImGuizmo::SetRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);
                ImRect rc(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);
                DrawOverlays(vCurrentFrame, rc, contextptr, {});
            }*/
            LayoutManager::Instance()->EndDockSpace();
        }

        if (LayoutManager::Instance()->DrawPanes(vCurrentFrame, contextptr, {})) {
            //ProjectFile::Instance()->SetProjectChange();
        }

        drawDialogsAndPopups(vCurrentFrame, m_displayRect, contextptr, {});

        ImGuiThemeHelper::Instance()->Draw();
        LayoutManager::Instance()->InitAfterFirstDisplay(io.DisplaySize);
    }
}

bool Frontend::drawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    bool res = false;
    return res;
}

bool Frontend::drawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    bool res = false;
    return res;
}

bool Frontend::drawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    m_actions.runImmediateActions();
    m_actions.executeFirstConditionalAction();
    LayoutManager::Instance()->DrawDialogsAndPopups(vCurrentFrame, vRect, vContextPtr, vUserDatas);
    if (m_showImGui) {
        ImGui::ShowDemoWindow(&m_showImGui);
    }
    if (m_showMetric) {
        ImGui::ShowMetricsWindow(&m_showMetric);
    }
    return false;
}

ez::xml::Nodes Frontend::getXmlNodes(const std::string& vUserDatas) {
    ez::xml::Node node;
    node.addChilds(ImGuiThemeHelper::Instance()->getXmlNodes("app"));
    node.addChilds(LayoutManager::Instance()->getXmlNodes("app"));
#ifdef USEPLACESFEATURE
    node.addChild("places").setContent(ImGuiFileDialog::Instance()->SerializePlaces());
#endif
    node.addChild("showaboutdialog").setContent(m_showAboutDialog);
    node.addChild("showimgui").setContent(m_showImGui);
    node.addChild("showmetric").setContent(m_showMetric);
    return node.getChildren();
}

bool Frontend::setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) {
    const auto& strName = vNode.getName();
    const auto& strValue = vNode.getContent();
    const auto& strParentName = vParent.getName();

    if (strName == "places") {
#ifdef USEPLACESFEATURE
        ImGuiFileDialog::Instance()->DeserializePlaces(strValue);
#endif
    } else if (strName == "showaboutdialog") {
        m_showAboutDialog = ez::ivariant(strValue).GetB();
    } else if (strName == "showimgui") {
        m_showImGui = ez::ivariant(strValue).GetB();
    } else if (strName == "showmetric") {
        m_showMetric = ez::ivariant(strValue).GetB();
    }

    ImGuiThemeHelper::Instance()->setFromXmlNodes(vNode, vParent, "app");
    LayoutManager::Instance()->setFromXmlNodes(vNode, vParent, "app");

    return true;
}

void Frontend::m_openAboutDialog() {
    m_showAboutDialog = true;
}


///////////////////////////////////////////////////////
//// SAVE DIALOG WHEN UN SAVED CHANGES ////////////////
///////////////////////////////////////////////////////

void Frontend::m_openUnSavedDialog() {
    // force close dialog if any dialog is opened
    ImGuiFileDialog::Instance()->Close();
    m_saveDialogIfRequired = true;
}
void Frontend::m_closeUnSavedDialog() {
    m_saveDialogIfRequired = false;
}

bool Frontend::m_showUnSavedDialog() {
    bool res = false;

    if (m_saveDialogIfRequired) {
        if (ProjectFile::ref().isProjectLoaded()) {
            if (ProjectFile::ref().isThereAnyProjectChanges()) {
                /*
                Unsaved dialog behavior :
                -	save :
                    -	insert action : save project
                -	save as :
                    -	insert action : save as project
                -	continue without saving :
                    -	quit unsaved dialog
                -	cancel :
                    -	clear actions
                */

                ImGui::CloseCurrentPopup();
                const char* label = "Save before closing ?";
                ImGui::OpenPopup(label);
                ImGui::SetNextWindowPos(m_displayRect.GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                if (ImGui::BeginPopupModal(label, (bool*)0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking)) {
                    const auto& width = ImGui::CalcTextSize("Continue without saving").x + ImGui::GetStyle().ItemInnerSpacing.x;

                    if (ImGui::ContrastedButton("Save", nullptr, nullptr, width * 0.5f)) {
                        res = m_actionUnSavedDialogSaveProject();
                    }
                    ImGui::SameLine();
                    if (ImGui::ContrastedButton("Save As", nullptr, nullptr, width * 0.5f)) {
                        m_actionUnSavedDialogSaveAsProject();
                    }

                    if (ImGui::ContrastedButton("Continue without saving")) {
                        res = true;  // quit the action
                    }

                    if (ImGui::ContrastedButton("Cancel", nullptr, nullptr, width + ImGui::GetStyle().FramePadding.x)) {
                        m_actionCancel();
                    }

                    ImGui::EndPopup();
                }
            }
        }

        return res;  // quit if true, else continue on the next frame
    }

    return true;  // quit the action
}

///////////////////////////////////////////////////////
//// ACTIONS //////////////////////////////////////////
///////////////////////////////////////////////////////

void Frontend::m_actionMenuNewProject() {
    m_actions.clear();
    m_actions.pushBackConditonalAction([this]() {

        return true;
    });
}

void Frontend::m_actionMenuOpenProject() {
    m_actions.clear();
    m_actionOpenUnSavedDialogIfNeeded();
    m_actions.pushBackConditonalAction([this]() {
        IGFD::FileDialogConfig config;
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("OpenProjectDlg", "Open Project File", ".*", config);
        return true;
    });
    m_actions.pushBackConditonalAction([this]() { return m_displayOpenProjectDialog(); });
}

void Frontend::m_actionMenuSaveProject() {
    m_actions.clear();
    m_actions.pushBackConditonalAction([this]() {
        return true;
    });
    m_actions.pushBackConditonalAction([this]() { return m_displaySaveProjectDialog(); });
}

void Frontend::m_actionMenuSaveAsProject() {
    m_actions.clear();
    m_actions.pushBackConditonalAction([this]() {
        return true;
        return true;
    });
    m_actions.pushBackConditonalAction([this]() { return m_displaySaveProjectDialog(); });
}

void Frontend::m_actionMenuCloseProject() {
    m_actions.clear();
    m_actionOpenUnSavedDialogIfNeeded();
    m_actions.pushBackConditonalAction([]() {
        return true;
    });
}

bool Frontend::m_actionUnSavedDialogSaveProject() {
    /*bool res = Backend::ref().saveProject();
    if (!res) {
        m_actions.pushFrontImmediateAction([this]() { return m_displaySaveProjectDialog(); });
        m_actions.pushFrontImmediateAction([this]() {
            m_closeUnSavedDialog();
            IGFD::FileDialogConfig config;
            config.countSelectionMax = 1;
            config.flags = ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal;
            config.path = ".";
            ImGuiFileDialog::Instance()->OpenDialog("SaveProjectDlg", "Save Project File", PROJECTEXT, config);
            return true;
        });
    }
    return res;*/
    return false;
}

void Frontend::m_actionUnSavedDialogSaveAsProject() {
    /*m_actions.pushFrontImmediateAction([this]() { return DisplaySaveProjectDialog(); });
    m_actions.pushFrontImmediateAction([this]() {
        m_closeUnSavedDialog();
        IGFD::FileDialogConfig config;
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlagsConfirmOverwrite | ImGuiFileDialogFlagsModal;
        config.path = ".";
        ImGuiFileDialog::Instance()->OpenDialog("SaveProjectDlg", "Save Project File", PROJECTEXT, config);
        return true;
    });*/
}

void Frontend::m_actionUnSavedDialogCancel() {
    m_actionCancel();
}

void Frontend::m_actionOpenUnSavedDialogIfNeeded() {
    /*if (ProjectFile::Instance()->IsProjectLoaded() && ProjectFile::Instance()->IsThereAnyProjectChanges()) {
        m_openUnSavedDialog();
        m_actions.pushBackConditonalAction([this]() { return m_showUnSavedDialog(); });
    }*/
}

void Frontend::m_actionCancel() {
    /*
    -	cancel :
        -	clear actions
    */
    m_closeUnSavedDialog();
    m_actions.clear();
    //Backend::ref().NeedToCloseApp(false);
}

bool Frontend::m_displayNewProjectDialog() {
    // need to return false to continue to be displayed next frame

    ImVec2 max = m_displayRect.GetSize();
    ImVec2 min = max * 0.5f;

    if (ImGuiFileDialog::Instance()->Display("NewProjectDlg", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            m_closeUnSavedDialog();
            auto file = ImGuiFileDialog::Instance()->GetFilePathName();
            SourcePane::ref()->LoadShader(file);
        } else {               // cancel
            m_actionCancel();  // we interrupts all actions
        }

        ImGuiFileDialog::Instance()->Close();

        return true;
    }

    return false;
}

bool Frontend::m_displayOpenProjectDialog() {
    // need to return false to continue to be displayed next frame

    ImVec2 max = m_displayRect.GetSize();
    ImVec2 min = max * 0.5f;

    if (ImGuiFileDialog::Instance()->Display("OpenProjectDlg", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            m_closeUnSavedDialog();
            auto file = ImGuiFileDialog::Instance()->GetFilePathName();
            SourcePane::ref()->LoadShader(file);
        } else {               // cancel
            m_actionCancel();  // we interrupts all actions
        }

        ImGuiFileDialog::Instance()->Close();

        return true;
    }

    return false;
}

bool Frontend::m_displaySaveProjectDialog() {
    // need to return false to continue to be displayed next frame

    ImVec2 max = m_displayRect.GetSize();
    ImVec2 min = max * 0.5f;

    if (ImGuiFileDialog::Instance()->Display("SaveProjectDlg", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            m_closeUnSavedDialog();
            //Backend::Instance()->SaveAsProject(ImGuiFileDialog::Instance()->GetFilePathName());
        } else {               // cancel
            m_actionCancel();  // we interrupts all actions
        }

        ImGuiFileDialog::Instance()->Close();

        return true;
    }

    return false;
}

void Frontend::m_drawMainMenuBar() {
    static float scontrollermenusize = 0.0f;
    static float stranslationmenusize = 0.0f;
    if (ImGui::BeginMainMenuBar()) {
        float fullwidth = ImGui::GetContentRegionAvail().x;
        if (ImGui::BeginMenu(ICON_FONT_ARCHIVE " File")) {
            if (ImGui::MenuItem(" New")) {
                m_actionMenuNewProject();
            }

            if (ImGui::MenuItem(" Open")) {
                m_actionMenuOpenProject();
            }

            if (ProjectFile::ref().isProjectLoaded()) {
                ImGui::Separator();

                if (ImGui::MenuItem(" Save")) {
                    m_actionMenuSaveProject();
                }

                if (ImGui::MenuItem(" Save As")) {
                    m_actionMenuSaveAsProject();
                }

                ImGui::Separator();

                if (ImGui::MenuItem(" Clear")) {
                    m_actionMenuCloseProject();
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem(" About")) {
                m_openAboutDialog();
            }

            ImGui::EndMenu();
        }
        LayoutManager::Instance()->DisplayMenu(ImGui::GetIO().DisplaySize);
        if (ImGui::BeginMenu(ICON_FONT_SETTINGS " Tools")) {
            if (ImGui::BeginMenu("Styles")) {
                ImGuiThemeHelper::Instance()->DrawMenu();

#ifdef _DEBUG
                ImGui::Separator();
                ImGui::MenuItem("Show ImGui", "", &m_showImGui);
                ImGui::MenuItem("Show ImGui Metric/Debug", "", &m_showMetric);
#endif

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }
        if (ProjectFile::ref().isThereAnyProjectChanges()) {
            ImGui::Spacing(20.0f);
            if (ImGui::MenuItem(" Save")) {
                m_actionMenuSaveProject();
            }
        }

        ImGui::EndMainMenuBar();
    }
}

void Frontend::m_drawMainStatusBar() {
    if (ImGui::BeginMainStatusBar()) {
        Messaging::Instance()->DrawStatusBar();

#ifdef _DEBUG
        const auto& io = ImGui::GetIO();
        const auto fps = ez::str::toStr("%.1f ms/frame (%.1f fps)", 1000.0f / io.Framerate, io.Framerate);
        const auto size = ImGui::CalcTextSize(fps.c_str());
        ImGui::Spacing(ImGui::GetContentRegionAvail().x - size.x - ImGui::GetStyle().FramePadding.x * 2.0f);
        ImGui::Text("%s", fps.c_str());
#endif

        ImGui::EndMainStatusBar();
    }
}