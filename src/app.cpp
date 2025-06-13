#include <app.h>
#include <ezlibs/ezTools.hpp>
#include <headers/ShaderOptimizerBuild.h>
#include <backend/backend.h>
#include <frontend/frontend.h>
#include <project/projectFile.h>

bool App::init(int vArgc, char** vArgv) {
    mp_app = std::make_unique<ez::App>(vArgc, vArgv);
    if (m_defineCli(vArgc, vArgv)) {
        ProjectFile::initSingleton();
        Backend::initSingleton();
        Frontend::initSingleton();
        Backend::ref().init();
        return true;
    }
    return false;
}

void App::run(){
    Backend::ref().loop();
}

void App::unit() {
    Backend::ref().unit();
    Frontend::unitSingleton();
    Backend::unitSingleton();
    ProjectFile::unitSingleton();
    mp_args.reset();
    mp_app.reset();
    ez::Log::Instance()->close();
}

bool App::m_defineCli(int vArgc, char** vArgv) {
    LogVarLightInfo("\n" ShaderOptimizer_FigFontLabel "\n-----------");
    LogVarLightInfo("[[ %s Beta v%s ]]", ShaderOptimizer_Label, ShaderOptimizer_BuildId);
    mp_args = std::make_unique<ez::Args>("ShaderOptimizer");
    mp_args->addDescription("A tool for optimize a shader");
    mp_args->addOptional("--headless").help("Launch the tool without the Gui");
    // do stuff
    if (mp_args->parse(vArgc, vArgv)){
        // do stuff
        return true;
    }
    return false;
}