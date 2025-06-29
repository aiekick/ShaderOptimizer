#pragma once
#include <ShaderOpt/shaderCompiler.h>
#include <ShaderOpt/flopEstimator.h>
#include <string>

namespace ShaderOpt {

class Controller {
public:
    struct Config {
        enum class OutputType {  //
            GLSL = 0,
            HLSL,
            MSL,
            SPIRV,
            AST,
            CPP,
            Count
        } outputType = OutputType::GLSL;
    };
    struct Result {
        bool valid{};
        std::string result;
        FlopEstimator::Stats stats;
    };

private:
    std::string m_sourceCode;
    std::string m_targetCode;

public:
    bool init();
    void unit();
    void loadShader(const std::string &vFilePathName);
    void setSource(const std::string &vCode);
    std::string getSource();
    void setTarget(const std::string &vCode);
    std::string getTarget();
    Controller::Result optimize(const Controller::Config &vInfos);

private:
    std::string m_convertToHumanReadableSpirv(const ShaderCompiler::SpirvCode &vSpirvCode);
    std::string m_convertToGlslCode(const ShaderCompiler::SpirvCode &vSpirvCode);
    std::string m_convertToCppCode(const ShaderCompiler::SpirvCode &vSpirvCode);
    std::string m_convertToMslCode(const ShaderCompiler::SpirvCode &vSpirvCode);
    std::string m_convertToHlslCode(const ShaderCompiler::SpirvCode &vSpirvCode);
};

}  // namespace ShaderOpt
