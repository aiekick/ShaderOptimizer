#pragma once
#include <abstracts/ASingleton.hpp>
#include <imguipack/imguipack.h>
#include <ShaderOpt/controller.h>

class Controller {
    IMPLEMENT_SINGLETON(Controller)
private:
    ShaderOpt::Controller m_controller;
    ShaderOpt::Controller::Result m_result;
    ImWidgets::QuickStringCombo m_outputTypeCombo;
    int32_t m_selectedIndex{};

public:
    bool init();
    void unit();
	void drawUI();
    ShaderOpt::Controller& getControllerRef();
};
