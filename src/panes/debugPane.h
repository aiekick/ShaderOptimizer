#pragma once
#include <abstracts/ASingleton.hpp>

#include <imguipack.h>
#include <cstdint>
#include <memory>
#include <string>

class ProjectFile;
class DebugPane : public AbstractPane {
    IMPLEMENT_SHARED_SINGLETON(DebugPane)
public:
    bool Init() override;
    void Unit() override;
    bool DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened, ImGuiContext* vContextPtr, void* vUserDatas) override;

public:
    DebugPane();                                               // Prevent construction
    DebugPane(const DebugPane&) = default;                     // Prevent construction by copying
    DebugPane& operator=(const DebugPane&) { return *this; };  // Prevent assignment
    virtual ~DebugPane();                                      // Prevent unwanted destruction};
};
