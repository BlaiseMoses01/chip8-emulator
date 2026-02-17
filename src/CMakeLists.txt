cmake_minimum_required(VERSION 3.16)
project(chip8_sdl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

add_executable(chip8
    main.cpp
    graphics.cpp
    chip8_emulator.cpp
    debugger.cpp
)

target_include_directories(chip8 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
find_package(SDL2_ttf REQUIRED)
target_link_libraries(chip8 PRIVATE SDL2_ttf::SDL2_ttf)


if (TARGET SDL2::SDL2)
    target_link_libraries(chip8 PRIVATE SDL2::SDL2)
    if (TARGET SDL2::SDL2main)
        target_link_libraries(chip8 PRIVATE SDL2::SDL2main)
    endif()
else()
    target_include_directories(chip8 PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(chip8 PRIVATE ${SDL2_LIBRARIES})
endif()
