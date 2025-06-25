#pragma once
#include <string>

class Controller {
public:
    bool init();
    void unit();

    void loadShader(const std::string& vFilePathName);
    std::string getSource();
    std::string getTarget();
};
