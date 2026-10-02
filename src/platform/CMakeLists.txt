add_library(iinf-platform
    sdl/platform.cpp 
    sdl/window.cpp 
)

target_link_libraries(iinf-platform
    PUBLIC
        iinf-core
    PRIVATE
        SDL3::SDL3
)

