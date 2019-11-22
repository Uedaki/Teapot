#include "Mesh.h"

#include <stdexcept>

#include <glm/gtx/transform.hpp>

#include "ctm/VkUtils.h"
#include "ctm/TransformMatrix.h"

teapot::Mesh::Mesh(ctm::VkCore &core)
	: core(core)
{

}

void teapot::Mesh::init(uint32_t newImageCount)
{
	imageCount = newImageCount;

	vBuffer.resize(imageCount);
	iBuffer.resize(imageCount);
	modelMatrixBuffer.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; i++)
	{
		memory.emplace_back(core);

		vBuffer[i] = memory.back().defineBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, sizeof(vertices[0]) * vertices.size());
		iBuffer[i] = memory.back().defineBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, sizeof(indices[0]) * indices.size());
		modelMatrixBuffer[i] = memory.back().defineBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, sizeof(glm::mat4));
		memory.back().allocateMemory();
	
		void *data;

		data = memory.back().mapBuffer(vBuffer[i]);
		memcpy(data, vertices.data(), sizeof(vertices[0]) * vertices.size());
		memory.back().unmapBuffer();

		data = memory.back().mapBuffer(iBuffer[i]);
		memcpy(data, indices.data(), sizeof(indices[0]) * indices.size());
		memory.back().unmapBuffer();

		glm::mat4 model = glm::mat4(1);
		data = memory.back().mapBuffer(modelMatrixBuffer[i]);
		memcpy(data, &model, sizeof(model));
		memory.back().unmapBuffer();
	}
}

void teapot::Mesh::destroy()
{
	if (descriptorPool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(core.device, descriptorPool, core.allocator);

	for (auto &memoryBlock : memory)
	{
		memoryBlock.destroy();
	}
}

teapot::Mesh::~Mesh()
{

}

void teapot::Mesh::createDescriptorPool()
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = imageCount;

	std::vector<VkDescriptorPoolSize> poolSizes(imageCount, poolSize);

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = imageCount;
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = imageCount;
	if (vkCreateDescriptorPool(core.device, &poolInfo, core.allocator, &descriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create VkDescriptorPool");
}

void teapot::Mesh::allocDescriptorSet(VkDescriptorSetLayout &layout)
{
	if (descriptorPool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(core.device, descriptorPool, core.allocator);

	createDescriptorPool();

	std::vector<VkDescriptorSetLayout> layouts(imageCount, layout);

	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = descriptorPool;
	alloc_info.descriptorSetCount = imageCount;
	alloc_info.pSetLayouts = layouts.data();

	descriptorSet.resize(imageCount);
	if (vkAllocateDescriptorSets(core.device, &alloc_info, descriptorSet.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to create VkDescriptorSet");
}

void teapot::Mesh::updateDescriptorSet(VkBuffer &buffer, uint32_t i)
{
	VkDescriptorBufferInfo cameraBufferInfo = {};
	cameraBufferInfo.buffer = buffer;
	cameraBufferInfo.offset = 0;
	cameraBufferInfo.range = 2 * sizeof(glm::mat4);

	VkDescriptorBufferInfo objectBufferInfo = {};
	objectBufferInfo.buffer = memory[i].getBuffer(modelMatrixBuffer[i]);
	objectBufferInfo.offset = 0;
	objectBufferInfo.range = sizeof(glm::mat4);

	VkWriteDescriptorSet descriptorWrite[2] = {};
	descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[0].dstSet = descriptorSet[i];
	descriptorWrite[0].dstBinding = 0;
	descriptorWrite[0].dstArrayElement = 0;
	descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite[0].descriptorCount = 1;
	descriptorWrite[0].pBufferInfo = &cameraBufferInfo;

	descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[1].dstSet = descriptorSet[i];
	descriptorWrite[1].dstBinding = 1;
	descriptorWrite[1].dstArrayElement = 0;
	descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite[1].descriptorCount = 1;
	descriptorWrite[1].pBufferInfo = &objectBufferInfo;
	vkUpdateDescriptorSets(core.device, 2, descriptorWrite, 0, nullptr);
}

void teapot::Mesh::destroyDescriptorPool()
{
}

void teapot::Mesh::updateTransform(const glm::vec3 &loc, const glm::vec3 &rot, const glm::vec3 &sc)
{
	if (loc != location || rot != rotation || sc != scale)
	{
		location = loc;
		rotation = rot;
		scale = sc;

		glm::mat4 mat(1);
		mat = glm::translate(mat, loc);

		mat = glm::rotate(mat, rot.x, glm::vec3(1, 0, 0));
		mat = glm::rotate(mat, rot.y, glm::vec3(0, 1, 0));
		mat = glm::rotate(mat, rot.z, glm::vec3(0, 0, 1));
		
		mat = glm::scale(mat, scale);

		vkDeviceWaitIdle(core.device);

		for (uint32_t i = 0; i < modelMatrixBuffer.size(); i++)
		{
			void *data = memory[i].mapBuffer(modelMatrixBuffer[i]);
			memcpy(data, &mat, sizeof(mat));
			memory[i].unmapBuffer();
		}
	}
}