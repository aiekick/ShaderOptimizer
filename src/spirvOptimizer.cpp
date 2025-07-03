#include <ShaderOpt/spirvOptimizer.h>
#include <spirv-tools/optimizer.hpp>
#include <ezlibs/ezLog.hpp>

using namespace spvtools;

namespace ShaderOpt {

SpirvCode const& SpirvOptimizer::Optimize(const SpirvCode& vSpirv, const SpirvOptimizer::Config& vConfig, const spv_target_env& vEnv) {
    m_source = vSpirv;
    spvtools::Optimizer optimizer(vEnv);
    optimizer.SetMessageConsumer([](spv_message_level_t vLevel, const char* vSource, const spv_position_t& vPosition, const char* vMessage) {
        switch (vLevel) {
            case SPV_MSG_FATAL:
            case SPV_MSG_INTERNAL_ERROR:
            case SPV_MSG_ERROR: {
                LogVarError("%s[%i:%i:%i] %s", vSource, vPosition.line, vPosition.column, vPosition.index, vMessage);
            } break;
            case SPV_MSG_WARNING: {
                LogVarWarning("%s[%i:%i:%i] %s", vSource, vPosition.line, vPosition.column, vPosition.index, vMessage);
            } break;
            case SPV_MSG_INFO: {
                LogVarInfo("%s[%i:%i:%i] %s", vSource, vPosition.line, vPosition.column, vPosition.index, vMessage);
            } break;
            case SPV_MSG_DEBUG: {
                LogVarDebugInfo("%s[%i:%i:%i] %s", vSource, vPosition.line, vPosition.column, vPosition.index, vMessage);
            } break;
        }
    });
    optimizer.SetValidateAfterAll(true);

    spvtools::ValidatorOptions validator_options;
    spvtools::OptimizerOptions optimizer_options;


    optimizer_options.set_validator_options(validator_options);

    //optimizer.RegisterPass(Optimizer::PassToken(spvtools::CreateInlineExhaustivePass()));
    //optimizer.RegisterPass(Optimizer::PassToken(spvtools::CreateSimplificationPass()));
    //optimizer.RegisterPerformancePasses();  // équivalent à -O
    //optimizer.RegisterSizePasses();         // équivalent à -Os

    bool preserve_interface = false; // false:perf, true:size

    //optimizer.RegisterPerformancePasses();  // équivalent à -O
    
    optimizer.RegisterPass(CreateWrapOpKillPass());
    optimizer.RegisterPass(CreateDeadBranchElimPass());
    optimizer.RegisterPass(CreateMergeReturnPass());
    optimizer.RegisterPass(CreateInlineExhaustivePass());
    optimizer.RegisterPass(CreateEliminateDeadFunctionsPass());
    optimizer.RegisterPass(CreateAggressiveDCEPass(preserve_interface));
    optimizer.RegisterPass(CreatePrivateToLocalPass());
    optimizer.RegisterPass(CreateLocalSingleBlockLoadStoreElimPass());
    optimizer.RegisterPass(CreateLocalSingleStoreElimPass());
    optimizer.RegisterPass(CreateAggressiveDCEPass(preserve_interface));
    optimizer.RegisterPass(CreateScalarReplacementPass(0));
    optimizer.RegisterPass(CreateLocalAccessChainConvertPass());
    optimizer.RegisterPass(CreateLocalSingleBlockLoadStoreElimPass());
    optimizer.RegisterPass(CreateLocalSingleStoreElimPass());
    optimizer.RegisterPass(CreateLocalMultiStoreElimPass());
    optimizer.RegisterPass(CreateAggressiveDCEPass(preserve_interface));
    optimizer.RegisterPass(CreateCCPPass());
    optimizer.RegisterPass(CreateAggressiveDCEPass(preserve_interface));
    optimizer.RegisterPass(CreateLoopUnrollPass(true));
    optimizer.RegisterPass(CreateDeadBranchElimPass());
    optimizer.RegisterPass(CreateRedundancyEliminationPass());
    optimizer.RegisterPass(CreateCombineAccessChainsPass());
    optimizer.RegisterPass(CreateSimplificationPass());
    optimizer.RegisterPass(CreateScalarReplacementPass(0));
    optimizer.RegisterPass(CreateLocalAccessChainConvertPass());
    optimizer.RegisterPass(CreateLocalSingleBlockLoadStoreElimPass());
    optimizer.RegisterPass(CreateLocalSingleStoreElimPass());
    optimizer.RegisterPass(CreateAggressiveDCEPass(preserve_interface));
    optimizer.RegisterPass(CreateSSARewritePass());
    optimizer.RegisterPass(CreateAggressiveDCEPass(preserve_interface));
    optimizer.RegisterPass(CreateVectorDCEPass());
    optimizer.RegisterPass(CreateDeadInsertElimPass());
    optimizer.RegisterPass(CreateDeadBranchElimPass());
    optimizer.RegisterPass(CreateSimplificationPass());
    optimizer.RegisterPass(CreateIfConversionPass());
    optimizer.RegisterPass(CreateCopyPropagateArraysPass());
    optimizer.RegisterPass(CreateReduceLoadSizePass());
    optimizer.RegisterPass(CreateAggressiveDCEPass(preserve_interface));
    optimizer.RegisterPass(CreateBlockMergePass());
    optimizer.RegisterPass(CreateRedundancyEliminationPass());
    optimizer.RegisterPass(CreateDeadBranchElimPass());
    optimizer.RegisterPass(CreateBlockMergePass());
    optimizer.RegisterPass(CreateSimplificationPass());
    
    if (optimizer.Run(m_source.data(), m_source.size(), &m_target, optimizer_options)) {
        return m_target;
    }
    return m_source;
}

}  // namespace ShaderOpt
