cmake_minimum_required(VERSION 2.6)

project(Teapot)
set(Teapot_PROJECT_NAME "Teapot")
set(Teapot_VERSION_MAJOR 0)
set(Teapot_VERSION_MINOR 1)

set(PROJECT_INCLUDE_DIR "include")
set(PROJECT_GEN_FILE_DIR "${PROJECT_BINARY_DIR}/generated")

configure_file("${PROJECT_INCLUDE_DIR}/Teapot.h.in" "${PROJECT_GEN_FILE_DIR}/Teapot.h")

include_directories("${PROJECT_INCLUDE_DIR}")
include_directories("${PROJECT_GEN_FILE_DIR}")

find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)

set(PROJECT_SRC src/main.cpp
				src/Application.cpp
				src/VulkanWindow.cpp)
set(PROJECT_INCLUDE ${PROJECT_INCLUDE_DIR}/Application.h
					${PROJECT_INCLUDE_DIR}/VulkanWindow.h)
set(PROJECT_GENERATED_INCLUDE ${PROJECT_GEN_FILE_DIR}/Teapot.h)

add_executable(Teapot ${PROJECT_SRC} ${PROJECT_INCLUDE} ${PROJECT_GENERATED_INCLUDE})

target_include_directories(Teapot
							PUBLIC ${OPENGL_INCLUDE_DIR}
							PUBLIC ${Vulkan_INCLUDE_DIRS})

target_link_libraries(Teapot
						${OPENGL_gl_LIBRARY}
						${Vulkan_LIBRARIES})