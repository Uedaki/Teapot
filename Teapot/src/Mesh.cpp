#include "Mesh.h"

#include <stdexcept>

#include "ctm/VkUtils.h"

teapot::Mesh::Mesh(ctm::VkCore &core)
	: core(core)
{
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(vertices[0]) * vertices.size();
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (vkCreateBuffer(core.device, &bufferInfo, core.allocator, &vBuffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to create VkBuffer");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(core.device, vBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = ctm::VkUtils::findMemoryType(core, memRequirements.memoryTypeBits,
																 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		if (vkAllocateMemory(core.device, &allocInfo, core.allocator, &vBufferMemory) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate buffer memory");
		vkBindBufferMemory(core.device, vBuffer, vBufferMemory, 0);

		void *data;
		vkMapMemory(core.device, vBufferMemory, 0, bufferInfo.size, 0, &data);
		memcpy(data, vertices.data(), bufferInfo.size);
		vkUnmapMemory(core.device, vBufferMemory);
	}

	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(indices[0]) * indices.size();
		bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (vkCreateBuffer(core.device, &bufferInfo, core.allocator, &iBuffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to create VkBuffer");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(core.device, iBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = ctm::VkUtils::findMemoryType(core, memRequirements.memoryTypeBits,
																 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		if (vkAllocateMemory(core.device, &allocInfo, core.allocator, &iBufferMemory) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate buffer memory");
		vkBindBufferMemory(core.device, iBuffer, iBufferMemory, 0);

		void *data;
		vkMapMemory(core.device, iBufferMemory, 0, bufferInfo.size, 0, &data);
		memcpy(data, indices.data(), bufferInfo.size);
		vkUnmapMemory(core.device, iBufferMemory);

	}
}

teapot::Mesh::~Mesh()
{
	vkDestroyBuffer(core.device, vBuffer, core.allocator);
	vkFreeMemory(core.device, vBufferMemory, core.allocator);

	vkDestroyBuffer(core.device, iBuffer, core.allocator);
	vkFreeMemory(core.device, iBufferMemory, core.allocator);
}