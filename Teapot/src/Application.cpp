#include "Application.h"

#include <stdexcept>

#include "Exception.h"
#include "Log.h"

#include "gui/AttributeEditorWidget.h"
#include "gui/SceneEditorWidget.h"
#include "Profiler.h"


namespace
{
	void glfwResizeCallback(GLFWwindow *win, int w, int h)
	{
		teapot::Application *app = reinterpret_cast<teapot::Application *>(glfwGetWindowUserPointer(win));
		app->resize(w, h);
	}
}

teapot::Application *teapot::Application::app = nullptr;

teapot::Application::Application()
{
	Application::app = this;
}

teapot::Application::~Application()
{
	if (isInitialized)
		destroy();
}

void teapot::Application::init()
{
	PROFILE_FUNCTION("Global");

	initGlfw();

	vulkan.init();
	command.init();

	gui.init();
	gui.addWidget<gui::SceneEditorWidget>();
	gui.addWidget<gui::AttributeEditorWidget>();

	sceneEditor.init();
	collection.init();
}

void teapot::Application::destroy()
{
	PROFILE_FUNCTION("Global");

	command.destroy();

	collection.destroy();
	sceneEditor.destroy();
	gui.destroy();
	vulkan.destroy();

	destroyGlfw();
}


int teapot::Application::run()
{
	while (!glfwWindowShouldClose(win))
	{
		PROFILE_FUNCTION("Global");

		glfwPollEvents();
		command.requestNextImage();
		// scene process
		gui.drawWidgets();
		render(command);
	}
	return (0);
}

void teapot::Application::resize(int width, int height)
{
	vulkan.recreateSwapchain(width, height);
	command.requestNextImage();
	gui.drawWidgets();
	render(command);
}

void teapot::Application::initGlfw()
{
	PROFILE_FUNCTION("Vulkan");

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	if (!(win = glfwCreateWindow(1280, 720, "Vulkan", nullptr, nullptr)))
		CRITICAL_EXCEPTION("Failed to create GLFW window");
	glfwSetFramebufferSizeCallback(win, glfwResizeCallback);
	glfwSetWindowUserPointer(win, this);
	glfwSetWindowSizeLimits(win, 600, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
}


void teapot::Application::destroyGlfw()
{
	PROFILE_FUNCTION("Vulkan");

	glfwDestroyWindow(win);
	glfwTerminate();
}

void teapot::Application::render(vk::Command &command)
{
	PROFILE_FUNCTION("Vulkan");

	VkCommandBuffer &commandBuffer = command.recordNextBuffer();
	sceneEditor.renderViews(commandBuffer);
	gui.render(commandBuffer);
	command.submitAndPresent();
}