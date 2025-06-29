#include <ShaderOpt/flopEstimator.h>

#include <iostream>
#include <sstream>
#include <regex>
#include <unordered_map>
#include <map>
#include <iomanip>

namespace ShaderOpt {

/* ──────────────────────────────────────────────────────────────
   1.  Barèmes de coût scalaire (FP-32)                           */
static const std::unordered_map<std::string, double> kScalarFlopCost = {
    /*  Les opcodes dans le dump `spirv-dis` sont sans préfixe « Op »  */
    {"FAdd", 1.0},
    {"FSub", 1.0},
    {"FMul", 1.0},
    {"FDiv", 1.0},
    {"FRem", 1.0},
    {"FNegate", 0.0},
    {"FAbs", 0.0},
    {"FMA", 2.0},
    {"FFma", 2.0}};

/*  Built-ins GLSL.std.450  ───────────────────────────────────── */
static const std::unordered_map<std::string, double> kExtInstCost = {
    /* racines              */ 
    {"Sqrt", 4.0},
    {"InverseSqrt", 4.0},
    /* trigo                */ 
    {"Sin", 10.0},
    {"Cos", 10.0},
    {"Tan", 10.0},
    /* expo / log / pow     */ 
    {"Exp", 20.0},
    {"Exp2", 20.0},
    {"Log", 20.0},
    {"Log2", 20.0},
    {"Pow", 20.0},
    /* divers               */ 
    {"Atan2", 24.0},
    {"Normalize", 7.0},  // coût scalaire, sera × largeur
    {"Length", 9.0},
    {"Floor", 12.0},
    {"Fract", 12.0},
    {"FAbs", 8.0}};

/* ──────────────────────────────────────────────────────────────
   2.  Constructeur / reset                                       */
FlopEstimator::FlopEstimator(const std::string& vSpirvCode) {
    m_parseTypes(vSpirvCode);
    m_gatherStats(vSpirvCode);
}

void FlopEstimator::clear() {
    m_types.clear();
    m_stats = {};
}

FlopEstimator::Stats const& FlopEstimator::stats() const {
    return m_stats;
}

/* ──────────────────────────────────────────────────────────────
   3.  Phase type : détecter vec / mat                            */
void FlopEstimator::m_parseTypes(const std::string& text) {
    /* 7:     TypeVector 6(float) 2 */
    static const std::regex reVec(R"(^\s*(\d+):\s+TypeVector\s+\d+\([^)]*\)\s+(\d+))");

    /* X:     TypeMatrix 7(fvec3) 4 */
    static const std::regex reMat(R"(^\s*(\d+):\s+TypeMatrix\s+(\d+)\([^)]*\)\s+(\d+))");

    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line)) {
        std::smatch m;
        if (std::regex_search(line, m, reVec)) {
            unsigned id = std::stoul(m[1]);
            unsigned comps = std::stoul(m[2]);
            m_types[id].comps = comps;
        } else if (std::regex_search(line, m, reMat)) {
            unsigned id = std::stoul(m[1]);
            unsigned vecTypeId = std::stoul(m[2]);
            unsigned cols = std::stoul(m[3]);
            unsigned rows = m_types[vecTypeId].comps;
            m_types[id].comps = rows * cols;  // mat = rows × cols
        }
    }
}

/* ──────────────────────────────────────────────────────────────
   4.  Phase instructions : comptage FLOPs                       */
void FlopEstimator::m_gatherStats(const std::string& text) {
    /* Exemple : 36:  7(fvec2) FAdd 34 35  */
    static const std::regex reOp(R"(^\s*\d+:\s+(\d+)\([^)]*\)\s+(\w+))");

    /* Exemple : 51:  6(float) ExtInst 1(GLSL.std.450) 25(Atan2) 47 50 */
    static const std::regex reExt(R"(^\s*\d+:\s+(\d+)\([^)]*\)\s+ExtInst\s+\d+\(GLSL\.std\.450\)\s+\d+\((\w+)\))");

    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line)) {
        std::smatch m;
        std::string key;
        unsigned typeId = 0;
        double scalarCost = 0.0;

        /* ------------------------ ExtInst ----------------------- */
        if (std::regex_search(line, m, reExt)) {
            typeId = std::stoul(m[1]);
            std::string builtin = m[2];
            key = "ExtInst." + builtin;
            auto it = kExtInstCost.find(builtin);
            scalarCost = (it != kExtInstCost.end()) ? it->second : 12.0;
        }
        /* ---------------------- Op ALU -------------------------- */
        else if (std::regex_search(line, m, reOp)) {
            typeId = std::stoul(m[1]);
            std::string op = m[2];
            auto it = kScalarFlopCost.find(op);
            if (it == kScalarFlopCost.end())
                continue;  // opcode non compté
            key = op;
            scalarCost = it->second;
        } else
            continue;  // ligne sans intérêt

        /* Largeur = nb composantes (vec/mat) -------------------- */
        unsigned width = 1;
        auto itTy = m_types.find(typeId);
        if (itTy != m_types.end())
            width = itTy->second.comps;

        double flop = scalarCost * width;
        m_stats.count[key] += 1;
        m_stats.flops[key] += flop;
        m_stats.total += flop;
    }
}

}  // namespace ShaderOpt
