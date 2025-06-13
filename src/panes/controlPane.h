#pragma once
#include <abstracts/ASingleton.hpp>

#include <imguipack.h>
#include <cstdint>
#include <memory>
#include <string>

class ProjectFile;
class ControlPane : public AbstractPane {
    IMPLEMENT_SHARED_SINGLETON(ControlPane)
public:
    bool Init() override;
    void Unit() override;
    bool DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened, ImGuiContext* vContextPtr, void* vUserDatas) override;

public:
    ControlPane();                                               // Prevent construction
    ControlPane(const ControlPane&) = default;                     // Prevent construction by copying
    ControlPane& operator=(const ControlPane&) { return *this; };  // Prevent assignment
    virtual ~ControlPane();                                      // Prevent unwanted destruction};
};
