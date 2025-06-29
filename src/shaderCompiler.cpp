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

#include <ShaderOpt/shaderCompiler.h>
#include <ShaderOpt/resLimits.h>
#include <ShaderOpt/uniformsIRLocator.h>
#include <ShaderOpt/flopTraverser.h>

#include <ezlibs/ezLog.hpp>

#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Include/ShHandle.h>
#include <glslang/OSDependent/osinclude.h>
#include <glslang/MachineIndependent/Initialize.h>
#include <StandAlone/DirStackFileIncluder.h>

#include <cstdio>     // printf, fprintf
#include <cstdlib>    // abort
#include <iostream>   // std::cout
#include <stdexcept>  // std::exception
#include <algorithm>  // std::min, std::max
#include <fstream>    // std::ifstream
#include <chrono>     // timer

#define VERBOSE_DEBUG

namespace ShaderOpt {

ShaderCompiler::SpirvCode ShaderCompiler::CompileGLSLString(
    const std::string& vCode,
    const EShLanguage& vShaderType,
    const ShaderEntryPoint& vEntryPoint,
    ShaderMessagingFunction vMessagingFunction,
    TraverserFunction vTraverser,
    std::string* vShaderCode,
    std::unordered_map<std::string, bool>* vUsedUniforms) {
    m_errors.clear();
    m_warnings.clear();

    SpirvCode SpirV;

    std::string InputGLSL = vCode;

    std::string shaderTypeString = m_getFullShaderStageString(vShaderType);

    if (!InputGLSL.empty() && vShaderType != EShLanguage::EShLangCount) {
        glslang::TShader Shader(vShaderType);

        if (vShaderCode)
            *vShaderCode = InputGLSL;

        // Set up Vulkan/SpirV Environment
        int ClientInputSemanticsVersion = 100;
        Shader.setEnvInput(glslang::EShSourceGlsl, vShaderType, glslang::EShClientOpenGL, ClientInputSemanticsVersion);
        Shader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
        Shader.setAutoMapLocations(true);

        const char* InputCString = InputGLSL.c_str();

        Shader.setStrings(&InputCString, 1);

        EShMessages messages = (EShMessages)(EShMsgDefault);

#ifdef _DEBUG
        //messages = (EShMessages)(messages | EShMsgDebugInfo);
#endif

        const int DefaultVersion = 110;  // 110 for desktop, 100 for es

        DirStackFileIncluder Includer;

        std::string PreprocessedGLSL;

        if (!Shader.preprocess(&glslang::DefaultTBuiltInResource, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL, Includer)) {
            LogVarError("Debug Preprocessing : GLSL stage %s Preprocessing Failed", shaderTypeString.c_str());
            std::string log = Shader.getInfoLog();
            if (!log.empty()) {
                LogVarDebugInfo("Debug Preprocessing Errors : %s", log.c_str());
                m_errors[vShaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Preprocessing Errors", shaderTypeString, log);
                }
            }
            m_warnings.clear();

            return SpirV;
        } else {
            m_errors.clear();
            std::string log = Shader.getInfoLog();
            if (!log.empty()) {
                LogVarWarning("Debug Preprocessing Warnings : %s", log.c_str());
                m_warnings[vShaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Preprocessing Warnings", shaderTypeString, log);
                }
            }
        }

        const char* PreprocessedCStr = PreprocessedGLSL.c_str();
        Shader.setStrings(&PreprocessedCStr, 1);

        auto entry = vEntryPoint;
        if (entry.empty())
            entry = "main";
        Shader.setEntryPoint(entry.c_str());
        Shader.setSourceEntryPoint("main");

        Shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

        if (!Shader.parse(&glslang::DefaultTBuiltInResource, 100, false, messages)) {
            LogVarError("Debug Parse (%s) : GLSL stage %s Parse Failed", entry.c_str(), shaderTypeString.c_str());
            std::string log = Shader.getInfoLog();
            if (!log.empty()) {
                LogVarError("Debug Parse Errors (%s) : %s", entry.c_str(), log.c_str());
                m_errors[vShaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Parse Parse Errors", shaderTypeString, log);
                }
            }
            m_warnings.clear();

            return SpirV;
        } else {
            m_errors.clear();
            std::string log = Shader.getInfoLog();
            if (!log.empty()) {
                LogVarWarning("Debug Parse Warnings (%s) : %s", entry.c_str(), log.c_str());
                m_warnings[vShaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Parse Warnings", shaderTypeString, log);
                }
            }
        }

        glslang::TProgram Program;
        Program.addShader(&Shader);

        if (!Program.link(messages)) {
            LogVarError("Debug Linking (%s) : GLSL stage %s Linking Failed", entry.c_str(), shaderTypeString.c_str());
            std::string log = Program.getInfoLog();
            if (!log.empty()) {
                LogVarDebugInfo("Debug Linking Errors (%s) : %s", entry.c_str(), log.c_str());
                m_errors[vShaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Linking Errors", shaderTypeString, log);
                }
            }
            m_warnings.clear();

            return SpirV;
        } else {
            m_errors.clear();
            std::string log = Shader.getInfoLog();
            if (!log.empty()) {
                LogVarWarning("Debug Linking Warnings (%s) : %s", entry.c_str(), log.c_str());
                m_warnings[vShaderType].push_back(log);
                if (vMessagingFunction) {
                    vMessagingFunction("Warnings", shaderTypeString, log);
                }
            }
        }

        if (vUsedUniforms) {
            auto usedUniforms = m_collectUniformInfosFromIR(*Shader.getIntermediate());
            for (auto u : usedUniforms) {
                (*vUsedUniforms)[u.first] |= u.second;
            }
        }

        {
            double worstFlops = m_computeFlops(*Shader.getIntermediate());
        }

        if (vTraverser) {
            vTraverser(Shader.getIntermediate());
        }

        spv::SpvBuildLogger logger;
        glslang::SpvOptions spvOptions;
        spvOptions.optimizeSize = true;
#ifdef _DEBUG
        //spvOptions.generateDebugInfo = true;
#else
        spvOptions.stripDebugInfo = true;
#endif

        glslang::GlslangToSpv(*Program.getIntermediate(vShaderType), SpirV, &logger, &spvOptions);

        if (logger.getAllMessages().length() > 0) {
            std::string allmsgs = logger.getAllMessages();
            std::cout << allmsgs << std::endl;
        }
    }

    if (SpirV.empty()) {
        LogVarError("Debug : Shader stage %s Spirv generation : NOK !", shaderTypeString.c_str());
    }

    return SpirV;
}

std::unordered_map<std::string, bool> ShaderCompiler::m_collectUniformInfosFromIR(const glslang::TIntermediate& intermediate) {
    auto* root_ptr = intermediate.getTreeRoot();
    if (root_ptr == nullptr) {
        return {};
    }
    TUniformsIRLocator it;
    root_ptr->traverse(&it);
    return it.usedUniforms;
}

double ShaderCompiler::m_computeFlops(const glslang::TIntermediate& intermediate) {
    auto* root_ptr = intermediate.getTreeRoot();
    if (root_ptr == nullptr) {
        return {};
    }
    FlopTraverser it;
    root_ptr->traverse(&it);
    return it.worstFlops();
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

}  // namespace ShaderOpt
