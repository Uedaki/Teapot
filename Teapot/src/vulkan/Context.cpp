#include "vulkan/Context.h"

#include <iostream>
#include <set>
#include <vector>

#include "Application.h"
#include "Log.h"
#include "Profiler.h"

namespace
{
#ifdef VULKAN_DEBUG_LOG
	VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT,
													   VkDebugReportObjectTypeEXT objectType,
													   uint64_t, size_t, int32_t, const char *,
													   const char *pMessage,
													   void *)
	{
		std::cerr << "[vulkan] ObjectType: " << objectType << "\nMessage: " << pMessage << "\n" << std::endl;;
		return VK_FALSE;
	}
#endif
}

void teapot::vk::Context::init()
{
	PROFILE_FUNCTION("Vulkan");

	createInstance();
	selectPhysicalDevice();
	createSurface();
	selectQueues();
	createLogicalDevice();
	getQueues();
	createSwapchain();
	createRenderPass();
	createSwapchainImageViews();
	createSwapchainFrames();

	retreiveConfig();
}

void teapot::vk::Context::destroy()
{
	PROFILE_FUNCTION("Vulkan");

	for (uint32_t i = 0; i < swapchainInfo.images.size(); i++)
	{
		vkDestroyFramebuffer(device, swapchainInfo.frames[i], allocator);
		vkDestroyImageView(device, swapchainInfo.imageViews[i], allocator);
	}
	vkDestroyRenderPass(device, renderPass, allocator);
	vkDestroySwapchainKHR(device, swapchainInfo.swapchain, allocator);
	vkDestroyDevice(device, allocator);
#ifdef VULKAN_DEBUG_LOG
	auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	vkDestroyDebugReportCallbackEXT(instance, debugReport, allocator);
#endif
	vkDestroySurfaceKHR(instance, surface, allocator);
	vkDestroyInstance(instance, allocator);
}

void teapot::vk::Context::recreateSwapchain(uint32_t width, uint32_t height)
{
	vkDeviceWaitIdle(device);

	for (uint32_t i = 0; i < swapchainInfo.images.size(); i++)
	{
		vkDestroyFramebuffer(device, swapchainInfo.frames[i], allocator);
		vkDestroyImageView(device, swapchainInfo.imageViews[i], allocator);
	}

	vkDestroyRenderPass(device, renderPass, allocator);
	vkDestroySwapchainKHR(device, swapchainInfo.swapchain, allocator);

	config.extent.width = width;
	config.extent.height = height;

	createSwapchain();
	createRenderPass();
	createSwapchainImageViews();
	createSwapchainFrames();
}

void teapot::vk::Context::createInstance()
{
#ifdef VULKAN_DEBUG_LOG
	const char *layers[] = { "VK_LAYER_LUNARG_standard_validation" };
#endif

	const char **glfwExtensions;
	uint32_t glfwExtensionCount = 0;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifdef VULKAN_DEBUG_LOG
	extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

	VkInstanceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	create_info.ppEnabledExtensionNames = extensions.data();
#ifdef VULKAN_DEBUG_LOG
	create_info.enabledLayerCount = 1;
	create_info.ppEnabledLayerNames = layers;
	VK_CHECK_RESULT(vkCreateInstance(&create_info, allocator, &instance));
#else
	VK_CHECK_RESULT(vkCreateInstance(&create_info, allocator, &instance));
#endif

#ifdef VULKAN_DEBUG_LOG
	auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

	VkDebugReportCallbackCreateInfoEXT debugReportInfo = {};
	debugReportInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debugReportInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	debugReportInfo.pfnCallback = debugReportCallback;
	debugReportInfo.pUserData = nullptr;
	VK_CHECK_RESULT(vkCreateDebugReportCallbackEXT(instance, &debugReportInfo, allocator, &debugReport));
#endif
}

void teapot::vk::Context::selectPhysicalDevice()
{
	uint32_t count;
	if (vkEnumeratePhysicalDevices(instance, &count, nullptr) != VK_SUCCESS)
		throw std::runtime_error("Enable to retreive physical device");
	std::vector<VkPhysicalDevice> physicalDevices(count);
	if (vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data()) != VK_SUCCESS)
		throw std::runtime_error("Enable to retreive physical device");


	for (VkPhysicalDevice &physicalDevice : physicalDevices)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			LOG_MSG("Device selected: %s", properties.deviceName	);
			this->physicalDevice = physicalDevice;
			return;
		}
	}

	throw std::runtime_error("Could not find a suitable physical device");
}

void teapot::vk::Context::createSurface()
{
	GLFWwindow *win = Application::get().getWindow();
	if (glfwCreateWindowSurface(instance, win, allocator, &surface) != VK_SUCCESS)
		throw std::runtime_error("Failed to create window surface");
}

void teapot::vk::Context::selectQueues()
{
	uint32_t count;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, NULL);
	std::vector<VkQueueFamilyProperties> queueProperties(count);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queueProperties.data());
	for (uint32_t i = 0; i < count; i++)
	{
		VkBool32 isSupported;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &isSupported);
		if (queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queue.presentFamily = i;
			queue.graphicsFamily = i;
			break;
		}
		if (isSupported)
			queue.presentFamily = i;
		if (queueProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
			queue.transferFamily = i;
	}
}

void teapot::vk::Context::createLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { queue.presentFamily, queue.graphicsFamily, queue.transferFamily };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		if (queueFamily != Queue::invalidFamily)
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

	VkPhysicalDeviceFeatures physicalDeviceFeature = {};
	physicalDeviceFeature.fillModeNonSolid = VK_TRUE;

	VkDeviceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	create_info.pQueueCreateInfos = queueCreateInfos.data();
	create_info.enabledExtensionCount = 1;
	create_info.ppEnabledExtensionNames = device_extensions;
	create_info.pEnabledFeatures = &physicalDeviceFeature;
	if (vkCreateDevice(physicalDevice, &create_info, allocator, &device) != VK_SUCCESS)
		throw std::runtime_error("Unable to create logical device");
}

void teapot::vk::Context::getQueues()
{
	if (queue.presentFamily != Queue::invalidFamily)
		vkGetDeviceQueue(device, queue.presentFamily, 0, &queue.present);
	if (queue.graphicsFamily != Queue::invalidFamily)
		vkGetDeviceQueue(device, queue.graphicsFamily, 0, &queue.graphics);
	if (queue.transferFamily != Queue::invalidFamily)
		vkGetDeviceQueue(device, queue.transferFamily, 0, &queue.transfer);
	else
	{
		queue.transferFamily = queue.graphicsFamily;
		queue.transfer = queue.graphics;
	}
}

void teapot::vk::Context::createSwapchain()
{
	VkSurfaceFormatKHR surfaceFormat = selectSurfaceFormat();
	VkPresentModeKHR presentMode = selectPresentMode();

	VkSurfaceCapabilitiesKHR capabilities = {};
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities));
	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		imageCount = capabilities.maxImageCount;
	chooseSwapExtent(capabilities);

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = config.extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchainInfo.swapchain))

	vkGetSwapchainImagesKHR(device, swapchainInfo.swapchain, &imageCount, nullptr);
	swapchainInfo.images.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapchainInfo.swapchain, &imageCount, swapchainInfo.images.data());

	swapchainInfo.imgCount = imageCount;

	config.surfaceFormat = surfaceFormat;
	config.presentMode = presentMode;
	config.format = surfaceFormat.format;
}

void teapot::vk::Context::createSwapchainImageViews()
{
	swapchainInfo.imageViews.resize(swapchainInfo.images.size());
	for (uint32_t i = 0; i < swapchainInfo.imageViews.size(); i++)
	{
		VkImageViewCreateInfo imageViewInfo = {};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.image = swapchainInfo.images[i];
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = config.format;
		imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;
		VK_CHECK_RESULT(vkCreateImageView(device, &imageViewInfo, allocator, &swapchainInfo.imageViews[i]));
	}
}

void teapot::vk::Context::createSwapchainFrames()
{
	swapchainInfo.frames.resize(swapchainInfo.images.size());
	for (uint32_t i = 0; i < swapchainInfo.frames.size(); i++)
	{
		VkFramebufferCreateInfo frameInfo = {};
		frameInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameInfo.renderPass = renderPass;
		frameInfo.attachmentCount = 1;
		frameInfo.pAttachments = &swapchainInfo.imageViews[i];
		frameInfo.width = config.extent.width;
		frameInfo.height = config.extent.height;
		frameInfo.layers = 1;
		VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameInfo, allocator, &swapchainInfo.frames[i]));
	}
}

void teapot::vk::Context::createRenderPass()
{
	VkAttachmentDescription colorAttachement = {};
	colorAttachement.format = config.format;
	colorAttachement.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachement.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachement.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachement.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachement.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachement.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachement.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorReference;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachement;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, allocator, &renderPass));
}

void teapot::vk::Context::retreiveConfig()
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
	config.extent = surfaceCapabilities.currentExtent;

	config.surfaceFormat = selectSurfaceFormat();
	config.format = config.surfaceFormat.format;
	config.presentMode = selectPresentMode();
}

VkSurfaceFormatKHR teapot::vk::Context::selectSurfaceFormat()
{
	uint32_t surfaceFormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data()) != VK_SUCCESS)
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

VkPresentModeKHR teapot::vk::Context::selectPresentMode()
{
	VkPresentModeKHR best;

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

	for (size_t i = 0; i < presentModeCount; i++)
	{
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentModes[i];
		else if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
			best = presentModes[i];
	}
	return (best);
}

VkExtent2D teapot::vk::Context::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		config.extent = capabilities.currentExtent;
		return capabilities.currentExtent;
	}
	else {
		config.extent.width = std::max(capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		config.extent.height = std::max(capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return config.extent;
	}
}