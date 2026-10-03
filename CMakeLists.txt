cmake_minimum_required(VERSION 3.20)
project(InfinityDungeon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(infinity_sim src/sim/DungeonSimulator.cpp)
add_library(infinity_input src/input/PlayerInput.cpp)
add_library(infinity_render src/render/Renderer.cpp)
add_library(infinity_net src/net/LeaderboardClient.cpp)

add_executable(infinity_dungeon src/main.cpp)
target_link_libraries(infinity_dungeon PRIVATE infinity_sim infinity_input infinity_render infinity_net)

enable_testing()
add_executable(sim_determinism_test tests/sim_determinism_test.cpp)
target_link_libraries(sim_determinism_test PRIVATE infinity_sim)
add_test(NAME sim_determinism_test COMMAND sim_determinism_test)
