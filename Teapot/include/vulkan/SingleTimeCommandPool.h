#pragma once

#include <vulkan/vulkan.h>

namespace teapot
{
	namespace vk
	{
		class SingleTimeCommandPool
		{
		public:
			void init(VkQueue queue, uint32_t queueFamily);
			void destroy();

			VkCommandBuffer &startRecording();
			void finishAndSubmit();

		private:
			VkQueue *queue = nullptr;
			VkCommandPool commandPool = VK_NULL_HANDLE;
			VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
			VkFence fence = VK_NULL_HANDLE;
		};
	}
}