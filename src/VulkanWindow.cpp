#include "VulkanWindow.h"

#include <stdexcept>

VulkanWindow::VulkanWindow()
{
	if (glfwInit() != GL_TRUE)
		throw std::runtime_error("Failed to initialized glfw");
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	if (!(window = glfwCreateWindow(700, 500, "Teapot", nullptr, nullptr)))
		throw std::runtime_error("Failed to create GLFWwindow");

	createInstance();
}

void VulkanWindow::createInstance()
{
	uint32_t extensionsCount = 0;
	const char **extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);

	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.enabledExtensionCount = extensionsCount;
	instanceInfo.ppEnabledExtensionNames = extensions;
	if (vkCreateInstance(&instanceInfo, allocator, &instance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create VkInstance");
}

