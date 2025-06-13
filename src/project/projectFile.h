#pragma once

#include <abstracts/ASingleton.hpp>
#include <ezlibs/ezXmlConfig.hpp>
#include <string>

class ProjectInterface {
public:
    virtual bool isProjectLoaded() const = 0;
    virtual bool isProjectNeverSaved() const = 0;
    virtual bool isThereAnyProjectChanges() const = 0;
    virtual void setProjectChange(bool vChange = true) = 0;
    virtual bool wasJustSaved() = 0;
};

class ProjectFile : public ProjectInterface, public ez::xml::Config {
    IMPLEMENT_SINGLETON(ProjectFile)
private:
    // to save
    std::string m_projectFilePathName;
    std::string m_projectFileName;
    std::string m_projectFilePath;
    // dont save
    bool m_isLoaded = false;
    bool m_neverSaved = false;
    bool m_isThereAnyChanges = false;
    bool m_wasJustSaved = false;
    size_t m_wasJustSavedFrameCounter = 0U;  // the state of m_WasJustSaved will be keeped during two frames

public:
    ProjectFile();
    explicit ProjectFile(const std::string& vFilePathName);
    virtual ~ProjectFile();

    void clear();
    void clearDatas();
    void newProject();
    void newProject(const std::string& vFilePathName);
    bool loadProject();
    bool loadProjectAs(const std::string vFilePathName);  // ils wanted to not pass the adress for re open case
    bool saveProject();
    bool saveProjectAs(const std::string& vFilePathName);

    bool isProjectLoaded() const override;
    bool isProjectNeverSaved() const override;
    bool isThereAnyProjectChanges() const override;
    void setProjectChange(bool vChange = true) override;
    bool wasJustSaved() override;

    void newFrame();

    std::string getProjectFilepathName() const;
    ez::xml::Nodes getXmlNodes(const std::string& vUserDatas = "") override;
    bool setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) override;
};
