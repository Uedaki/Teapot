#pragma once

#include <Vulkan/vulkan.h>

#include <vector>

namespace teapot
{
	namespace vk
	{
		class Command
		{
		public:
			void init();
			void destroy();

			void requestNextImage();
			VkCommandBuffer &recordNextBuffer();
			void submit();
			void submitAndPresent();
		private:
			uint32_t currImage = 0;
			uint32_t imageCount = 0;

			VkCommandPool commandPool = VK_NULL_HANDLE;
			std::vector<VkCommandBuffer> commandBuffers;

			std::vector<VkFence> fences;
			std::vector<VkSemaphore> submitSemaphores;
			std::vector<VkSemaphore> presentSemaphores;

			void createSyncObjects();
		};
	}
}