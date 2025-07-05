#include <ShaderOpt/flopTraverser.h>

#include <unordered_map>
#include <algorithm>

using namespace glslang;
namespace ShaderOpt {

/* ------------ table (même que ci-dessus, portée fichier) ------------ */
static const std::unordered_map<glslang::TOperator, double> kOpCost{
    {EOpAdd, 1},   {EOpSub, 1},  {EOpMul, 1},    {EOpDiv, 1},       {EOpAbs, 0},    {EOpFma, 2},  // mul+add
    {EOpSin, 10},  {EOpCos, 10}, {EOpTan, 10},   {EOpSqrt, 4},      {EOpInverseSqrt, 4}, {EOpExp, 20},   {EOpExp2, 20},  {EOpLog, 20},
    {EOpLog2, 20}, {EOpPow, 20}, {EOpNormalize, 7}, {EOpLength, 9},      {EOpFloor, 12}, {EOpFract, 12},
};
/* ----------- petits utilitaires ------------------------------------ */
double FlopTraverser::scalarCost(TOperator op) const {
    auto it = kOpCost.find(op);
    return it == kOpCost.end() ? 0.0 : it->second;
}

unsigned FlopTraverser::width(const TType& ty) const {
    return std::max(1, ty.getVectorSize() * ty.getMatrixCols() * ty.getMatrixRows());
}

/* ----------- unary / binary → ajout coût --------------------------- */
bool FlopTraverser::visitUnary(TVisit, TIntermUnary* node) {
    double add = scalarCost(node->getOp()) * width(node->getType());
    if (!m_pathCost.empty())
        m_pathCost.top() += add;
    return true;
}
bool FlopTraverser::visitBinary(TVisit, TIntermBinary* node) {
    double add = scalarCost(node->getOp()) * width(node->getType());
    if (!m_pathCost.empty())
        m_pathCost.top() += add;
    return true;
}

/* ----------- aggregates (built-ins appelés comme Intrinsics) ------- */
bool FlopTraverser::visitAggregate(TVisit, TIntermAggregate* node) {
    const TOperator op = node->getOp();

    /* cas 1 : la plupart des built-ins apparaissent avec leur enum dédié
       (EOpSin, EOpPow, EOpNormalize …).  */
    if (op != EOpFunctionCall) {
        double add = scalarCost(op) * width(node->getType());
        if (!m_pathCost.empty())
            m_pathCost.top() += add;
        return true;  // rien d’autre à faire
    }

    /* cas 2 : opérateur = EOpFunctionCall  → peut être un appel utilisateur
     *ou* un builtin spécial.  On passe par TFunction. */
    /*const TFunction* fn = node->getName();
    if (fn && fn->isBuiltIn()) {
        // nom du builtin sous forme de chaîne : fn->getName().c_str()
        double add = scalarCost(op) * width(node->getType());
        if (!m_pathCost.empty())
            m_pathCost.top() += add;
    }*/

    if (op == EOpFunction && !m_pathCost.empty()) {
        m_worst = std::max(m_worst, m_pathCost.top());
    }
    /* on laisse quand même les enfants se parcourir (args) */
    return true;
}

/* ----------- if / switch : explorer branches ----------------------- */
bool FlopTraverser::visitSelection(TVisit pre, TIntermSelection* node) {
    if (pre == EvPreVisit) {
        m_pathCost.push(0);  // coût accumulé dans « then »
        if (node->getTrueBlock()) {
            node->getTrueBlock()->traverse(this);
        }
        double thenCost = m_pathCost.top();
        m_pathCost.pop();

        double elseCost = 0;
        if (node->getFalseBlock()) {
            m_pathCost.push(0);
            node->getFalseBlock()->traverse(this);
            elseCost = m_pathCost.top();
            m_pathCost.pop();
        }

        double add = std::max(thenCost, elseCost);
        if (!m_pathCost.empty())
            m_pathCost.top() += add;
        return false;  // on vient de parcourir manuellement
    }
    return true;
}

/* ----------- boucles structurées ----------------------------------- */
bool FlopTraverser::visitLoop(TVisit pre, TIntermLoop* node) {
    if (pre == EvPreVisit) {
        /* est-ce un for(i=0; i<const; ++i) ? */
        int iterations = -1;
        /*if (node-> getLoopType() == EvLoopFor && node->getCondition()) {
            const TIntermConstantUnion* cu = node->getCondition()->getAsConstantUnion();
            if (cu && cu->isScalarInteger())
                iterations = cu->getConstArray()[0].getIConst();
        }*/
        if (iterations <= 0)
            iterations = 32;  // borne pessimiste

        m_pathCost.push(0);
        node->getBody()->traverse(this);
        double body = m_pathCost.top();
        m_pathCost.pop();

        double add = body * iterations;
        if (!m_pathCost.empty())
            m_pathCost.top() += add;
        return false;
    }
    return true;
}


}  // namespace ShaderOpt
