#pragma once

#include <glfw/glfw3.h>

#include "imgui.h"
#include "imgui_impl_vulkan.h"

class VulkanContext;

class WrapperImgui
{
public:
	void init(GLFWwindow *win, VulkanContext &vulkan);
	void newFrame();
	void render(VulkanContext &vulkan);
	void destroy();

//private:
	ImGui_ImplVulkanH_Window wd = {};

	void uploadFont(VulkanContext &vulkan);
};