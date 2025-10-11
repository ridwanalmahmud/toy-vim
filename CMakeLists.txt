cmake_minimum_required(VERSION 3.15)
project(shire)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -pedantic -pipe)
add_compile_options("$<$<CONFIG:Debug>:-g3;-Og>")
add_compile_options("$<$<CONFIG:Release>:-D_FORTIFY_SOURCE=2;-D_GNU_SOURCE;-fstack-protector-strong;-fPIE;-fPIC>")
add_compile_options("$<$<CONFIG:Release>:-O3;-ffast-math;-march=native>")
add_link_options("$<$<CONFIG:Release>:-Wl,-z,relro>")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
    src/main.c
    src/modes.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ncurses)

add_custom_target(run
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    USES_TERMINAL
)
