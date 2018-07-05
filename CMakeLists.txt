# ==-----------------------------------------------------------------------== #
#                              CMake Preamble
# ==-----------------------------------------------------------------------== #
cmake_minimum_required(VERSION 3.0.0)
project(soccer-monitoring VERSION 0.1.0)

# Enable unit testing support 
include(CTest)
enable_testing()

# ==-----------------------------------------------------------------------== #
#                       DEBS 2013 - Soccer Monitoring
# ==-----------------------------------------------------------------------== #
set(SOCCER_MONITORING_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

add_executable(soccer-monitoring ${SOCCER_MONITORING_SRC})

target_include_directories(soccer-monitoring
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

# OpenMP dependency
find_package(OpenMP)
if (OPENMP_FOUND)
        target_compile_options(soccer-monitoring PUBLIC -fopenmp)
endif(OPENMP_FOUND)
target_link_libraries(soccer-monitoring omp)

# ==-----------------------------------------------------------------------== #
#                               Unit tests
# ==-----------------------------------------------------------------------== #
# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/catch)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

set(TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/hello.cpp)

add_executable(tests ${TEST_SOURCES})

target_include_directories(tests
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/test
        ${CMAKE_CURRENT_SOURCE_DIR}/external
    )

target_link_libraries(tests
    PUBLIC
        Catch
    )

add_test(NAME tests COMMAND tests)
