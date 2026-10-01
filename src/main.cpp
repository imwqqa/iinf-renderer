// entry point

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

int main() {
    SDL_Window* window;
    bool done = false;

    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("IINF Renderer", 1920, 1080, SDL_WINDOW_HIGH_PIXEL_DENSITY);

    while (!done) {
        SDL_Event event; 
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                done = true;
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}
