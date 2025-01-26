cmake_minimum_required(VERSION 3.22)
project(Explorer3D VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(SDL_SHARED False)
set(SDL_STATIC True)
set(BUILD_SHARED_LIBS False)

add_subdirectory(SDL)
add_executable(Explorer3D main.cxx)

target_include_directories(Explorer3D PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

target_link_libraries(Explorer3D 
    PUBLIC SDL3-static
    PUBLIC opengl32.lib)


