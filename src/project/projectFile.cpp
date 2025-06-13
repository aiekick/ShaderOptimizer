/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "projectFile.h"

#include <ezlibs/ezLog.hpp>
#include <ezlibs/ezFile.hpp>
#include <imguipack/imguipack.h>

ProjectFile::ProjectFile() = default;

ProjectFile::ProjectFile(const std::string& vFilePathName) {
    m_projectFilePathName = ez::file::simplifyFilePath(vFilePathName);
    auto ps = ez::file::parsePathFileName(m_projectFilePathName);
    if (ps.isOk) {
        m_projectFileName = ps.name;
        m_projectFilePath = ps.path;
    }
}

ProjectFile::~ProjectFile() = default;

void ProjectFile::clear() {
    m_projectFilePathName.clear();
    m_projectFileName.clear();
    m_projectFilePath.clear();
    m_isLoaded = false;
    m_isThereAnyChanges = false;
    Messaging::Instance()->Clear();
}

void ProjectFile::clearDatas() {

}

void ProjectFile::newProject() {
    clear();
    clearDatas();
    m_isLoaded = true;
    m_neverSaved = true;
    setProjectChange(true);
}

void ProjectFile::newProject(const std::string& vFilePathName) {
    clear();
    clearDatas();
    m_projectFilePathName = ez::file::simplifyFilePath(vFilePathName);
    auto ps = ez::file::parsePathFileName(m_projectFilePathName);
    if (ps.isOk) {
        m_projectFileName = ps.name;
        m_projectFilePath = ps.path;
    }
    m_isLoaded = true;
    setProjectChange(false);
}

bool ProjectFile::loadProject() {
    return loadProjectAs(m_projectFilePathName);
}

// ils wanted to not pass the adress for re open case
// elwse, the clear will set vFilePathName to empty because with re open, target m_projectFilePathName
bool ProjectFile::loadProjectAs(const std::string vFilePathName) {
    if (!vFilePathName.empty()) {
        clear();
        std::string filePathName = ez::file::simplifyFilePath(vFilePathName);
        if (LoadConfigFile(filePathName, "project")) {
            m_projectFilePathName = ez::file::simplifyFilePath(vFilePathName);
            auto ps = ez::file::parsePathFileName(m_projectFilePathName);
            if (ps.isOk) {
                m_projectFileName = ps.name;
                m_projectFilePath = ps.path;
            }
            m_isLoaded = true;
            setProjectChange(false);
        } else {
            clear();
            LogVarError("Error : the project file %s cant be loaded", filePathName.c_str());
        }
    }    
    return m_isLoaded;
}

bool ProjectFile::saveProject() {
    if (m_neverSaved) {
        return false;
    }
    SaveConfigFile(m_projectFilePathName, "project", "project");
    setProjectChange(false);
    return false;
}

bool ProjectFile::saveProjectAs(const std::string& vFilePathName) {
    std::string filePathName = ez::file::simplifyFilePath(vFilePathName);
    auto ps = ez::file::parsePathFileName(filePathName);
    if (ps.isOk) {
        m_projectFilePathName = filePathName;
        m_projectFilePath = ps.path;
        m_neverSaved = false;
        return saveProject();
    }
    return false;
}

bool ProjectFile::isProjectLoaded() const {
    return m_isLoaded;
}

bool ProjectFile::isProjectNeverSaved() const {
    return m_neverSaved;
}

bool ProjectFile::isThereAnyProjectChanges() const {
    return m_isThereAnyChanges;
}

void ProjectFile::setProjectChange(bool vChange) {
    m_isThereAnyChanges = vChange;
    m_wasJustSaved = true;
    m_wasJustSavedFrameCounter = 2U;
}

void ProjectFile::newFrame() {
    if (m_wasJustSavedFrameCounter) {
        --m_wasJustSavedFrameCounter;
    } else {
        m_wasJustSaved = false;
    }
}

bool ProjectFile::wasJustSaved() {
    return m_wasJustSaved;
}

std::string ProjectFile::getProjectFilepathName() const {
    return m_projectFilePathName;
}

ez::xml::Nodes ProjectFile::getXmlNodes(const std::string& vUserDatas) {
    ez::xml::Node node;
    return node.getChildren();
}

bool ProjectFile::setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) {
    return true;
}