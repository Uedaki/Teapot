#pragma once

#include <vulkan/vulkan.h>

#include <glfw/glfw3.h>

class VulkanWindow
{
public:
	VulkanWindow();

private:
	GLFWwindow *window;

	VkInstance instance;

	VkAllocationCallbacks *allocator = nullptr;

	void createInstance();
};