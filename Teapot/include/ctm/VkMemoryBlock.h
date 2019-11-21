#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "ctm/VkCore.h"

namespace ctm
{
	class VkMemoryBlock
	{
	public:

		typedef size_t BlockID;

		VkMemoryBlock(ctm::VkCore &core) : core(core) {};
		BlockID defineBuffer(VkBufferUsageFlags usage, VkSharingMode sharing, uint32_t size);
		void allocateMemory();
		void *mapBuffer(BlockID idx);
		void unmapBuffer();

		VkBuffer &getBuffer(BlockID id) { return (buffers[id].buffer); };

		void destroy();

	private:
		struct Block
		{
			VkDeviceSize offset;
			VkDeviceSize size;
			VkBuffer buffer;
		};
		
		VkCore &core;

		VkDeviceMemory memory = VK_NULL_HANDLE;
		std::vector<Block> buffers;
	};
}