cmake_minimum_required(VERSION 3.22)
project(Explorer3D VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SDL_SHARED OFF)
set(SDL_STATIC ON)
set(BUILD_SHARED_LIBS False)

set(SDLIMAGE_AVIF OFF)
set(SDLIMAGE_BMP ON)
set(SDLIMAGE_GIF OFF)
set(SDLIMAGE_JPG OFF)
set(SDLIMAGE_JXL OFF)
set(SDLIMAGE_LBM OFF)
set(SDLIMAGE_PCX OFF)
set(SDLIMAGE_PNG ON)
set(SDLIMAGE_PNM OFF)
set(SDLIMAGE_QOI OFF)
set(SDLIMAGE_SVG OFF)
set(SDLIMAGE_TGA OFF)
set(SDLIMAGE_TIF OFF)
set(SDLIMAGE_WEBP OFF)
set(SDLIMAGE_XCF OFF)
set(SDLIMAGE_XPM OFF)
set(SDLIMAGE_XV OFF)



add_subdirectory(SDL)
add_subdirectory(SDL_image)
add_executable(Explorer3D main.cxx)

target_include_directories(Explorer3D PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

target_link_libraries(Explorer3D 
    PUBLIC SDL3-static
    PUBLIC SDL3_image-static
    PUBLIC opengl32.lib)


