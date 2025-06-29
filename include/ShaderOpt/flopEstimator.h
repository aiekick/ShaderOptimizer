#pragma once
#include <ShaderOpt/shaderCompiler.h>
#include <string>
#include <cstdint>

namespace ShaderOpt {

class FlopEstimator {
public:
    struct Stats {
        std::map<std::string, size_t> count;
        std::map<std::string, double> flops;
        double total{};
    };

private:
    struct TypeInfo {
        uint32_t comps = 1;
    };

private:
    std::unordered_map<uint32_t, TypeInfo> m_types;
    Stats m_stats;

public:
    explicit FlopEstimator(const std::string& vSpirvCode);
    void clear();
    Stats const& stats() const;

private:
    void m_parseTypes(std::string const& text);
    void m_gatherStats(std::string const& text);
};

}  // namespace ShaderOpt
