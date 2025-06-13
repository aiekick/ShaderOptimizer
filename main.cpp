#include <app.h>
#include <ezlibs/ezTools.hpp>
#include <exception>

int main(int vArgc, char** vArgv) {
    int ret = EXIT_SUCCESS;
    App app;
    try {
        if (app.init(vArgc, vArgv)) {
            app.run();
        }
    } catch (std::exception& ex) {
        LogVarError("Exception : %s", ex.what());
        ret = EXIT_FAILURE;
        EZ_TOOLS_DEBUG_BREAK;
    } catch (...) {
        LogVarError("Unknown exception");
        ret = EXIT_FAILURE;
        EZ_TOOLS_DEBUG_BREAK;
    }
    app.unit();
    return ret;
}
