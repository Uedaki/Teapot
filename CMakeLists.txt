cmake_minimum_required(VERSION 2.6)

project(Teapot)
set(Teapot_PROJECT_NAME "Teapot")
set(Teapot_VERSION_MAJOR 0)
set(Teapot_VERSION_MINOR 1)

set(PROJECT_INCLUDE_DIR "include")
set(PROJECT_GEN_FILE_DIR "${PROJECT_BINARY_DIR}generated")

configure_file("${PROJECT_INCLUDE_DIR}/Teapot.h.in" "${PROJECT_GEN_FILE_DIR}/Teapot.h")

include_directories("${PROJECT_INCLUDE_DIR}")
include_directories("${PROJECT_GEN_FILE_DIR}")

add_executable(Teapot src/main.cpp)