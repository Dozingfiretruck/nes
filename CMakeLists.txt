cmake_minimum_required(VERSION 3.10)

project(nes)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

file(GLOB SRCS
        src/*.c
        src/nes_mapper/*.c
        port/*.c
        main.c
        )
list(APPEND INCS 
        inc
        port
        )
include_directories(${INCS})

add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
