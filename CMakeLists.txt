cmake_minimum_required(VERSION 3.25)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(panning_zooming)

add_executable(${CMAKE_PROJECT_NAME} main.cpp stb_image.cpp)
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Wshadow)
target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_20)

find_package(SDL2 REQUIRED)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
