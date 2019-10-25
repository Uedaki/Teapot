#pragma once

#include <Vulkan/vulkan.h>

struct GLFWwindow;

namespace ctm
{
	struct VkCore
	{
		struct Configuration
		{
			VkFormat format = {};
			VkSurfaceFormatKHR surfaceFormat = {};
			VkExtent2D extent = {};
			VkPresentModeKHR presentMode = {};
		};

		struct Queue
		{
			constexpr static uint32_t invalidIdx = -1;

			uint32_t presentIdx = invalidIdx;
			VkQueue present = VK_NULL_HANDLE;

			uint32_t graphicsIdx = invalidIdx;
			VkQueue graphics = VK_NULL_HANDLE;

			uint32_t transferIdx = invalidIdx;
			VkQueue transfer = VK_NULL_HANDLE;
		};

		VkInstance instance = VK_NULL_HANDLE;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		Configuration config;
		Queue queue;

#ifdef _DEBUG
		VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;
#endif

		VkAllocationCallbacks *allocator = nullptr;

		static void init(VkCore &core, GLFWwindow *win);
		static void destroy(VkCore &core);
	};
}