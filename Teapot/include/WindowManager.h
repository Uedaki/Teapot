#pragma once

#include <glfw/glfw3.h>

#include "VulkanContext.h"
#include "WrapperImgui.h"

class WindowManager
{
public:
	void runWindow();

private:
	GLFWwindow *win;

	VulkanContext vulkan;
	WrapperImgui imgui;

	void initGlfw();
	void destroyGlfw();
};