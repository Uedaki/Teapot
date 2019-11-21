#include "ctm/VkMemoryBlock.h"

#include <stdexcept>

#include "ctm/VkUtils.h"

ctm::VkMemoryBlock::BlockID ctm::VkMemoryBlock::defineBuffer(VkBufferUsageFlags usage, VkSharingMode sharing, uint32_t size)
{
	buffers.emplace_back();
	Block &block = buffers.back();
	VkBuffer &buffer = block.buffer;

	VkBufferCreateInfo vBufferInfo = {};
	vBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vBufferInfo.size = size;
	vBufferInfo.usage = usage;
	vBufferInfo.sharingMode = sharing;
	if (vkCreateBuffer(core.device, &vBufferInfo, core.allocator, &buffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to create VkBuffer");
	return (buffers.size() - 1);
}

void ctm::VkMemoryBlock::allocateMemory()
{
	VkDeviceSize size = 0;
	uint32_t memoryType = 0;
	for (auto &block : buffers)
	{
		block.offset = size;

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(core.device, block.buffer, &memRequirements);
		memoryType = memoryType | memRequirements.memoryTypeBits;
		block.size = memRequirements.size;
		size += memRequirements.size;
	}

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = size;
	allocInfo.memoryTypeIndex = ctm::VkUtils::findMemoryType(core, memoryType,
															 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	if (vkAllocateMemory(core.device, &allocInfo, core.allocator, &memory) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate buffer memory");

	for (auto &block : buffers)
	{
		vkBindBufferMemory(core.device, block.buffer, memory, block.offset);
	}
}

void *ctm::VkMemoryBlock::mapBuffer(BlockID idx)
{
	void *data;
	vkMapMemory(core.device, memory, buffers[idx].offset, buffers[idx].size, 0, &data);
	return (data);
}

void ctm::VkMemoryBlock::unmapBuffer()
{
	vkUnmapMemory(core.device, memory);
}

void ctm::VkMemoryBlock::destroy()
{
	for (auto &bufferBlock : buffers)
	{
		vkDestroyBuffer(core.device, bufferBlock.buffer, core.allocator);
	}
	buffers.clear();

	vkFreeMemory(core.device, memory, core.allocator);
}