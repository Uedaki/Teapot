#include "Application.h"

#include <algorithm>
#include <iostream>
#include <set>
#include <stdexcept>

#include "DebugUtilsMessenger.h"

namespace
{
	void framebufferResizeCallback(GLFWwindow *window, int width, int height)
	{
		auto app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
		app->windowHasBeenResized();
	}
}

Application::Application(int width, int height, const std::string &title)
	: width(width)
	, height(height)
	, title(title)
{

}

void Application::init()
{
	initGlfwWindow();
	initVulkan();
}

void Application::destroy()
{
	destroyGlfwWindow();

	
#ifdef _DEBUG
	auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	vkDestroyDebugReportCallbackEXT(instance, debugReport, allocator);
#endif
	vkDestroyDevice(device, allocator);
	vkDestroyInstance(instance, allocator);
}

void Application::windowHasBeenResized()
{
	hasBeenResized = true;
}

void Application::initGlfwWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	if (!(window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr)))
		throw std::runtime_error("Unable to create window");
	//glfwSetWindowUserPointer(window, this);
	//glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Application::destroyGlfwWindow()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool Application::isRunning()
{
	return (!glfwWindowShouldClose(window));
}

void Application::initVulkan()
{
	createInstance();
	selectPhysicalDevice();
	selectGraphicsQueue();
	createLogicalDevice();
	createSurface();
	createSwapchain();
	//createImageViews();
	//createRenderPass();
	//createGraphicsPipeline();
	//createFrameBuffers();
	//createCommandPool();
	//createCommandBuffers();
	//createSyncObjects();
}

void Application::createInstance()
{
	VkInstanceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

	auto extensionsRequired = getRequiredExtensions();
	create_info.enabledExtensionCount = extensionsRequired.size();
	create_info.ppEnabledExtensionNames = extensionsRequired.data();

#ifdef _DEBUG
	VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objectType,
		uint64_t object,
		size_t location,
		int32_t messageCode,
		const char *pLayerPrefix,
		const char *pMessage,
		void *pUserData);

	// Enabling multiple validation layers grouped as LunarG standard validation
	const char *layers[] = { "VK_LAYER_LUNARG_standard_validation" };
	create_info.enabledLayerCount = 1;
	create_info.ppEnabledLayerNames = layers;

	// Enable debug report extension (we need additional storage, so we duplicate the user array to add our new extension to it)
	const char **extensions_ext = (const char **)malloc(sizeof(const char *) * (extensionsRequired.size() + 1));
	memcpy(extensions_ext, extensionsRequired.data(), extensionsRequired.size() * sizeof(const char *));
	extensions_ext[extensionsRequired.size()] = "VK_EXT_debug_report";
	create_info.enabledExtensionCount = extensionsRequired.size() + 1;
	create_info.ppEnabledExtensionNames = extensions_ext;

	if (vkCreateInstance(&create_info, allocator, &instance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create instance");
	free(extensions_ext);

	// Get the function pointer (required for any extensions)
	auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

	// Setup the debug report callback
	VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
	debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	debug_report_ci.pfnCallback = debug_report;
	debug_report_ci.pUserData = NULL;
	if (vkCreateDebugReportCallbackEXT(instance, &debug_report_ci, allocator, &debugReport) != VK_SUCCESS)
		throw std::runtime_error("Unable to create debug report");
#else
	if (vkCreateInstance(&create_info, allocator, &instance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create instance");
#endif
}

void Application::selectPhysicalDevice()
{	
	uint32_t gpu_count;
	if (vkEnumeratePhysicalDevices(instance, &gpu_count, NULL) != VK_SUCCESS)
		throw std::runtime_error("Enable to retreive physical device");

	VkPhysicalDevice *gpus = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * gpu_count);
	if (vkEnumeratePhysicalDevices(instance, &gpu_count, gpus) != VK_SUCCESS)
		throw std::runtime_error("Enable to retreive physical device");

	// If a number >1 of GPUs got reported, you should find the best fit GPU for your purpose
	// e.g. VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU if available, or with the greatest memory available, etc.
	// for sake of simplicity we'll just take the first one, assuming it has a graphics queue family.
	physicalDevice = gpus[0];
	free(gpus);
}

void Application::selectGraphicsQueue()
{
	uint32_t count;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, NULL);
	VkQueueFamilyProperties *queues = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * count);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queues);
	for (uint32_t i = 0; i < count; i++)
		if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsQueueIdx = i;
			break;
		}
	free(queues);
}

void Application::createLogicalDevice()
{
	int device_extension_count = 1;
	const char *device_extensions[] = { "VK_KHR_swapchain" };
	const float queue_priority[] = { 1.0f };
	VkDeviceQueueCreateInfo queue_info[1] = {};
	queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info[0].queueFamilyIndex = graphicsQueueIdx;
	queue_info[0].queueCount = 1;
	queue_info[0].pQueuePriorities = queue_priority;
	VkDeviceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
	create_info.pQueueCreateInfos = queue_info;
	create_info.enabledExtensionCount = device_extension_count;
	create_info.ppEnabledExtensionNames = device_extensions;
	if (vkCreateDevice(physicalDevice, &create_info, allocator, &device) != VK_SUCCESS)
		throw std::runtime_error("Unable to create logical device");
	vkGetDeviceQueue(device, graphicsQueueIdx, 0, &graphicsQueue);
}

void Application::createSurface()
{
	//VkWin32SurfaceCreateInfoKHR createInfo = {};
	//createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	//createInfo.hwnd = glfwGetWin32Window(window);
	//createInfo.hinstance = GetModuleHandle(nullptr);

	//if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS)
	//	throw std::runtime_error("Failed to create window surface");

	if (glfwCreateWindowSurface(instance, window, allocator, &surface) != VK_SUCCESS)
		throw std::runtime_error("Failed to create window surface");
}

void Application::createImageViews()
{
	swapchainImageViews.resize(swapchainImages.size());

	for (size_t i = 0; i < swapchainImages.size(); i++)
	{
		VkImageViewCreateInfo imageViewInfo = {};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.image = swapchainImages[i];
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = swapchainImageFormat;
		imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &imageViewInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create image view");
	}
}

void Application::createRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachementRef = {};
	colorAttachementRef.attachment = 0;
	colorAttachementRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachementRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		throw std::runtime_error("Failed to create render pass");
}

std::vector<const char *> Application::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef _DEBUG
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	return extensions;
}

bool Application::isDeviceSuitable(VkPhysicalDevice device)
{
	bool isExtensionsSupported = checkDeviceExtensionsSupport(device);

	bool isSwapChainAdequate = false;
	if (isExtensionsSupported)
	{
		SwapChainSupportDetails swapChainSupportDetails = querySwapChainSupport(device);
		isSwapChainAdequate = !swapChainSupportDetails.formats.empty() && !swapChainSupportDetails.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeature;
	vkGetPhysicalDeviceFeatures(device, &supportedFeature);

	return (findQueueFamilies(device).isComplete() && isExtensionsSupported && isSwapChainAdequate && supportedFeature.samplerAnisotropy);
}

int Application::rateDeviceSuitability(VkPhysicalDevice device)
{
	int score = 0;

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 1000;
	score += deviceProperties.limits.maxImageDimension2D;

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	if (!deviceFeatures.geometryShader)
		return (0);

	if (!isDeviceSuitable(device))
		return (0);

	return (score);
}

bool Application::checkDeviceExtensionsSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto &extension : extensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}
	return (requiredExtensions.empty());
}

Application::SwapChainSupportDetails Application::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
	if (count != 0)
	{
		details.formats.resize(count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, details.formats.data());
	}

	count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
	if (count != 0)
	{
		details.presentModes.resize(count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, details.presentModes.data());
	}

	return (details);
}

VkSurfaceFormatKHR Application::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
	for (const auto &availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM
			&& availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
			return (availableFormat);
	}
	return (availableFormats[0]);
}

VkPresentModeKHR Application::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
	for (const auto &availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return (availablePresentMode);
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			return (availablePresentMode);
	}
	return (VK_PRESENT_MODE_FIFO_KHR);
}

VkExtent2D Application::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return (capabilities.currentExtent);
	else
	{
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width = std::max(capabilities.minImageExtent.width,
			std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, actualExtent.height));
		return (actualExtent);
	}
}

Application::QueueFamilyIndice Application::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndice indice;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto &queueFamily : queueFamilies)
	{
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport)
		{
			indice.presentFamily = i;
			if (indice.isComplete())
				break;
		}

		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indice.graphicFamily = i;
			if (indice.isComplete())
				break;
		}
		i++;
	}

	return (indice);
}

#ifdef _DEBUG

VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char *pLayerPrefix, const char *pMessage, void *pUserData)
{
	(void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
	fprintf(stderr, "[vulkan] ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
	return VK_FALSE;
}

#endif