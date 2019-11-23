#pragma once

#include <Vulkan/vulkan.h>

#include "Exception.h"

struct GLFWwindow;

#define VK_STATUS(a, msg) a != VK_SUCCESS ? EXCEPTION(msg) : true
#define VK_CRITICAL_STATUS(a, msg) a != VK_SUCCESS ? CRITICAL_EXCEPTION(msg) : true

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

#ifdef VULKAN_DEBUG_LOG
		VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;
#endif

		VkAllocationCallbacks *allocator = nullptr;

		static void init(VkCore &core, GLFWwindow *win);
		static void destroy(VkCore &core);
	};
}