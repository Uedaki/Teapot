cmake_minimum_required(VERSION 2.6)

project(Teapot)
set(Teapot_PROJECT_NAME "Teapot")
set(Teapot_VERSION_MAJOR 0)
set(Teapot_VERSION_MINOR 1)

set(PROJECT_INCLUDE_DIR "Teapot/include")
set(PROJECT_GEN_FILE_DIR "${PROJECT_BINARY_DIR}/generated")

set (CMAKE_CXX_STANDARD 17)

configure_file("${PROJECT_INCLUDE_DIR}/Teapot.h.in" "${PROJECT_GEN_FILE_DIR}/Teapot.h")

find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)

set(PROJECT_SRC Teapot/src/main.cpp
				Teapot/src/Application.cpp
				Teapot/src/WindowManager.cpp
				Teapot/src/WrapperImgui.cpp
				Teapot/src/VulkanContext.cpp)
set(PROJECT_INCLUDE ${PROJECT_INCLUDE_DIR}/Application.h
					${PROJECT_INCLUDE_DIR}/WindowManager.h
					${PROJECT_INCLUDE_DIR}/WrapperImgui.h
					${PROJECT_INCLUDE_DIR}/VulkanContext.h)
set(PROJECT_GENERATED_INCLUDE ${PROJECT_GEN_FILE_DIR}/Teapot.h)
include_directories("${PROJECT_INCLUDE_DIR}")
include_directories("${PROJECT_GEN_FILE_DIR}")

set(IMGUI_DIR "modules/imgui")
set(IMGUI_SRC ${IMGUI_DIR}/imgui.cpp
				${IMGUI_DIR}/imgui_demo.cpp
				${IMGUI_DIR}/imgui_draw.cpp
				${IMGUI_DIR}/imgui_widgets.cpp
				${IMGUI_DIR}/examples/imgui_impl_glfw.cpp
				${IMGUI_DIR}/examples/imgui_impl_vulkan.cpp)
set(IMGUI_INCLUDE ${IMGUI_DIR}/imconfig.h
					${IMGUI_DIR}/imgui.h
					${IMGUI_DIR}/imgui_internal.h
					${IMGUI_DIR}/imstb_rectpack.h
					${IMGUI_DIR}/imstb_textedit.h
					${IMGUI_DIR}/imstb_truetype.h
					${IMGUI_DIR}/examples/imgui_impl_glfw.h
					${IMGUI_DIR}/examples/imgui_impl_vulkan.h)
include_directories("${IMGUI_DIR}")
include_directories("${IMGUI_DIR}/examples")

add_executable(Teapot ${PROJECT_SRC} ${PROJECT_INCLUDE} ${PROJECT_GENERATED_INCLUDE}
										${IMGUI_SRC} ${IMGUI_INCLUDE})

source_group("imgui" FILES ${IMGUI_SRC} ${IMGUI_INCLUDE})

target_include_directories(Teapot
							PUBLIC ${OPENGL_INCLUDE_DIR}
							PUBLIC ${Vulkan_INCLUDE_DIRS})

target_link_libraries(Teapot
						${OPENGL_gl_LIBRARY}
						${Vulkan_LIBRARIES})