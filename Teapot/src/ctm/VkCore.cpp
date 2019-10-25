#include "ctm/VkCore.h"

#include <glfw/glfw3.h>

#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>

namespace
{
#ifdef _DEBUG
	VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT , 
													   VkDebugReportObjectTypeEXT objectType, 
													   uint64_t , size_t , int32_t , const char *,
													   const char *pMessage, 
													   void *)
	{
		std::cerr << "[vulkan] ObjectType: " << objectType << "\nMessage: " << pMessage << "\n" << std::endl;;
		return VK_FALSE;
	}
#endif

	void createInstance(ctm::VkCore &core)
	{
#ifdef _DEBUG
		const char *layers[] = { "VK_LAYER_LUNARG_standard_validation" };
#endif

		const char **glfwExtensions;
		uint32_t glfwExtensionCount = 0;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifdef _DEBUG
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

		VkInstanceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.enabledExtensionCount = extensions.size();
		create_info.ppEnabledExtensionNames = extensions.data();
#ifdef _DEBUG
		create_info.enabledLayerCount = 1;
		create_info.ppEnabledLayerNames = layers;
		if (vkCreateInstance(&create_info, core.allocator, &core.instance) != VK_SUCCESS)
			throw std::runtime_error("Failed to create instance");
#else
		if (vkCreateInstance(&create_info, allocator, &instance) != VK_SUCCESS)
			throw std::runtime_error("Failed to create instance");
#endif

#ifdef _DEBUG
		auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(core.instance, "vkCreateDebugReportCallbackEXT");

		VkDebugReportCallbackCreateInfoEXT debugReportInfo = {};
		debugReportInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debugReportInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		debugReportInfo.pfnCallback = debugReportCallback;
		debugReportInfo.pUserData = NULL;
		if (vkCreateDebugReportCallbackEXT(core.instance, &debugReportInfo, core.allocator, &core.debugReport) != VK_SUCCESS)
			throw std::runtime_error("Unable to create debug report");
#endif
	}

	void selectPhysicalDevice(ctm::VkCore &core)
	{
		uint32_t count;
		if (vkEnumeratePhysicalDevices(core.instance, &count, nullptr) != VK_SUCCESS)
			throw std::runtime_error("Enable to retreive physical device");
		std::vector<VkPhysicalDevice> physicalDevices(count);
		if (vkEnumeratePhysicalDevices(core.instance, &count, physicalDevices.data()) != VK_SUCCESS)
			throw std::runtime_error("Enable to retreive physical device");
		

		for (VkPhysicalDevice &physicalDevice : physicalDevices)
		{
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(physicalDevice, &properties);
			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				core.physicalDevice = physicalDevice;
				return ;
			}
		}

		throw std::runtime_error("Could not find a suitable physical device");
	}

	void createSurface(ctm::VkCore &core, GLFWwindow *win)
	{
		if (glfwCreateWindowSurface(core.instance, win, core.allocator, &core.surface) != VK_SUCCESS)
			throw std::runtime_error("Failed to create window surface");
	}

	void selectQueues(ctm::VkCore &core)
	{
		uint32_t count;
		vkGetPhysicalDeviceQueueFamilyProperties(core.physicalDevice, &count, NULL);
		std::vector<VkQueueFamilyProperties> queueProperties(count);
		vkGetPhysicalDeviceQueueFamilyProperties(core.physicalDevice, &count, queueProperties.data());
		for (uint32_t i = 0; i < count; i++)
		{
			VkBool32 isSupported;
			vkGetPhysicalDeviceSurfaceSupportKHR(core.physicalDevice, i, core.surface, &isSupported);
			if (queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				core.queue.presentIdx = i;
				core.queue.graphicsIdx = i;
				break;
			}
			if (isSupported)
				core.queue.presentIdx = i;
			if (queueProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
				core.queue.transferIdx = i;
		}
	}

	void createLogicalDevice(ctm::VkCore &core)
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { core.queue.presentIdx, core.queue.graphicsIdx, core.queue.transferIdx };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			if (queueFamily != ctm::VkCore::Queue::invalidIdx)
			{
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			}
		}

		const char *device_extensions[] = { "VK_KHR_swapchain" };

		VkDeviceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = queueCreateInfos.size();
		create_info.pQueueCreateInfos = queueCreateInfos.data();
		create_info.enabledExtensionCount = 1;
		create_info.ppEnabledExtensionNames = device_extensions;
		if (vkCreateDevice(core.physicalDevice, &create_info, core.allocator, &core.device) != VK_SUCCESS)
			throw std::runtime_error("Unable to create logical device");
	}

	void getQueues(ctm::VkCore &core)
	{
		if (core.queue.presentIdx != ctm::VkCore::Queue::invalidIdx)
			vkGetDeviceQueue(core.device, core.queue.presentIdx, 0, &core.queue.present);
		if (core.queue.graphicsIdx != ctm::VkCore::Queue::invalidIdx)
			vkGetDeviceQueue(core.device, core.queue.graphicsIdx, 0, &core.queue.graphics);
		if (core.queue.transferIdx != ctm::VkCore::Queue::invalidIdx)
			vkGetDeviceQueue(core.device, core.queue.transferIdx, 0, &core.queue.transfer);
	}

	VkSurfaceFormatKHR selectSurfaceFormat(ctm::VkCore &core)
	{
		uint32_t surfaceFormatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(core.physicalDevice, core.surface, &surfaceFormatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(core.physicalDevice, core.surface, &surfaceFormatCount, surfaceFormats.data()) != VK_SUCCESS)
			throw std::runtime_error("Unable to retreive available surface format.");

		VkSurfaceFormatKHR selectedSurfaceFormat = {};
		for (uint32_t i = 0; i < surfaceFormatCount; i++)
		{
			if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM
				&& surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				selectedSurfaceFormat = surfaceFormats[i];
				break;
			}
		}
		return (selectedSurfaceFormat);
	}

	VkPresentModeKHR selectPresentMode(ctm::VkCore &core)
	{
		VkPresentModeKHR best;

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(core.physicalDevice, core.surface, &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(core.physicalDevice, core.surface, &presentModeCount, presentModes.data());

		for (size_t i = 0; i < presentModeCount; i++)
		{
			if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				return presentModes[i];
			else if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
				best = presentModes[i];
		}
		return (best);
	}
}

void ctm::VkCore::init(VkCore &core, GLFWwindow *win)
{
	int width;
	int height;
	glfwGetWindowSize(win, &width, &height);

	createInstance(core);
	selectPhysicalDevice(core);
	createSurface(core, win);
	selectQueues(core);
	createLogicalDevice(core);
	getQueues(core);

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(core.physicalDevice, core.surface, &surfaceCapabilities);
	core.config.extent = surfaceCapabilities.currentExtent;

	core.config.surfaceFormat = selectSurfaceFormat(core);
	core.config.format = core.config.surfaceFormat.format;
	core.config.presentMode = selectPresentMode(core);
}

void ctm::VkCore::destroy(VkCore &core)
{
	vkDestroyDevice(core.device, core.allocator);
#ifdef _DEBUG
	auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(core.instance, "vkDestroyDebugReportCallbackEXT");
	vkDestroyDebugReportCallbackEXT(core.instance, core.debugReport, core.allocator);
#endif
	vkDestroySurfaceKHR(core.instance, core.surface, core.allocator);
	vkDestroyInstance(core.instance, core.allocator);
}