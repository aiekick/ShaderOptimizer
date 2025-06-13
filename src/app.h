#pragma once
#include <ezlibs/ezApp.hpp>
#include <ezlibs/ezArgs.hpp>
#include <memory>

class App {
private:
    std::unique_ptr<ez::App> mp_app;
    std::unique_ptr<ez::Args> mp_args;

public:
    bool init(int vArgc, char** vArgv);
    void run();
    void unit();

private:
    bool m_defineCli(int vArgc, char** vArgv);
};
