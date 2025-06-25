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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <core/shaderCompiler.h>

#include <core/resLimits.h>
#include <core/uniformsIRLocator.h>
#include <ezlibs/ezLog.hpp>

#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Include/ShHandle.h>
#include <glslang/OSDependent/osinclude.h>
#include <StandAlone/DirStackFileIncluder.h>

#include <cstdio>     // printf, fprintf
#include <cstdlib>    // abort
#include <iostream>   // std::cout
#include <stdexcept>  // std::exception
#include <algorithm>  // std::min, std::max
#include <fstream>    // std::ifstream
#include <chrono>     // timer

#define VERBOSE_DEBUG

// TODO: Multithread, manage SpirV that doesn't need recompiling (only recompile when dirty)
const std::vector<unsigned int> ShaderCompiler::CompileGLSLFile(const std::string& filename,
    const ShaderEntryPoint& vEntryPoint,
    ShaderMessagingFunction vMessagingFunction,
    std::string* vShaderCode,
    std::unordered_map<std::string, bool>* vUsedUniforms) {

    std::vector<unsigned int> SpirV;

    // Load GLSL into a string
    std::ifstream file(filename);

    if (!file.is_open()) {
        LogVarError("Debug : Failed to load shader %s", filename.c_str());
        return SpirV;
    }

    std::string InputGLSL((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    file.close();

    if (!InputGLSL.empty()) {
        if (vShaderCode)
            *vShaderCode = InputGLSL;

        return CompileGLSLString(InputGLSL, m_getSuffix(filename), filename, vEntryPoint, vMessagingFunction, vShaderCode, vUsedUniforms);
    }

    return SpirV;
}

const std::vector<unsigned int> ShaderCompiler::CompileGLSLString(const std::string& vCode,
    const std::string& vShaderSuffix,
    const std::string& vOriginalFileName,
    const ShaderEntryPoint& vEntryPoint,
    ShaderMessagingFunction vMessagingFunction,
    std::string* vShaderCode,
    std::unordered_map<std::string, bool>* vUsedUniforms) {

    m_errors.clear();
    m_warnings.clear();

    std::vector<unsigned int> SpirV;

    std::string InputGLSL = vCode;

    EShLanguage shaderType = m_getShaderStage(vShaderSuffix);

    if (!InputGLSL.empty() && shaderType != EShLanguage::EShLangCount) {
        glslang::TShader Shader(shaderType);

        if (vShaderCode)
            *vShaderCode = InputGLSL;

        // Set up Vulkan/SpirV Environment
        int ClientInputSemanticsVersion = 100;  // maps to, say, #define VULKAN 100
        // glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;  // would map to, say, Vulkan 1.0
        // glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;    // maps to, say, SPIR-V 1.0

        // RTX SUpport
        glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_2;  // would map to, say, Vulkan 1.0
        glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_4;         // maps to, say, SPIR-V 1.0

        Shader.setEnvInput(glslang::EShSourceGlsl, shaderType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
        Shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
        Shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

        const char* InputCString = InputGLSL.c_str();

        Shader.setStrings(&InputCString, 1);

        EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

#ifdef _DEBUG
        messages = (EShMessages)(messages | EShMsgDebugInfo);
#endif

        const int DefaultVersion = 110;  // 110 for desktop, 100 for es

        DirStackFileIncluder Includer;

        std::string PreprocessedGLSL;

        std::string shaderTypeString = m_getFullShaderStageString(shaderType);

        if (!Shader.preprocess(&glslang::DefaultTBuiltInResource, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL, Includer)) {
            LogVarError("Debug Preprocessing : GLSL stage %s Preprocessing Failed for : %s", vShaderSuffix.c_str(), vOriginalFileName.c_str());

            std::string log = Shader.getInfoLog();
            if (!log.empty()) {
                LogVarDebugInfo("Debug Preprocessing Errors : %s", log.c_str());
                m_errors[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Preprocessing Errors", shaderTypeString, log);
                }
            }
#ifdef VERBOSE_DEBUG
            log = Shader.getInfoDebugLog();
            if (!log.empty()) {
                LogVarError("Debug Preprocessing Errors : %s", log.c_str());
                m_errors[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Preprocessing Errors", shaderTypeString, log);
                }
            }
#endif
            m_warnings.clear();

            return SpirV;
        } else {
            m_errors.clear();
            std::string log = Shader.getInfoLog();
            if (!log.empty()) {
                LogVarWarning("Debug Preprocessing Warnings : %s", log.c_str());
                m_warnings[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Preprocessing Warnings", shaderTypeString, log);
                }
            }
#ifdef VERBOSE_DEBUG
            log = Shader.getInfoDebugLog();
            if (!log.empty()) {
                LogVarWarning("Debug Preprocessing Warnings : %s", log.c_str());
                m_warnings[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Preprocessing Warnings", shaderTypeString, log);
                }
            }
#endif
        }

        const char* PreprocessedCStr = PreprocessedGLSL.c_str();
        Shader.setStrings(&PreprocessedCStr, 1);

        auto entry = vEntryPoint;
        if (entry.empty())
            entry = "main";
        Shader.setEntryPoint(entry.c_str());
        Shader.setSourceEntryPoint("main");

        if (!Shader.parse(&glslang::DefaultTBuiltInResource, 100, false, messages)) {
            LogVarError(
                "Debug Parse (%s) : GLSL stage %s Parse Failed for stage : %s", entry.c_str(), vShaderSuffix.c_str(), vOriginalFileName.c_str());

            std::string log = Shader.getInfoLog();
            if (!log.empty()) {
                LogVarError("Debug Parse Errors (%s) : %s", entry.c_str(), log.c_str());
                m_errors[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Parse Parse Errors", shaderTypeString, log);
                }
            }
#ifdef VERBOSE_DEBUG
            log = Shader.getInfoDebugLog();
            if (!log.empty()) {
                LogVarError("Debu Parse Errors (%s) : %s", entry.c_str(), log.c_str());
                m_errors[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Parse Errors", shaderTypeString, log);
                }
            }
#endif
            m_warnings.clear();

            return SpirV;
        } else {
            m_errors.clear();
            std::string log = Shader.getInfoLog();
            if (!log.empty()) {
                LogVarWarning("Debug Parse Warnings (%s) : %s", entry.c_str(), log.c_str());
                m_warnings[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Parse Warnings", shaderTypeString, log);
                }
            }
#ifdef VERBOSE_DEBUG
            log = Shader.getInfoDebugLog();
            if (!log.empty()) {
                LogVarWarning("Debug Parse Warnings (%s) : %s", entry.c_str(), log.c_str());
                m_warnings[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Parse Warnings", shaderTypeString, log);
                }
            }
#endif
        }

        glslang::TProgram Program;
        Program.addShader(&Shader);

        if (!Program.link(messages)) {
            LogVarError(
                "Debug Linking (%s) : GLSL stage %s Linking Failed for : %s", entry.c_str(), vShaderSuffix.c_str(), vOriginalFileName.c_str());

            std::string log = Program.getInfoLog();
            if (!log.empty()) {
                LogVarDebugInfo("Debug Linking Errors (%s) : %s", entry.c_str(), log.c_str());
                m_errors[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Linking Errors", shaderTypeString, log);
                }
            }
#ifdef VERBOSE_DEBUG
            log = Program.getInfoDebugLog();
            if (!log.empty()) {
                LogVarError("Debug Linking Errors (%s) : %s", entry.c_str(), log.c_str());
                m_errors[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Linking Errors", shaderTypeString, log);
                }
            }
#endif
            m_warnings.clear();

            return SpirV;
        } else {
            m_errors.clear();
            std::string log = Shader.getInfoLog();
            if (!log.empty()) {
                LogVarWarning("Debug Linking Warnings (%s) : %s", entry.c_str(), log.c_str());
                m_warnings[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Warnings", shaderTypeString, log);
                }
            }
#ifdef VERBOSE_DEBUG
            log = Shader.getInfoDebugLog();
            if (!log.empty()) {
                LogVarWarning("Debug Linking Warnings (%s) : %s", entry.c_str(), log.c_str());
                m_warnings[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Linking Warnings", shaderTypeString, log);
                }
            }
#endif
        }

        if (vUsedUniforms) {
            auto usedUniforms = CollectUniformInfosFromIR(*Shader.getIntermediate());
            for (auto u : usedUniforms) {
                (*vUsedUniforms)[u.first] |= u.second;
            }
        }

        spv::SpvBuildLogger logger;
        glslang::SpvOptions spvOptions;
        spvOptions.optimizeSize = true;
#ifdef _DEBUG
        spvOptions.generateDebugInfo = true;
#else
        spvOptions.stripDebugInfo = true;
#endif

        glslang::GlslangToSpv(*Program.getIntermediate(shaderType), SpirV, &logger, &spvOptions);

        if (logger.getAllMessages().length() > 0) {
            std::string allmsgs = logger.getAllMessages();
            std::cout << allmsgs << std::endl;
        }
    }

    if (SpirV.empty()) {
        LogVarError("Debug : Shader stage %s Spirv generation of %s : NOK !", vShaderSuffix.c_str(), vOriginalFileName.c_str());
    } 

    return SpirV;
}

void ShaderCompiler::ParseGLSLString(const std::string& vCode,
    const std::string& vShaderSuffix,
    const std::string& vOriginalFileName,
    const ShaderEntryPoint& vEntryPoint,
    ShaderMessagingFunction vMessagingFunction,
    TraverserFunction vTraverser) {
    std::string InputGLSL = vCode;

    EShLanguage shaderType = m_getShaderStage(vShaderSuffix);

    if (!InputGLSL.empty() && shaderType != EShLanguage::EShLangCount) {
        const char* InputCString = InputGLSL.c_str();

        glslang::TShader Shader(shaderType);
        Shader.setStrings(&InputCString, 1);

        // Set up Vulkan/SpirV Environment
        int ClientInputSemanticsVersion = 100;                                               // maps to, say, #define VULKAN 100
        glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;  // would map to, say, Vulkan 1.0
        glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;         // maps to, say, SPIR-V 1.0

        Shader.setEnvInput(glslang::EShSourceGlsl, shaderType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
        Shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
        Shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);
        auto entry = vEntryPoint;
        if (entry.empty())
            entry = "main";
        Shader.setEntryPoint(entry.c_str());
        Shader.setSourceEntryPoint("main");

        EShMessages messages = (EShMessages)(EShMsgAST);

        const int DefaultVersion = 110;  // 110 for desktop, 100 for es

        DirStackFileIncluder Includer;

        std::string PreprocessedGLSL;

        std::string shaderTypeString = m_getFullShaderStageString(shaderType);

        if (!Shader.preprocess(&glslang::DefaultTBuiltInResource, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL, Includer)) {
            LogVarError("Debug : GLSL stage %s Preprocessing Failed for : %s", vShaderSuffix.c_str(), vOriginalFileName.c_str());
            LogVarError("Debug : %s", Shader.getInfoLog());
            LogVarError("Debug : %s", Shader.getInfoDebugLog());

            std::string log = Shader.getInfoLog();
            if (!log.empty()) {
                m_errors[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Preprocessing Errors", shaderTypeString, log);
                }
            }
#ifdef VERBOSE_DEBUG
            log = Shader.getInfoDebugLog();
            if (!log.empty()) {
                m_errors[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Preprocessing Errors", shaderTypeString, log);
                }
            }
#endif
            m_warnings.clear();

            // LogVarDebugInfo("Debug : ==========================================");
        } else {
            m_errors.clear();
            std::string log = Shader.getInfoLog();
            if (!log.empty()) {
                m_warnings[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Preprocessing Warnings", shaderTypeString, log);
                }
            }
#ifdef VERBOSE_DEBUG
            log = Shader.getInfoDebugLog();
            if (!log.empty()) {
                m_warnings[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Preprocessing Warnings", shaderTypeString, log);
                }
            }
#endif
        }

        const char* PreprocessedCStr = PreprocessedGLSL.c_str();
        Shader.setStrings(&PreprocessedCStr, 1);

        if (!Shader.parse(&glslang::DefaultTBuiltInResource, 100, false, messages)) {
            LogVarError("Debug : GLSL stage %s Parse Failed for stage : %s", vShaderSuffix.c_str(), vOriginalFileName.c_str());
            LogVarError("Debug : %s", Shader.getInfoLog());
            LogVarError("Debug : %s", Shader.getInfoDebugLog());

            std::string log = Shader.getInfoLog();
            if (!log.empty()) {
                m_errors[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Parse Errors", shaderTypeString, log);
                }
            }
#ifdef VERBOSE_DEBUG
            log = Shader.getInfoDebugLog();
            if (!log.empty()) {
                m_errors[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Parse Errors", shaderTypeString, log);
                }
            }
#endif
            m_warnings.clear();
        } else {
            m_errors.clear();
            std::string log = Shader.getInfoLog();
            if (!log.empty()) {
                m_warnings[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Parse Warnings", shaderTypeString, log);
                }
            }
#ifdef VERBOSE_DEBUG
            log = Shader.getInfoDebugLog();
            if (!log.empty()) {
                m_warnings[shaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Parse Warnings", shaderTypeString, log);
                }
            }
#endif
        }

        if (vTraverser) {
            vTraverser(Shader.getIntermediate());
        }
    }
}

std::unordered_map<std::string, bool> ShaderCompiler::CollectUniformInfosFromIR(const glslang::TIntermediate& intermediate) {
    auto* root_ptr = intermediate.getTreeRoot();
    if (root_ptr == nullptr) {
        return {};
    }
    TUniformsIRLocator it;
    root_ptr->traverse(&it);
    return it.usedUniforms;
}

std::string ShaderCompiler::m_getSuffix(const std::string& name) {
    const size_t pos = name.rfind('.');
    return (pos == std::string::npos) ? "" : name.substr(name.rfind('.') + 1);
}

EShLanguage ShaderCompiler::m_getShaderStage(const std::string& stage) {
    if (stage == "vert") {
        return EShLangVertex;
    } else if (stage == "ctrl") {
        return EShLangTessControl;
    } else if (stage == "eval") {
        return EShLangTessEvaluation;
    } else if (stage == "geom") {
        return EShLangGeometry;
    } else if (stage == "frag") {
        return EShLangFragment;
    } else if (stage == "comp") {
        return EShLangCompute;
    } else if (stage == "rgen") {
        return EShLangRayGen;
    } else if (stage == "rint") {
        return EShLangIntersect;
    } else if (stage == "miss") {
        return EShLangMiss;
    } else if (stage == "ahit") {
        return EShLangAnyHit;
    } else if (stage == "chit") {
        return EShLangClosestHit;
    } else {
        assert(0 && "Unknown shader stage");
        return EShLangCount;
    }
}

std::string ShaderCompiler::m_getShaderStageString(const EShLanguage& stage) {
    switch (stage) {
        case EShLangVertex: return "vert";
        case EShLangTessControl: return "ctrl";
        case EShLangTessEvaluation: return "eval";
        case EShLangGeometry: return "geom";
        case EShLangFragment: return "frag";
        case EShLangCompute: return "comp";
        case EShLangRayGen: return "rgen";
        case EShLangIntersect: return "rint";
        case EShLangMiss: return "miss";
        case EShLangAnyHit: return "ahit";
        case EShLangClosestHit: return "chit";
        case EShLangCallable: return "call";
        case EShLangTask: return "task";
        case EShLangMesh: return "mesh";
        case EShLangCount: break;
    }
    return "";
}

std::string ShaderCompiler::m_getFullShaderStageString(const EShLanguage& stage) {
    switch (stage) {
        case EShLangVertex: return "Vertex";
        case EShLangTessControl: return "Tesselation Control";
        case EShLangTessEvaluation: return "Tesselation Evaluation";
        case EShLangGeometry: return "Geometry";
        case EShLangFragment: return "Fragment";
        case EShLangCompute: return "Compute";
        case EShLangRayGen: return "Ray Generation";
        case EShLangIntersect: return "Ray Intersection";
        case EShLangMiss: return "Miss";
        case EShLangAnyHit: return "Any Hit";
        case EShLangClosestHit: return "Closest Hit";
        case EShLangCallable: return "Callable";
        case EShLangTask: return "Task";
        case EShLangMesh: return "Mesh";
        case EShLangCount: break;
    }
    return "";
}