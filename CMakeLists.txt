cmake_minimum_required(VERSION 3.6)

set (CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_FOLDER_NAME "a.out")
set(EXECUTABLE_OUTPUT_PATH "./${EXECUTABLE_FOLDER_NAME}")

project(Teapot)
set(Teapot_PROJECT_NAME "Teapot")
set(Teapot_VERSION_MAJOR 0)
set(Teapot_VERSION_MINOR 5)
set(Teapot_SOURCE_DIR "${PROJECT_SOURCE_DIR}/${Teapot_PROJECT_NAME}/src")
set(Teapot_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/${Teapot_PROJECT_NAME}/include")
set(Teapot_GEN_FILE_DIR "${PROJECT_BINARY_DIR}/generated")

configure_file("${Teapot_INCLUDE_DIR}/Teapot.h.in" "${Teapot_GEN_FILE_DIR}/Teapot.h")

include_directories("${Teapot_INCLUDE_DIR}")
include_directories("${Teapot_GEN_FILE_DIR}")
set(Teapot_SRC ${Teapot_SOURCE_DIR}/main.cpp
				${Teapot_SOURCE_DIR}/Application.cpp
				${Teapot_SOURCE_DIR}/Collection.cpp
				${Teapot_SOURCE_DIR}/ObjectPicker.cpp
				${Teapot_SOURCE_DIR}/Profiler.cpp)
set(Teapot_SRC_VULKAN ${Teapot_SOURCE_DIR}/vulkan/Context.cpp
						${Teapot_SOURCE_DIR}/vulkan/Command.cpp
						${Teapot_SOURCE_DIR}/vulkan/Pipeline.cpp
						${Teapot_SOURCE_DIR}/vulkan/Mesh.cpp
						${Teapot_SOURCE_DIR}/vulkan/Utils.cpp
						${Teapot_SOURCE_DIR}/vulkan/Vertex.cpp
						${Teapot_SOURCE_DIR}/vulkan/SceneEditor.cpp)
set(Teapot_SRC_GUI ${Teapot_SOURCE_DIR}/gui/Manager.cpp
					${Teapot_SOURCE_DIR}/gui/SceneEditorWidget.cpp
					${Teapot_SOURCE_DIR}/gui/AttributeEditorWidget.cpp)
					
					
set(Teapot_INCLUDE ${Teapot_INCLUDE_DIR}/Application.h
					${Teapot_INCLUDE_DIR}/Collection.h
					${Teapot_INCLUDE_DIR}/Profiler.h
					${Teapot_INCLUDE_DIR}/ObjectPicker.h
					${Teapot_INCLUDE_DIR}/DisplayMode.h
					${Teapot_INCLUDE_DIR}/Log.h
					${Teapot_INCLUDE_DIR}/Exception.h)
set(Teapot_INCLUDE_VULKAN ${Teapot_INCLUDE_DIR}/vulkan/Context.h
						${Teapot_INCLUDE_DIR}/vulkan/Command.h
						${Teapot_INCLUDE_DIR}/vulkan/Pipeline.h
						${Teapot_INCLUDE_DIR}/vulkan/Mesh.h
						${Teapot_INCLUDE_DIR}/vulkan/Utils.h
						${Teapot_INCLUDE_DIR}/vulkan/Vertex.h
						${Teapot_INCLUDE_DIR}/vulkan/SceneEditor.h)
set(Teapot_INCLUDE_GUI ${Teapot_INCLUDE_DIR}/gui/Manager.h
						${Teapot_INCLUDE_DIR}/gui/Widget.h
						${Teapot_INCLUDE_DIR}/gui/SceneEditorWidget.h
						${Teapot_INCLUDE_DIR}/gui/AttributeEditorWidget.h)
set(PROJECT_GENERATED_INCLUDE ${Teapot_GEN_FILE_DIR}/Teapot.h)

find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)

set(Glfw_INCLUDE_DIRS "" CACHE PATH "Define path to glfw include")
set(Glfw_LIBRARIES_DIRS "" CACHE PATH "Define path to glfw libraries")
set(Glfw_LIBRARIES ${Vulkan_LIBRARIES_DIRS}/glfw3dll.lib)

set(PROFILING "" CACHE BOOL "Define if profiling is activated or not")
if (PROFILING)
	add_definitions(-DPROFILING)
endif (PROFILING)

set(LOG_INFO ON CACHE BOOL "Define if the application will output log")
if (LOG_INFO)
	add_definitions(-DLOG_INFO)
endif (LOG_INFO)

set(LOG_DEBUG "" CACHE BOOL "Define if the application will output debug info")
if (LOG_DEBUG)
	add_definitions(-DLOG_DEBUG)
endif (LOG_DEBUG)

set(VULKAN_DEBUG "" CACHE BOOL "Define if vulkan debug layer are used or not")
if (VULKAN_DEBUG)
	add_definitions(-DVULKAN_DEBUG)
endif (VULKAN_DEBUG)

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
									${Teapot_SRC_VULKAN} ${Teapot_INCLUDE_VULKAN}
									${Teapot_SRC_GUI} ${Teapot_INCLUDE_GUI}
									${IMGUI_SRC} ${IMGUI_INCLUDE}
									
									${Teapot_SRC_TRASH} ${Teapot_INCLUDE_TRASH}
									${Teapot_CTM_SRC} ${Teapot_CTM_INCLUDE})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Teapot)

source_group("Sources" FILES ${Teapot_SRC})
source_group("Sources\\vulkan" FILES ${Teapot_SRC_VULKAN})
source_group("Sources\\gui" FILES ${Teapot_SRC_GUI})

source_group("Include" FILES ${Teapot_INCLUDE})
source_group("Include\\vulkan" FILES ${Teapot_INCLUDE_VULKAN})
source_group("Include\\gui" FILES ${Teapot_INCLUDE_GUI})
source_group("Include\\generated" FILES ${Teapot_GENERATED_INCLUDE})

source_group("lib\\imgui" FILES ${IMGUI_SRC})
source_group("lib\\imgui" FILES ${IMGUI_INCLUDE})

target_include_directories(Teapot
							PUBLIC ${OPENGL_INCLUDE_DIR}
							PUBLIC ${Vulkan_INCLUDE_DIRS}
							PUBLIC ${Glfw_INCLUDE_DIRS})

target_link_libraries(Teapot
						${OPENGL_gl_LIBRARY}
						${Vulkan_LIBRARIES}
						${Glfw_LIBRARIES})
						
add_dependencies(Teapot Shader)

set_property(TARGET Teapot 
				PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/${EXECUTABLE_FOLDER_NAME}")
	


set(SHADER_DIR "${PROJECT_SOURCE_DIR}/Teapot/shader")
set(SHADER_FILES ${SHADER_DIR}/rasterizer.vert
					${SHADER_DIR}/rasterizer.frag
					${SHADER_DIR}/rasterizerFace.frag
					${SHADER_DIR}/rasterizerPoint.frag
					${SHADER_DIR}/rasterizerLine.frag)

SET(SHADER_COMPILER "P:/VulkanSDK/1.1.108.0/Bin/glslc.exe")
add_custom_target(Shader 
	COMMAND ${CMAKE_COMMAND} -E make_directory "${EXECUTABLE_FOLDER_NAME}/shader"
	COMMAND cd "${EXECUTABLE_FOLDER_NAME}/shader"
	COMMAND glslc "-c" ${SHADER_FILES}
	SOURCES ${SHADER_FILES}
)
source_group("shader" FILES ${SHADER_FILES})