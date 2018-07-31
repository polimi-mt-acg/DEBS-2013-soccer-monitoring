# ==-----------------------------------------------------------------------== #
#                              CMake Preamble
# ==-----------------------------------------------------------------------== #
cmake_minimum_required(VERSION 3.0.0)
project(soccer-monitoring VERSION 0.1.0)

# Enable unit testing support
include(CTest)
enable_testing()

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==-----------------------------------------------------------------------== #
#                       DEBS 2013 - Soccer Monitoring
# ==-----------------------------------------------------------------------== #
# OpenMP dependency
find_package(OpenMP)

# Fmtlib dependency
add_subdirectory(external/fmt EXCLUDE_FROM_ALL)

# Boost
find_package(Boost REQUIRED COMPONENTS program_options)

set(SOCCER_MONITORING_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/soccer_monitoring.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/event.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/event_fetcher.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/metadata.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/position.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/visualizer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/game_statistics.cpp)

add_executable(soccer-monitoring
        ${SOCCER_MONITORING_SRC}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

target_include_directories(soccer-monitoring
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/external
        )

if (OPENMP_FOUND)
    target_compile_options(soccer-monitoring PUBLIC -fopenmp)
endif (OPENMP_FOUND)

target_link_libraries(soccer-monitoring
        PUBLIC
            omp
            fmt-header-only
            stdc++fs
            Boost::program_options
        )

# ==-----------------------------------------------------------------------== #
#                               Unit tests
# ==-----------------------------------------------------------------------== #
# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/catch)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

set(TEST_SOURCES
        ${SOCCER_MONITORING_SRC}
        ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/test_metadata.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/test_event_fetcher.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/test_distance.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/test_game_statistics.cpp test/test_visualizer.cpp)

add_executable(tests ${TEST_SOURCES})

target_include_directories(tests
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/test
            ${CMAKE_CURRENT_SOURCE_DIR}/external
        )

target_compile_options(tests PUBLIC -fopenmp)

target_link_libraries(tests
        PUBLIC
            Catch
            fmt-header-only
            stdc++fs
            omp
        )

target_compile_definitions(tests
        PUBLIC
            FULL_GAME="${CMAKE_CURRENT_SOURCE_DIR}/datasets/preprocessed/full-game"
            GAME_DATA_START_10_10000="${CMAKE_CURRENT_SOURCE_DIR}/test/resources/game_data_start_10_10000"
            GAME_DATA_START_10_100000="${CMAKE_CURRENT_SOURCE_DIR}/test/resources/game_data_start_10_100000"
            GAME_DATA_START_10_1e7="${CMAKE_CURRENT_SOURCE_DIR}/test/resources/game_data_start_10_1e7")

add_test(NAME tests COMMAND tests)
