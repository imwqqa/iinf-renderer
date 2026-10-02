#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

namespace iinf::platform {
    
bool Init() {
    return SDL_Init(SDL_INIT_VIDEO);
}

void Quit() {
    SDL_Quit();
}

} // namespace iinf::platform
