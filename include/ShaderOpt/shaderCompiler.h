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

#include <ShaderOpt/defs.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Include/intermediate.h>

#include <unordered_map>
#include <functional>
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <list>
#include <map>
#include <set>

/*
todo : to Refactor and Convert for use of Vulkan.hpp
*/

namespace ShaderOpt {

typedef std::string ShaderEntryPoint;

class ShaderCompiler {
public:
    typedef std::function<void(std::string, std::string, std::string)> ShaderMessagingFunction;
    typedef std::function<void(glslang::TIntermediate*)> TraverserFunction;
    typedef std::unordered_map<EShLanguage, std::vector<std::string>> ShaderInfos;

private:
    ShaderInfos m_errors;
    ShaderInfos m_warnings;

public:
    SpirvCode CompileGLSLString(
        const std::string& vCode,
        const EShLanguage& vShaderType,
        const ShaderEntryPoint& vEntryPoint = "main",
        ShaderMessagingFunction vMessagingFunction = nullptr,
        TraverserFunction vTraverser = nullptr,
        std::string* vShaderCode = nullptr,
        std::unordered_map<std::string, bool>* vUsedUniforms = nullptr);

private:
    std::unordered_map<std::string, bool> m_collectUniformInfosFromIR(const glslang::TIntermediate& intermediate);
    double m_computeFlops(const glslang::TIntermediate& intermediate);

private:
    std::string m_getSuffix(const std::string& name);
    EShLanguage m_getShaderStage(const std::string& stage);
    std::string m_getShaderStageString(const EShLanguage& stage);
    std::string m_getFullShaderStageString(const EShLanguage& stage);
};

}  // namespace ShaderOpt
