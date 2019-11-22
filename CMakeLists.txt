cmake_minimum_required(VERSION 3.6)

set (CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_FOLDER_NAME "a.out")
set(EXECUTABLE_OUTPUT_PATH "./${EXECUTABLE_FOLDER_NAME}")

project(Teapot)
set(Teapot_PROJECT_NAME "Teapot")
set(Teapot_VERSION_MAJOR 0)
set(Teapot_VERSION_MINOR 2)
set(Teapot_SOURCE_DIR "${PROJECT_SOURCE_DIR}/${Teapot_PROJECT_NAME}/src")
set(Teapot_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/${Teapot_PROJECT_NAME}/include")
set(Teapot_GEN_FILE_DIR "${PROJECT_BINARY_DIR}/generated")

configure_file("${Teapot_INCLUDE_DIR}/Teapot.h.in" "${Teapot_GEN_FILE_DIR}/Teapot.h")

include_directories("${Teapot_INCLUDE_DIR}")
include_directories("${Teapot_GEN_FILE_DIR}")
set(Teapot_SRC ${Teapot_SOURCE_DIR}/main.cpp
				${Teapot_SOURCE_DIR}/Application.cpp
				${Teapot_SOURCE_DIR}/ImguiWrapper.cpp
				${Teapot_SOURCE_DIR}/SceneView.cpp
				${Teapot_SOURCE_DIR}/Mesh.cpp
				${Teapot_SOURCE_DIR}/Rasterizer.cpp)
set(Teapot_CTM_SRC ${Teapot_SOURCE_DIR}/ctm/VkCore.cpp
					${Teapot_SOURCE_DIR}/ctm/VkVertex.cpp
					${Teapot_SOURCE_DIR}/ctm/VkUtils.cpp
					${Teapot_SOURCE_DIR}/ctm/VkMemoryBlock.cpp)
set(Teapot_INCLUDE ${Teapot_INCLUDE_DIR}/Application.h
					${Teapot_INCLUDE_DIR}/ImguiWrapper.h
					${Teapot_INCLUDE_DIR}/SceneView.h
					${Teapot_INCLUDE_DIR}/Rasterizer.h
					${Teapot_INCLUDE_DIR}/Mesh.h)
set(Teapot_CTM_INCLUDE ${Teapot_INCLUDE_DIR}/ctm/VkCore.h
						${Teapot_INCLUDE_DIR}/ctm/VkVertex.h
						${Teapot_INCLUDE_DIR}/ctm/VkUtils.h
						${Teapot_INCLUDE_DIR}/ctm/VkMemoryBlock.h)
set(PROJECT_GENERATED_INCLUDE ${Teapot_GEN_FILE_DIR}/Teapot.h)

find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)

set(PROFILING "" CACHE BOOL "Define if profiling is activated or not")
if (PROFILING)
	add_definitions(-DPROFILING)
endif (PROFILING)

set(GLM_INCLUDE_DIR "" CACHE PATH "Path to the glm include files")
include_directories(${GLM_INCLUDE_DIR})

set(IMGUI_DIR "modules/imgui")
set(IMGUI_SRC ${IMGUI_DIR}/imgui.cpp
				${IMGUI_DIR}/imgui_demo.cpp
				${IMGUI_DIR}/imgui_draw.cpp
				${IMGUI_DIR}/imgui_widgets.cpp
				${IMGUI_DIR}/examples/imgui_impl_glfw.cpp
				${Teapot_SOURCE_DIR}/imgui/ctm_imgui_impl_vulkan.cpp)
set(IMGUI_INCLUDE ${IMGUI_DIR}/imconfig.h
					${IMGUI_DIR}/imgui.h
					${IMGUI_DIR}/imgui_internal.h
					${IMGUI_DIR}/imstb_rectpack.h
					${IMGUI_DIR}/imstb_textedit.h
					${IMGUI_DIR}/imstb_truetype.h
					${IMGUI_DIR}/examples/imgui_impl_glfw.h
					${Teapot_INCLUDE_DIR}/imgui/ctm_imgui_impl_vulkan.h)
include_directories("${IMGUI_DIR}")
include_directories("${IMGUI_DIR}/examples")



add_executable(Teapot ${Teapot_SRC} ${Teapot_INCLUDE} ${Teapot_GENERATED_INCLUDE}
									${Teapot_CTM_SRC} ${Teapot_CTM_INCLUDE}
										${IMGUI_SRC} ${IMGUI_INCLUDE})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Teapot)

source_group("Sources" FILES ${Teapot_SRC})
source_group("Sources\\ctm" FILES ${Teapot_CTM_SRC})
source_group("Sources\\imgui" FILES ${IMGUI_SRC})

source_group("Include" FILES ${Teapot_INCLUDE})
source_group("Include\\ctm" FILES ${Teapot_CTM_INCLUDE})
source_group("Include\\generated" FILES ${Teapot_GENERATED_INCLUDE})
source_group("Include\\imgui" FILES ${IMGUI_INCLUDE})

target_include_directories(Teapot
							PUBLIC ${OPENGL_INCLUDE_DIR}
							PUBLIC ${Vulkan_INCLUDE_DIRS})

target_link_libraries(Teapot
						${OPENGL_gl_LIBRARY}
						${Vulkan_LIBRARIES})
						
add_dependencies(Teapot Shader)

set_property(TARGET Teapot 
				PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/${EXECUTABLE_FOLDER_NAME}")
	


set(SHADER_DIR "${PROJECT_SOURCE_DIR}/Teapot/shader")
set(SHADER_FILES ${SHADER_DIR}/rasterizer.vert
					${SHADER_DIR}/rasterizer.frag)

SET(SHADER_COMPILER "P:/VulkanSDK/1.1.108.0/Bin/glslc.exe")
add_custom_target(Shader 
	COMMAND ${CMAKE_COMMAND} -E make_directory "${EXECUTABLE_FOLDER_NAME}/shader"
	COMMAND cd "${EXECUTABLE_FOLDER_NAME}/shader"
	COMMAND glslc "-c" ${SHADER_FILES}
	SOURCES ${SHADER_FILES}
)
source_group("shader" FILES ${SHADER_FILES})