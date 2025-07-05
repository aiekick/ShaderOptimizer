#include <ShaderOpt/controller.h>
#include <ShaderOpt/resLimits.h>
#include <ShaderOpt/uniformsIRLocator.h>
#include <ShaderOpt/spirvOptimizer.h>
#include <ShaderOpt/flopEstimator.h>

#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Include/ShHandle.h>
#include <glslang/OSDependent/osinclude.h>
#include <StandAlone/DirStackFileIncluder.h>
#include <SPIRV/disassemble.h>

#include <spirv_glsl.hpp>
#include <spirv_msl.hpp>
#include <spirv_cpp.hpp>
#include <spirv_hlsl.hpp>

#include <ezlibs/ezFile.hpp>

#include <sstream>
#include <regex>

namespace ShaderOpt {

bool Controller::init() {
    return glslang::InitializeProcess();
}

void Controller::unit() {
    glslang::FinalizeProcess();
}

void Controller::loadShader(const std::string& vFilePathName) {
    setSource(ez::file::loadFileToString(vFilePathName));
}

void Controller::setSource(const std::string& vCode) {
    m_sourceCode = vCode;
}

std::string Controller::getSource() {
    return m_sourceCode;
}

void Controller::setTarget(const std::string& vCode) {
    m_targetCode = vCode;
}

std::string Controller::getTarget() {
    return m_targetCode;
}

inline float asFloat(std::uint32_t bits) {
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

Controller::Result Controller::optimize(const Controller::Config& vConfig) {
    try {
        ShaderCompiler compiler;
        ShaderCompiler::Config shaderConfig;
        shaderConfig.debug = vConfig.debug;
        Controller::Result ret;
        if (vConfig.outputType != Config::OutputType::AST) {
            const auto spirv_source = compiler.CompileGLSLString(m_sourceCode, EShLangFragment, shaderConfig);
            if (spirv_source.empty()) {
                return {};
            }
            SpirvOptimizer optimizer;
            const auto spirv_optimized = optimizer.Optimize(spirv_source, vConfig.m_optimizerConfig, SPV_ENV_OPENGL_4_5);
            switch (vConfig.outputType) {
                case Config::OutputType::GLSL: {
                    ret.result = m_convertToGlslCode(spirv_optimized);
                } break;
                case Config::OutputType::HLSL: {
                    ret.result = m_convertToHlslCode(spirv_optimized);
                } break;
                case Config::OutputType::MSL: {
                    ret.result = m_convertToMslCode(spirv_optimized);
                } break;
                case Config::OutputType::CPP: {
                    ret.result = m_convertToCppCode(spirv_optimized);
                } break;
                case Config::OutputType::SPIRV: {
                    ret.result = m_convertToHumanReadableSpirv(spirv_optimized);
                    ret.stats = FlopEstimator(ret.result).stats();
                } break;
                case Config::OutputType::Count:
                default: break;
            }
        } else {
            TInfoSink sink;
            compiler.CompileGLSLString(m_sourceCode, EShLangFragment, shaderConfig, "main", nullptr, [&](glslang::TIntermediate* vIt) {
                if (vIt != nullptr) {
                    vIt->output(sink, true);
                }
            });
            ret.result = sink.debug.c_str();
        }
        if (!ret.result.empty()) {
            setTarget(ret.result);
            ret.valid = true;
            return ret;
        }
    } catch (const std::exception& ex) {
        LogVarError("Error on exception : %s", ex.what());
    } catch (...) {
        LogVarError("Unknow Error on exception");
    }
    return {};
}

std::string Controller::m_convertToHumanReadableSpirv(const SpirvCode& vSpirvCode) {
    std::stringstream ss;
    spv::Disassemble(ss, vSpirvCode);
    // const auto result = ss.str();
    // will tranform 6(float) Constant 1065353216 in 6(float) Constant 1065353216 (1.0)
    std::regex pattern(R"((\bfloat\)\s+Constant\s+)(\d+))");
    std::string line;
    std::string result;
    while (std::getline(ss, line)) {
        std::smatch m;
        if (std::regex_search(line, m, pattern)) {
            std::uint32_t raw = static_cast<std::uint32_t>(std::stoul(m[2]));
            float f = asFloat(raw);
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(7) << f;
            line += "  (" + oss.str() + ")";
        }
        result += line + "\n";
    }    
    return result;
}

std::string Controller::m_convertToGlslCode(const SpirvCode& vSpirvCode) {
    spirv_cross::CompilerGLSL glsl(vSpirvCode);

    // The SPIR-V is now parsed, and we can perform reflection on it.
    spirv_cross::ShaderResources resources = glsl.get_shader_resources();

    // Get all sampled images in the shader.
    for (auto& resource : resources.sampled_images) {
        unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
        unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
        LogVarInfo("Image %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);

        // Modify the decoration to prepare it for GLSL.
        glsl.unset_decoration(resource.id, spv::DecorationDescriptorSet);

        // Some arbitrary remapping if we want.
        glsl.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
    }

    // Set some options.
    spirv_cross::CompilerGLSL::Options options = glsl.get_common_options();
    options.enable_420pack_extension = false;
    options.force_zero_initialized_variables = true;
    glsl.set_common_options(options);

    // Compile to GLSL, ready to give to GL driver.
    return glsl.compile();
}

std::string Controller::m_convertToCppCode(const SpirvCode& vSpirvCode) {
    spirv_cross::CompilerCPP cpp(vSpirvCode);

    // The SPIR-V is now parsed, and we can perform reflection on it.
    spirv_cross::ShaderResources resources = cpp.get_shader_resources();

    // Get all sampled images in the shader.
    for (auto& resource : resources.sampled_images) {
        unsigned set = cpp.get_decoration(resource.id, spv::DecorationDescriptorSet);
        unsigned binding = cpp.get_decoration(resource.id, spv::DecorationBinding);
        LogVarInfo("Image %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);

        // Modify the decoration to prepare it for GLSL.
        cpp.unset_decoration(resource.id, spv::DecorationDescriptorSet);

        // Some arbitrary remapping if we want.
        cpp.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
    }

    // Set some options.
    spirv_cross::CompilerCPP::Options options = cpp.get_common_options();
    options.enable_420pack_extension = false;
    options.force_zero_initialized_variables = true;
    cpp.set_common_options(options);

    // Compile to GLSL, ready to give to GL driver.
    return cpp.compile();
}

std::string Controller::m_convertToMslCode(const SpirvCode& vSpirvCode) {
    spirv_cross::CompilerMSL msl(vSpirvCode);

    // The SPIR-V is now parsed, and we can perform reflection on it.
    spirv_cross::ShaderResources resources = msl.get_shader_resources();

    // Get all sampled images in the shader.
    for (auto& resource : resources.sampled_images) {
        unsigned set = msl.get_decoration(resource.id, spv::DecorationDescriptorSet);
        unsigned binding = msl.get_decoration(resource.id, spv::DecorationBinding);
        LogVarInfo("Image %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);

        // Modify the decoration to prepare it for GLSL.
        msl.unset_decoration(resource.id, spv::DecorationDescriptorSet);

        // Some arbitrary remapping if we want.
        msl.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
    }

    // Set some options.
    spirv_cross::CompilerMSL::Options options = msl.get_msl_options();
    msl.set_msl_options(options);

    // Compile to GLSL, ready to give to GL driver.
    return msl.compile();
}

std::string Controller::m_convertToHlslCode(const SpirvCode& vSpirvCode) {
    spirv_cross::CompilerHLSL hlsl(vSpirvCode);

    // The SPIR-V is now parsed, and we can perform reflection on it.
    spirv_cross::ShaderResources resources = hlsl.get_shader_resources();

    // Get all sampled images in the shader.
    for (auto& resource : resources.sampled_images) {
        unsigned set = hlsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
        unsigned binding = hlsl.get_decoration(resource.id, spv::DecorationBinding);
        LogVarInfo("Image %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);

        // Modify the decoration to prepare it for GLSL.
        hlsl.unset_decoration(resource.id, spv::DecorationDescriptorSet);

        // Some arbitrary remapping if we want.
        hlsl.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
    }

    // Set some options.
    spirv_cross::CompilerHLSL::Options options = hlsl.get_hlsl_options();
    hlsl.set_hlsl_options(options);

    // Compile to GLSL, ready to give to GL driver.
    return hlsl.compile();
}

}  // namespace ShaderOpt
