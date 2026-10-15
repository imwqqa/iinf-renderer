cmake_minimum_required(VERSION 3.15)

project(iinf-renderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for language server

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-gdwarf-4)
endif()

find_package(SDL3 REQUIRED)
find_package(imgui REQUIRED)
find_package(VulkanLoader REQUIRED)
find_package(vk-bootstrap REQUIRED)
find_package(glm REQUIRED)
find_package(VulkanMemoryAllocator REQUIRED)
find_package(fastgltf REQUIRED)

add_subdirectory(src)
add_subdirectory(glsl)
