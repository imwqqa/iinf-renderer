add_library(iinf-core INTERFACE)

target_include_directories(iinf-core
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_subdirectory(platform)

add_executable(iinf-app main.cpp)
target_link_libraries(iinf-app
    PRIVATE
        iinf-core
        iinf-platform
        SDL3::SDL3
        imgui::imgui
        Vulkan::Loader
        vk-bootstrap::vk-bootstrap
        glm::glm
        GPUOpen::VulkanMemoryAllocator
        fastgltf::fastgltf
)

target_precompile_headers(iinf-app # for language server
    PRIVATE
        <SDL3/SDL.h>
        <imgui.h>
        <VkBootstrap.h>
        <glm/glm.hpp>
        <vk_mem_alloc.h>
        <fastgltf/core.hpp>
)
