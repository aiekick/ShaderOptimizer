#pragma once
#include <ShaderOpt/shaderCompiler.h>
#include <string>
#include <cstdint>

namespace ShaderOpt {

class FlopEstimator {
public:
    struct OpeStats {
        struct OpeStat {
            std::string name;
            size_t count;
            double flops;
        };
        std::map<std::string, OpeStat> ops;
        double total{};
    };

private:
    struct TypeInfo {
        uint32_t comps = 1;
    };

private:
    std::unordered_map<uint32_t, TypeInfo> m_types;
    OpeStats m_stats;

public:
    explicit FlopEstimator(const std::string& vSpirvCode);
    void clear();
    OpeStats const& stats() const;

private:
    void m_parseTypes(std::string const& text);
    void m_gatherStats(std::string const& text);
};

}  // namespace ShaderOpt
