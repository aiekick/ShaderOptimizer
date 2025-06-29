/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once
#pragma warning(disable : 4251)

#include <glslang/MachineIndependent/localintermediate.h>
#include <glslang/Include/InfoSink.h>

#include <unordered_map>
#include <string>

// we will parse the shader
// all seen uniforms will be marcked as used
// but for the moment we ddont search to know if the uniform is really used
// by ex if at end its value is consumed by nothing

namespace ShaderOpt {

class TUniformsIRLocator : public glslang::TIntermTraverser {
public:
    std::unordered_map<std::string, bool> usedUniforms;

public:
    TUniformsIRLocator();

    virtual bool visitBinary(glslang::TVisit, glslang::TIntermBinary* vNode);
    virtual void visitSymbol(glslang::TIntermSymbol* vNode);

protected:
    TUniformsIRLocator(TUniformsIRLocator&);
    TUniformsIRLocator& operator=(TUniformsIRLocator&);
};

}  // namespace ShaderOpt
