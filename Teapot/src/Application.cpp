#include "Application.h"

#include <stdexcept>

#include "Mesh.h"

namespace
{
	bool swapChainRebuild = false;

	void glfwResizeCallback(GLFWwindow *win, int w, int h)
	{
		swapChainRebuild = true;
	}
}

teapot::Application::Application()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	win = glfwCreateWindow(1280, 720, "Vulkan", nullptr, nullptr);

	ctm::VkCore::init(vCore, win);
	ImguiWrapper::init(imgui, win, vCore);

	isRunning = true;
}

teapot::Application::~Application()
{
	teapot::SceneView::destroy(scene, vCore);
	teapot::ImguiWrapper::destroy(imgui, vCore);
	ctm::VkCore::destroy(vCore);
	glfwDestroyWindow(win);
	glfwTerminate();
}

int teapot::Application::run()
{
	uint32_t i = 0;

	teapot::Mesh mesh(vCore);

	if (!isRunning)
		throw std::runtime_error("Application is not initialized");
	while (!glfwWindowShouldClose(win))
	{
		glfwPollEvents();

		if (swapChainRebuild)
		{
			swapChainRebuild = false;
			teapot::ImguiWrapper::rebuildSwapChain(imgui, win, vCore);
		}

		ImguiWrapper::newFrame(imgui);

		ImVec2 size = ImGui::GetIO().DisplaySize;
		size.x -= 400;
		if (scene.extent.width != size.x || scene.extent.height != size.y)
		{
			teapot::SceneView::init(scene, vCore, imgui.descriptorPool, mesh, { (uint32_t)size.x, (uint32_t)size.y }, 2);
		}

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(size);
		ImGui::Begin("SceneView", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::Image((void *)&scene.descriptorSets[i], ImGui::GetContentRegionAvail());
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(size.x, 0));
		ImGui::SetNextWindowSize(ImVec2(400, size.y));
		ImGui::Begin("Details panel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		static bool isDisplayed = true;
		if (ImGui::CollapsingHeader("Transform", nullptr, ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Location: ");
			ImGui::SameLine(100);
			ImGui::InputFloat3("##1", scene.location, "%.0f");
			ImGui::Text("Rotation: ");
			ImGui::SameLine(100);
			ImGui::InputFloat3("##2", scene.rotation, "%.0f");
			ImGui::Text("Scale: ");
			ImGui::SameLine(100);
			ImGui::InputFloat3("##3", scene.scale, "%.0f");
		}
		ImGui::End();

		teapot::SceneView::render(scene, vCore, i);
		ImguiWrapper::render(imgui, scene.signalSemaphores[i], vCore);
		i = (i + 1) % 2;
	}
	vkQueueWaitIdle(vCore.queue.present);
	return (0);
}