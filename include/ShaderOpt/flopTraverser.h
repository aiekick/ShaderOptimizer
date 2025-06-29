#pragma once
#pragma warning(disable : 4251)

#include <glslang/MachineIndependent/localintermediate.h>
#include <glslang/Include/InfoSink.h>

#include <stack>

namespace ShaderOpt {

class FlopTraverser : public glslang::TIntermTraverser {
private:
    std::stack<double> m_pathCost;  // somme partielle par niveau récursif
    double m_worst{};               // max observé

public:
    double worstFlops() const { return m_worst; }

private:
    bool visitUnary(glslang::TVisit, glslang::TIntermUnary* node) override;
    bool visitBinary(glslang::TVisit, glslang::TIntermBinary* node) override;
    bool visitAggregate(glslang::TVisit, glslang::TIntermAggregate* node) override;
    bool visitLoop(glslang::TVisit, glslang::TIntermLoop* node) override;
    bool visitSelection(glslang::TVisit, glslang::TIntermSelection* node) override;
    double scalarCost(glslang::TOperator op) const;
    unsigned width(const glslang::TType& ty) const;
};

}  // namespace ShaderOpt
