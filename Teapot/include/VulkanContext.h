#pragma once

#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.h>

#include <glfw/glfw3.h>

#include <optional>
#include <string>
#include <vector>

class VulkanContext
{
public:
	VulkanContext() = default;

	void init(GLFWwindow *win);
	void destroy();

	uint32_t getQueueFamily() { return (graphicsQueueIdx); }
	VkInstance getInstance() { return (instance); }
	VkPhysicalDevice getPhysicalDevice() { return (physicalDevice); }
	VkDevice getDevice() { return (device); }
	VkSurfaceKHR getSurface() { return (surface); }
	VkQueue getGraphicsQueue() { return (graphicsQueue); }
	VkRenderPass getRenderPass() { return (renderPass); }
	VkDescriptorPool getDescriptorPool() { return (descriptorPool); }
	VkAllocationCallbacks *getVulkanAllocator() { return(allocator); }

private:

public:
	struct QueueFamilyIndice
	{
		std::optional<uint32_t> graphicFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return (graphicFamily.has_value()
				&& presentFamily.has_value());
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	const std::vector<const char *> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkAllocationCallbacks *allocator = nullptr;

	VkInstance instance = VK_NULL_HANDLE;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	uint32_t graphicsQueueIdx = -1;
	VkQueue graphicsQueue = VK_NULL_HANDLE;

	VkDevice device = VK_NULL_HANDLE;

	VkSurfaceKHR surface = VK_NULL_HANDLE;

	VkSwapchainKHR swapchain = VK_NULL_HANDLE;

	VkQueue presentQueue = VK_NULL_HANDLE;
	std::vector<VkImage> swapchainImages = {};
	std::vector<VkImageView> swapchainImageViews = {};
	VkFormat swapchainImageFormat = {};
	VkExtent2D swapchainExtent = {};
	VkRenderPass renderPass = VK_NULL_HANDLE;

	VkPipelineCache pipelineCache = VK_NULL_HANDLE;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

	void createInstance();
	void selectPhysicalDevice();
	void selectGraphicsQueue();

	void createSurface(GLFWwindow *win);

	void createLogicalDevice();
	//void createSwapchain();
	void createImageViews();
	void createRenderPass();
	void createDescriptorPool();
	//void createGraphicsPipeline();
	//void createFrameBuffers();
	//void createCommandPool();
	//void createCommandBuffers();
	//void createSyncObjects();

	std::vector<const char *> getRequiredExtensions();
	bool isDeviceSuitable(VkPhysicalDevice device);
	int rateDeviceSuitability(VkPhysicalDevice device);
	bool checkDeviceExtensionsSupport(VkPhysicalDevice device);

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
	VkExtent2D chooseSwapExtent(GLFWwindow *win, const VkSurfaceCapabilitiesKHR &capabilities);

	QueueFamilyIndice findQueueFamilies(VkPhysicalDevice device);

#ifdef _DEBUG
	const std::vector<const char *> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;

#endif
};