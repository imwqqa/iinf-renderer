#pragma once

#include <functional>
#include <memory>

#include <iinf/core/types.h>
#include <iinf/platform/types.h>

namespace iinf::platform {

class Window {
public:
    virtual ~Window() = default;
    static std::unique_ptr<Window> Create(const WindowCreateInfo& info);
    virtual void PollEvents(std::function<void(const WindowEvent&)> callback) = 0;

    virtual Extent2D extent() const = 0;
    virtual void set_extent(Extent2D extent) = 0;
    virtual NativeWindowHandle native_handle() const = 0;
};

}  // namespace iinf::platform

