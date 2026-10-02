#include <functional>
#include <memory>
#include <optional>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

#include <iinf/core/types.h>
#include <iinf/platform/types.h>
#include <iinf/platform/window.h>

namespace iinf::platform {
namespace {

std::optional<WindowEvent> ConvertEvent(SDL_Event sdl_event) {
    switch (sdl_event.type) {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            return WindowEvent { .type = WindowEventType::CloseRequested };
        default:
            return std::nullopt;
    }
}

class SDLWindow final : public Window {
public: 
    explicit SDLWindow(SDL_Window* handle) : handle_(handle) {}

    ~SDLWindow() override {
        SDL_DestroyWindow(handle_);
    }

    void PollEvents(std::function<void(const WindowEvent&)> callback) override {
        SDL_Event sdl_event;
        while (SDL_PollEvent(&sdl_event)) {
            if (auto event = ConvertEvent(sdl_event)) {
                callback(*event);
            }
        }
    }

    Extent2D extent() const override {
        int width = 0;
        int height = 0;
        SDL_GetWindowSize(handle_, &width, &height);
        return {
            .width = static_cast<uint32>(width),
            .height = static_cast<uint32>(height)
        };
    }

    void set_extent(Extent2D extent) override {
        SDL_SetWindowSize(handle_, static_cast<int>(extent.width), static_cast<int>(extent.height));
    }

    NativeWindowHandle native_handle() const override {
        return native_handle_;
    }

private:
    SDL_Window* handle_;
    NativeWindowHandle native_handle_;
};

} // namespace

std::unique_ptr<Window> Window::Create(const WindowCreateInfo& info) {
    uint32 sdl_flags = 0;

    if (info.mode == WindowMode::Fullscreen) {
        sdl_flags |= SDL_WINDOW_FULLSCREEN;
    }
    if (info.mode == WindowMode::Borderless){
        sdl_flags |= SDL_WINDOW_BORDERLESS;
    }
    if ((info.flags & WindowFlags::Resizable) != WindowFlags::None) {
        sdl_flags |= SDL_WINDOW_RESIZABLE;
    }
    if ((info.flags & WindowFlags::MatchPixelDensity) != WindowFlags::None) {
        sdl_flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
    }   

    SDL_Window* sdl_window = SDL_CreateWindow(info.title.c_str(), static_cast<int>(info.width), static_cast<int>(info.height), sdl_flags);

    return std::make_unique<SDLWindow>(sdl_window);
}

} // namespace iinf::platform
