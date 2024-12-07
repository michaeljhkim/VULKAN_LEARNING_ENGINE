cmake_minimum_required(VERSION 3.20.0)
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
    src/vulkan.c
    lib/stb.cpp
    lib/vkbootstrap/VkBootstrap.cpp

    ${ALGORITHIMS_SOURCES}  # Add all gathered files here
    ${GRAPHICS_SOURCES}  # Add all gathered files here
    ${IO_SOURCES}  # Add all gathered files here
    ${PHYSICS_SOURCES}  # Add all gathered files here
    ${VULKAN_SOURCES}  # Add all gathered files here
)

# Find SDL2 package
find_package(SDL2 REQUIRED)
# Include SDL2 headers
include_directories(${SDL2_INCLUDE_DIRS})

find_package(assimp REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glslang REQUIRED)

# Link SDL2 library
target_link_libraries(yurrgoht_engine ${SDL2_LIBRARIES} assimp freetype Vulkan glslang)