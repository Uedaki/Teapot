#include "Application.h"

#include <stdexcept>

#include "ImguiWrapper.h"
#include "SceneView.h"

teapot::Application::Application()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	win = glfwCreateWindow(700, 500, "Vulkan", nullptr, nullptr);

	ctm::VkCore::init(vCore, win);
	ImguiWrapper::init(imgui, win, vCore);

	isRunning = true;
}

teapot::Application::~Application()
{
	teapot::ImguiWrapper::destroy(imgui, vCore);
	ctm::VkCore::destroy(vCore);
	glfwDestroyWindow(win);
	glfwTerminate();
}

int teapot::Application::run()
{
	teapot::SceneView rast;
	uint32_t i = 0;

	if (!isRunning)
		throw std::runtime_error("Application is not initialized");
	while (!glfwWindowShouldClose(win))
	{
		glfwPollEvents();
		ImguiWrapper::newFrame(imgui);
		
		ImVec2 size = ImGui::GetIO().DisplaySize;
		if (rast.extent.width != size.x || rast.extent.height != size.y)
		{
			teapot::SceneView::init(rast, vCore, imgui.descriptorPool, { (uint32_t)size.x, (uint32_t)size.y }, 2);
		}

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(size);
		ImGui::Begin("SceneView", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize);
		ImGui::Image((void *)&rast.descriptorSets[i], ImGui::GetContentRegionAvail());
		ImGui::End();

		teapot::SceneView::render(rast, vCore, i);
		ImguiWrapper::render(imgui, rast.signalSemaphores[i], vCore);
		i = (i + 1) % 2;
	}
	vkQueueWaitIdle(vCore.queue.present);
	teapot::SceneView::destroy(rast, vCore);
	return (0);
}