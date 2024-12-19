#cmake_minimum_required(VERSION 3.20.0)
cmake_minimum_required(VERSION 3.15.0)

# Include the library builder to fetch dependencies
include(LibraryBuilder.cmake)

project(yurrgoht_engine)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# Include the 'include' directory for headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Automatically gather all .cpp files in src/
file(GLOB_RECURSE ALGORITHIMS_SOURCES ${CMAKE_SOURCE_DIR}/src/algorithms/*.cpp)
file(GLOB_RECURSE GRAPHICS_SOURCES ${CMAKE_SOURCE_DIR}/src/graphics/*.cpp)
file(GLOB_RECURSE IO_SOURCES ${CMAKE_SOURCE_DIR}/src/io/*.cpp)
file(GLOB_RECURSE PHYSICS_SOURCES ${CMAKE_SOURCE_DIR}/src/physics/*.cpp)
file(GLOB_RECURSE VULKAN_SOURCES ${CMAKE_SOURCE_DIR}/src/vulkan_core/*.cpp)

add_executable(yurrgoht_engine
    src/main.cpp
    src/scene.cpp
    #lib/stb.cpp

    ${ALGORITHIMS_SOURCES}
    ${GRAPHICS_SOURCES}
    ${IO_SOURCES}
    ${PHYSICS_SOURCES}
    ${VULKAN_SOURCES}
)

# Find Assimp, Vulkan, and other dependencies
find_package(Vulkan REQUIRED)

# Link the libraries
target_link_libraries(yurrgoht_engine
    #${SDL2_LIBRARIES}
    SDL2
    GLM
    ASSIMP  # Assimp target
    FREETYPE
    Vulkan
    glslang
)