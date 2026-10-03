cmake_minimum_required(VERSION 3.24)
project(InfinityDungeon LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # -std=c++20, not gnu++20, so code stays portable across compilers
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
include(CTest)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

# Game modules. Only render and input may link raylib; sim must stay raylib-free.
add_library(infinity_sim src/sim/DungeonSimulator.cpp src/sim/FixedTimestep.cpp src/sim/GameState.cpp)
add_library(infinity_input src/input/PlayerInput.cpp)
add_library(infinity_render src/render/Renderer.cpp)
add_library(infinity_net src/net/LeaderboardClient.cpp)

# Orchestration layer: ties sim, render and input together via a scene
# stack. Not simulation itself, so it may depend on raylib directly for
# scene drawing.
add_library(infinity_app src/app/SceneManager.cpp src/app/MenuScene.cpp src/app/PlayingScene.cpp)

foreach(module infinity_sim infinity_input infinity_render infinity_net infinity_app)
    target_include_directories(${module} PUBLIC ${PROJECT_SOURCE_DIR}/src)
    target_compile_features(${module} PUBLIC cxx_std_20)
endforeach()

target_link_libraries(infinity_input PUBLIC raylib)
target_link_libraries(infinity_render PUBLIC raylib)
target_link_libraries(infinity_app PUBLIC infinity_sim infinity_render infinity_input raylib)

add_executable(InfinityDungeon src/main.cpp)
target_compile_features(InfinityDungeon PRIVATE cxx_std_20)
target_link_libraries(InfinityDungeon PRIVATE infinity_app infinity_sim infinity_input infinity_render infinity_net raylib)

if(BUILD_TESTING)
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.12
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(doctest)

    add_executable(infinity_tests tests/test_main.cpp tests/sim_determinism_test.cpp tests/fixed_timestep_test.cpp tests/game_state_test.cpp tests/player_combat_test.cpp)
    target_compile_features(infinity_tests PRIVATE cxx_std_20)
    target_link_libraries(infinity_tests PRIVATE infinity_sim doctest::doctest)
    # Avoids doctest forward-declaring std types, which newer MSVC warns about (C5285).
    target_compile_definitions(infinity_tests PRIVATE DOCTEST_CONFIG_USE_STD_HEADERS)

    include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
    doctest_discover_tests(infinity_tests)
endif()
