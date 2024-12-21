#cmake_minimum_required(VERSION 3.20.0)
cmake_minimum_required(VERSION 3.15.0)

include(FetchContent)

# Declare SDL2
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.30.10
)

# Declare GLM
FetchContent_Declare(
    GLM
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)

# Declare Assimp
FetchContent_Declare(
    ASSIMP
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v5.4.3
)

# Declare Freetype 2
FetchContent_Declare(
    FREETYPE
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG        VER-2-13-3
)

# Declare KTX
FetchContent_Declare(
    KTX
    GIT_REPOSITORY https://github.com/KhronosGroup/KTX-Software
    GIT_TAG        v4.3.2
)

set(KTX2_ENABLE ON CACHE BOOL "Enable KTX2 support")

# Make libraries available
FetchContent_MakeAvailable(SDL2 GLM ASSIMP FREETYPE KTX)