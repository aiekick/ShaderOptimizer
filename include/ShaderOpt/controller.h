#pragma once
#include <ShaderOpt/shaderCompiler.h>
#include <ShaderOpt/flopEstimator.h>
#include <ShaderOpt/spirvOptimizer.h>
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
        SpirvOptimizer::Config m_optimizerConfig;
    };
    struct Result {
        bool valid{};
        std::string result;
        FlopEstimator::OpeStats stats;
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
    std::string m_convertToHumanReadableSpirv(const SpirvCode &vSpirvCode);
    std::string m_convertToGlslCode(const SpirvCode &vSpirvCode);
    std::string m_convertToCppCode(const SpirvCode &vSpirvCode);
    std::string m_convertToMslCode(const SpirvCode &vSpirvCode);
    std::string m_convertToHlslCode(const SpirvCode &vSpirvCode);
};

}  // namespace ShaderOpt
