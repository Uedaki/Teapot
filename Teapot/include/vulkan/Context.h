#pragma once

#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>

#include <vector>

#define VK_CHECK_RESULT(result)				\
{											\
	VkResult res = (result);				\
	if (VK_SUCCESS != res) {				\
		char buffer[256];					\
		snprintf(							\
			buffer,							\
			256,							\
			"(%s:%d)",						\
			__FILE__,						\
			__LINE__						\
		);									\
		throw std::runtime_error(buffer);	\
	}										\
}

namespace teapot
{
	namespace vk
	{
		struct Context
		{
			struct Configuration
			{
				VkFormat format = {};
				VkSurfaceFormatKHR surfaceFormat = {};
				VkExtent2D extent = {};
				VkPresentModeKHR presentMode = {};
			};

			struct SwapChain
			{
				uint32_t currImg;
				uint32_t imgCount;
				VkSwapchainKHR swapchain;
				std::vector<VkImage> images;
				std::vector<VkImageView> imageViews;
				std::vector<VkFramebuffer> frames;
			};

			struct Queue
			{
				constexpr static uint32_t invalidFamily = -1;

				uint32_t presentFamily = invalidFamily;
				VkQueue present = VK_NULL_HANDLE;

				uint32_t graphicsFamily = invalidFamily;
				VkQueue graphics = VK_NULL_HANDLE;

				uint32_t transferFamily = invalidFamily;
				VkQueue transfer = VK_NULL_HANDLE;
			};

			Configuration config;

			VkInstance instance = VK_NULL_HANDLE;
			VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
			VkDevice device = VK_NULL_HANDLE;
			Queue queue;

			VkSurfaceKHR surface = VK_NULL_HANDLE;

			SwapChain swapchainInfo;

			VkRenderPass renderPass;

			VkAllocationCallbacks *allocator = nullptr;
#ifdef VULKAN_DEBUG_LOG
			VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;
#endif

			void init();
			void destroy();
			void recreateSwapchain(uint32_t width, uint32_t height);

		private:
			void createInstance();
			void selectPhysicalDevice();
			void selectQueues();
			void createLogicalDevice();
			void getQueues();
			
			void createSwapchain();
			void createSwapchainImageViews();
			void createSwapchainFrames();
			void createRenderPass();

			void createSurface();

			void retreiveConfig();
			VkSurfaceFormatKHR selectSurfaceFormat();
			VkPresentModeKHR selectPresentMode();
			VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
		};
	}
}