#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace teapot
{
	namespace vk
	{
		class Utils
		{
		public:
			static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
			static void createBuffer(VkBuffer &buffer, VkDeviceMemory &bufferMemory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
			static VkShaderModule createShaderModule(const std::string &shader);

		private:
			Utils() = default;
		};
	}
}