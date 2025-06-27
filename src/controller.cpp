#include <controller.h>
#include <resLimits.h>
#include <uniformsIRLocator.h>

#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Include/ShHandle.h>
#include <glslang/OSDependent/osinclude.h>
#include <StandAlone/DirStackFileIncluder.h>

bool Controller::init() {
    return glslang::InitializeProcess();
}

void Controller::unit() {
    glslang::FinalizeProcess();
}

void Controller::loadShader(const std::string& vFilePathName) {}

std::string Controller::getSource() {
    return {};
}

std::string Controller::getTarget() {
    return {};
}
