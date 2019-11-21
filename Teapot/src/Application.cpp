#include "Application.h"

#include <stdexcept>

#include "Mesh.h"

namespace
{
	void glfwResizeCallback(GLFWwindow *win, int w, int h)
	{
		teapot::Application *app = reinterpret_cast<teapot::Application *>(glfwGetWindowUserPointer(win));
		app->resize(w, h);
	}
}

teapot::Application::Application()
	: scene(vCore)
	, mesh(vCore)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	win = glfwCreateWindow(1280, 720, "Vulkan", nullptr, nullptr);
	glfwSetFramebufferSizeCallback(win, glfwResizeCallback);
	glfwSetWindowUserPointer(win, this);
	glfwSetWindowSizeLimits(win, 600, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);

	ctm::VkCore::init(vCore, win);
	ImguiWrapper::init(imgui, win, vCore);
	mesh.init(2);

	resize();
	isRunning = true;
}

teapot::Application::~Application()
{
	mesh.destroy();
	scene.destroy();
	teapot::ImguiWrapper::destroy(imgui, vCore);
	ctm::VkCore::destroy(vCore);
	glfwDestroyWindow(win);
	glfwTerminate();
}

int teapot::Application::run()
{
	if (!isRunning)
		throw std::runtime_error("Application is not initialized");
	while (!glfwWindowShouldClose(win))
	{
		glfwPollEvents();
		ImguiWrapper::newFrame(imgui);
		display();
	}
	vkQueueWaitIdle(vCore.queue.present);
	return (0);
}

void teapot::Application::display()
{
	scene.render();

	ImVec2 size = ImGui::GetIO().DisplaySize;
	size.x -= 400;

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(size);
	ImGui::Begin("SceneView", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::Image((void *)&scene.getOutputDescriptorSet(), ImGui::GetContentRegionAvail());
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
		ImGui::InputFloat3("##1", &location[0], "%.0f");
		ImGui::Text("Rotation: ");
		ImGui::SameLine(100);
		ImGui::InputFloat3("##2", &rotation[0], "%.0f");
		ImGui::Text("Scale: ");
		ImGui::SameLine(100);
		ImGui::InputFloat3("##3", &scale[0], "%.0f");

		mesh.updateTransform(location, rotation, scale);
	}
	ImGui::End();

	ImguiWrapper::render(imgui, scene.getCurrentSignalSemaphore(), vCore);
}

void teapot::Application::resize(int width, int height)
{
	if (width != 0 && height != 0)
		teapot::ImguiWrapper::rebuildSwapChain(imgui, vCore, width, height);
	
	ImguiWrapper::newFrame(imgui);

	ImVec2 size = ImGui::GetIO().DisplaySize;
	size.x -= 400;
	if (scene.needToBeResized((uint32_t)size.x, (uint32_t)size.y))
	{
		scene.init(mesh, imgui.descriptorPool, (uint32_t)size.x, (uint32_t)size.y, 2);
	}
	display();
}