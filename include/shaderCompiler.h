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

#pragma once
#pragma warning(disable : 4251)

#include <glslang/Public/ShaderLang.h>
#include <glslang/Include/intermediate.h>

#include <unordered_map>
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <array>

/*
todo : to Refactor and Convert for use of Vulkan.hpp
*/

typedef std::string ShaderEntryPoint;

class ShaderCompiler {
public:
    typedef std::function<void(std::string, std::string, std::string)> ShaderMessagingFunction;
    typedef std::function<void(glslang::TIntermediate*)> TraverserFunction;

private:  // errors
    std::unordered_map<EShLanguage, std::vector<std::string>> m_errors;
    std::unordered_map<EShLanguage, std::vector<std::string>> m_warnings;

public:
    const std::vector<unsigned int> CompileGLSLFile(
        const std::string& filename,
        const ShaderEntryPoint& vEntryPoint = "main",
        ShaderMessagingFunction vMessagingFunction = nullptr,
        std::string* vShaderCode = nullptr,
        std::unordered_map<std::string, bool>* vUsedUniforms = nullptr);
    const std::vector<unsigned int> CompileGLSLString(
        const std::string& vCode,
        const std::string& vShaderSuffix,
        const std::string& vOriginalFileName,
        const ShaderEntryPoint& vEntryPoint = "main",
        ShaderMessagingFunction vMessagingFunction = nullptr,
        std::string* vShaderCode = nullptr,
        std::unordered_map<std::string, bool>* vUsedUniforms = nullptr);
    void ParseGLSLString(
        const std::string& vCode,
        const std::string& vShaderSuffix,
        const std::string& vOriginalFileName,
        const ShaderEntryPoint& vEntryPoint,
        ShaderMessagingFunction vMessagingFunction,
        TraverserFunction vTraverser);
    std::unordered_map<std::string, bool> CollectUniformInfosFromIR(const glslang::TIntermediate& intermediate);

private:
    std::string m_getSuffix(const std::string& name);
    EShLanguage m_getShaderStage(const std::string& stage);
    std::string m_getShaderStageString(const EShLanguage& stage);
    std::string m_getFullShaderStageString(const EShLanguage& stage);
};

