#pragma once

#include <ShaderOpt/defs.h>
#include <spirv-tools/libspirv.hpp>

namespace ShaderOpt {

class SpirvOptimizer {
public:
    struct Config {};

private:
    SpirvCode m_source;
    SpirvCode m_target;

public:
    SpirvCode const& Optimize(const SpirvCode& vSpirv, const Config& vConfig, const spv_target_env& vEnv);
};

}  // namespace ShaderOpt
