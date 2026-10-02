#pragma once

#include <string>

#include <iinf/core/types.h>

namespace iinf::platform {

enum class WindowMode : uint8 {
    Windowed,
    Borderless,
    Fullscreen
};

enum class WindowFlags : uint8 {
    None = 0,
    Resizable = 1 << 0,
    MatchPixelDensity = 1 << 1
};

constexpr WindowFlags operator|(WindowFlags left, WindowFlags right) {
    uint8 flags = static_cast<uint8>(left) | static_cast<uint8>(right);
    return static_cast<WindowFlags>(flags);
}

constexpr WindowFlags operator&(WindowFlags left, WindowFlags right) {
    uint8 flags = static_cast<uint8>(left) & static_cast<uint8>(right);
    return static_cast<WindowFlags>(flags);
}

struct WindowCreateInfo {
    std::string title = "Untitled";
    uint32 width = 1920;
    uint32 height = 1080;
    WindowMode mode = WindowMode::Windowed;
    WindowFlags flags = WindowFlags::MatchPixelDensity | WindowFlags::Resizable;
};

struct NativeWindowHandle {
    void* window_handle;
    void* display_handle; 
};

enum class WindowEventType : uint8 {
    CloseRequested,
    Minized,
    Resized
};

struct WindowEvent {
    WindowEventType type;
};

}  // namespace iinf::platform

