#include <backend/backend.h>
#include <imguipack/imguipack.h>
#include <frontend/frontend.h>
#include <project/projectFile.h>
#include <headers/customMessagingConfigHeader.h>
#include <ezlibs/ezFile.hpp>
#include <headers/ShaderOptimizerBuild.h>
#include <res/fontIcons.cpp>
#include <res/robotoMedium.cpp>
#include <res/FiraCode.cpp>
#include <panes/consolePane.h>

// messaging
#define MESSAGING_CODE_INFOS 0
#define MESSAGING_LABEL_INFOS "Info(s)"
#define MESSAGING_CODE_WARNINGS 1
#define MESSAGING_LABEL_WARNINGS "Warning(s)"
#define MESSAGING_CODE_ERRORS 2
#define MESSAGING_LABEL_ERRORS "Error(s)"
#define MESSAGING_CODE_DEBUG 3
#define MESSAGING_LABEL_DEBUG "Debug(s)"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define IMGUI_IMPL_API
#include <3rdparty/imgui_docking/backends/imgui_impl_opengl3.h>
#include <3rdparty/imgui_docking/backends/imgui_impl_glfw.h>

//////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

static void glfw_error_callback(int error, const char* description) {
    LogVarError("glfw error %i : %s", error, description);
}

static void glfw_window_close_callback(GLFWwindow* window) {
    glfwSetWindowShouldClose(window, GLFW_FALSE);  // block app closing
    Frontend::ref().actionWindowCloseApp();
}

static void glfw_drop_callback(GLFWwindow* /*window*/, int path_count, const char* paths[]) {
    std::vector<std::string> files;
    files.reserve(path_count);
    for (int idx = 0; idx < path_count; ++idx) {
        files.push_back(paths[idx]);
    }
    //Controller::Instance()->setInputFiles(files);
}

//////////////////////////////////////////////////////////////////////////////////
//// BACKEND /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bool Backend::init() {
    bool ret = true;
    ret &= m_initWindow();
    ret &= m_initImGui();
    ret &= m_initPanes();
    if (ret) {
        Messaging::Instance()->AddCategory(MESSAGING_CODE_INFOS, MESSAGING_LABEL_INFOS, BAR_LABEL_INFOS, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
        Messaging::Instance()->AddCategory(MESSAGING_CODE_WARNINGS, MESSAGING_LABEL_WARNINGS, BAR_LABEL_WARNINGS, ImVec4(0.8f, 0.8f, 0.0f, 1.0f));
        Messaging::Instance()->AddCategory(MESSAGING_CODE_ERRORS, MESSAGING_LABEL_ERRORS, ICON_FONT_SPACE_INVADERS, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
        Messaging::Instance()->AddCategory(MESSAGING_CODE_DEBUG, MESSAGING_LABEL_DEBUG, BAR_LABEL_ERROR, ImVec4(0.8f, 0.8f, 0.0f, 1.0f));
        Messaging::Instance()->SetLayoutManager(LayoutManager::Instance());
        ez::Log::Instance()->setStandardLogMessageFunctor([](const int& vType, const std::string& vMessage) {
            MessageData msg_datas;
            Messaging::Instance()->AddMessage(vMessage, vType, false, msg_datas, {});
        });
        LoadConfigFile("config.xml", "app");
    }
    return ret;
}

void Backend::loop() {
    int display_w, display_h;
    ImRect viewRect;
    while (!glfwWindowShouldClose(mp_window)) {
        ProjectFile::ref().newFrame();

        // maintain active, prevent user change via imgui dialog
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Disable Viewport

        glfwPollEvents();

        glfwGetFramebufferSize(mp_window, &display_w, &display_h);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        if (viewport) {
            viewRect.Min = viewport->WorkPos;
            viewRect.Max = viewRect.Min + viewport->WorkSize;
        } else {
            viewRect.Max = ImVec2((float)display_w, (float)display_h);
        }

        Frontend::ref().display(0, viewRect);

        ImGui::Render();

        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        auto* backup_current_context = glfwGetCurrentContext();

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste
        // this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
        glfwMakeContextCurrent(backup_current_context);

        glfwSwapBuffers(mp_window);
    }
}

void Backend::unit() {
    SaveConfigFile("config.xml", "app", "config");
    m_unitImGui();
    m_unitWindow();
}

void Backend::needToNewProject(const std::string& vFilePathName) {
    m_needToNewProject = true;
    m_projectFileToLoad = vFilePathName;
}

void Backend::needToLoadProject(const std::string& vFilePathName) {
    m_needToLoadProject = true;
    m_projectFileToLoad = vFilePathName;
}

void Backend::needToCloseProject() {
    m_needToCloseProject = true;
}

// actions to do after rendering
void Backend::m_postRenderingActions() {
    if (m_needToNewProject) {
        ProjectFile::ref().clear();
        ProjectFile::ref().newProject(m_projectFileToLoad);
        m_projectFileToLoad.clear();
        m_needToNewProject = false;
    }

    if (m_needToLoadProject) {
        if (!m_projectFileToLoad.empty()) {
            if (ProjectFile::ref().loadProjectAs(m_projectFileToLoad)) {
                setAppTitle(m_projectFileToLoad);
                ProjectFile::ref().setProjectChange(false);
            } else {
                LogVarError("Failed to load project %s", m_projectFileToLoad.c_str());
            }
        }

        m_projectFileToLoad.clear();
        m_needToLoadProject = false;
    }

    if (m_needToCloseProject) {
        ProjectFile::ref().clear();
        m_needToCloseProject = false;
    }
}

bool Backend::isNeedToCloseApp() {
    return m_needToCloseApp;
}

void Backend::needToCloseApp(const bool& vFlag) {
    m_needToCloseApp = vFlag;
}

void Backend::closeApp() {
    // will escape the main loop
    glfwSetWindowShouldClose(mp_window, 1);
}

void Backend::setAppTitle(const std::string& vFilePathName) {
    auto ps = ez::file::parsePathFileName(vFilePathName);
    if (ps.isOk) {
        char bufTitle[1024];
        snprintf(bufTitle, 1023, "%s Beta %s - shader : %s", ShaderOptimizer_Label, ShaderOptimizer_BuildId, vFilePathName.c_str());
        glfwSetWindowTitle(mp_window, bufTitle);
    } else {
        char bufTitle[1024];
        snprintf(bufTitle, 1023, "%s Beta %s", ShaderOptimizer_Label, ShaderOptimizer_BuildId);
        glfwSetWindowTitle(mp_window, bufTitle);
    }
}

void Backend::setConsoleVisibility(const bool& vFlag) {
    m_consoleVisiblity = vFlag;
    if (m_consoleVisiblity) {
#ifdef WIN32
        ShowWindow(GetConsoleWindow(), SW_SHOW);
#endif
    } else {
#ifdef WIN32
        ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
    }
}

void Backend::switchConsoleVisibility() {
    m_consoleVisiblity = !m_consoleVisiblity;
    setConsoleVisibility(m_consoleVisiblity);
}

bool Backend::getConsoleVisibility() {
    return m_consoleVisiblity;
}

ez::xml::Nodes Backend::getXmlNodes(const std::string& vUserDatas) {
    ez::xml::Node node;
    node.addChilds(Frontend::ref().getXmlNodes(vUserDatas));
    node.addChild("project").setContent(ProjectFile::ref().getProjectFilepathName());
    return node.getChildren();
}

bool Backend::setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) {
    const auto& strName = vNode.getName();
    const auto& strValue = vNode.getContent();
    //const auto& strParentName = vParent.getName();
    if (strName == "project") {
        needToLoadProject(strValue);
    }
    Frontend::ref().setFromXmlNodes(vNode, vParent, vUserDatas);
    return true;
}

bool Backend::m_initWindow() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return false;
    }

    // GL 3.0 + GLSL 130
    m_GlslVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create window with graphics context
    mp_window = glfwCreateWindow(1280, 720, ShaderOptimizer_Label, nullptr, nullptr);
    if (mp_window == nullptr) {
        LogVarError("Fail to create the window");
        return false;
    }
    glfwMakeContextCurrent(mp_window);
    glfwSwapInterval(1);  // Enable vsync

    if (gladLoadGL() == 0) {
        LogVarError("Failed to initialize OpenGL loader!");
        return false;
    }

    glfwSetDropCallback(mp_window, glfw_drop_callback);
    glfwSetWindowCloseCallback(mp_window, glfw_window_close_callback);

    return true;
}

void Backend::m_unitWindow() {
    glfwDestroyWindow(mp_window);
    glfwTerminate();
}

bool Backend::m_initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable Viewport
    // io.FontAllowUserScaling = true;                      // activate zoom feature with ctrl + mousewheel
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
    io.ConfigViewportsNoDecoration = false;  // toujours mettre une frame aux fenetres enfants
#endif

    float dpiScaleFactor = 1.0f;  // 100.0f / 16.0f;

    // fonts
    {
        {  // main font
            auto fontPtr = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_RM, 15.0f * dpiScaleFactor);
            if (fontPtr == nullptr) {
                assert(0);  // failed to load font
            } else {
                fontPtr->Scale = 1.0f / dpiScaleFactor;
            }
        }
        {  // icon font
            static const ImWchar icons_ranges[] = {ICON_MIN_FONT, ICON_MAX_FONT, 0};
            ImFontConfig icons_config;
            icons_config.MergeMode = true;
            icons_config.PixelSnapH = true;
            auto fontPtr = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_FONT, 15.0f * dpiScaleFactor, &icons_config, icons_ranges);
            if (fontPtr == nullptr) {
                assert(0);  // failed to load font
            } else {
                fontPtr->Scale = 1.0f / dpiScaleFactor;
            }
        }
        {  // programming font (Fira Code)
            auto fontPtr = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_FCR, 20.0f * dpiScaleFactor);
            if (fontPtr == nullptr) {
                assert(0);  // failed to load font
            } else {
                fontPtr->Scale = 1.0f / dpiScaleFactor;
            }
        }
    }

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    style.ScaleAllSizes(dpiScaleFactor);

    // Setup Platform/Renderer bindings
    if (ImGui_ImplGlfw_InitForOpenGL(mp_window, true) &&  //
        ImGui_ImplOpenGL3_Init(m_GlslVersion.c_str())) {
        // ui init
        if (Frontend::ref().init()) {
            return true;
        }
    }
    return false;
}

void Backend::m_unitImGui() {
    Frontend::ref().unit();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();
}


bool Backend::m_initPanes() {
    if (LayoutManager::Instance()->InitPanes()) {
        // a faire apres InitPanes() sinon ConsolePane::Instance()->paneFlag vaudra 0 et changeras apres InitPanes()
        Messaging::Instance()->sMessagePaneId = ConsolePane::ref()->GetFlag();
        return true;
    }
    return false;
}

void Backend::m_unitPanes() {
    LayoutManager::Instance()->UnitPanes();
}
