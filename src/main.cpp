#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

#include <iinf/platform/platform.h>
#include <iinf/platform/types.h>
#include <iinf/platform/window.h>

namespace platform = iinf::platform;

int main() {
    if (!platform::Init()) {
        return 1;
    }

    platform::WindowCreateInfo window_info = {
        .title = "IINF Renderer",
        .width = 1920,
        .height = 1080,
        .mode = platform::WindowMode::Windowed,
        .flags = platform::WindowFlags::MatchPixelDensity | platform::WindowFlags::Resizable
    };
    {
        auto window = platform::Window::Create(window_info);
        bool is_close_requested = false;
        while (!is_close_requested) {
            window->PollEvents([&](const platform::WindowEvent& event) {
                if (event.type == platform::WindowEventType::CloseRequested) {
                    is_close_requested = true;
                }
            });
        }
    }
    
    platform::Exit();
    return 0;
}
