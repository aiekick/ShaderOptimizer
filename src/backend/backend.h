#pragma once
#include <abstracts/ASingleton.hpp>
#include <ezlibs/ezXmlConfig.hpp>

struct GLFWwindow;
class Backend : public ez::xml::Config {
    IMPLEMENT_SINGLETON(Backend)

private:
    GLFWwindow* mp_window{};
    bool m_needToCloseApp{};  // when app closing app is required
    bool m_needToNewProject{};
    bool m_needToLoadProject{};
    bool m_needToCloseProject{};
    std::string m_GlslVersion;
    std::string m_projectFileToLoad;
    bool m_consoleVisiblity{};

public:
    bool init();
    void loop();
    void unit();

    bool isNeedToCloseApp();
    void needToCloseApp(const bool& vFlag = true);
    void closeApp();

    void needToNewProject(const std::string& vFilePathName);
    void needToLoadProject(const std::string& vFilePathName);
    void needToCloseProject();

    void setAppTitle(const std::string& vFilePathName = {});

    void setConsoleVisibility(const bool& vFlag);
    void switchConsoleVisibility();
    bool getConsoleVisibility();

    ez::xml::Nodes getXmlNodes(const std::string& vUserDatas = "") override;
    bool setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) override;

private:
    void m_postRenderingActions();
    bool m_initWindow();
    void m_unitWindow();
    bool m_initImGui();
    void m_unitImGui();
    bool m_initPanes();
    void m_unitPanes();
};
