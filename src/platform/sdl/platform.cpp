#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

#include <iinf/platform/platform.h>

namespace iinf::platform {
    
bool Init() {
    return SDL_Init(SDL_INIT_VIDEO);
}

void Exit() {
    SDL_Quit();
}

} // namespace iinf::platform
