#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "ctm/VkCore.h"
#include "ctm/VkVertex.h"
#include "ctm/VkMemoryBlock.h"

namespace teapot
{
	class Mesh
	{
	public:
		Mesh(ctm::VkCore &core);
		~Mesh();

		void init(uint32_t imageCount);
		void destroy();

		void updateTransform(const glm::vec3 &loc, const glm::vec3 &rot, const glm::vec3 &sc);

		void createDescriptorPool(VkDescriptorSetLayout &layout);
		void updateDescriptorSet(VkBuffer &buffer, uint32_t i);

		VkBuffer &getVertexBuffer(uint32_t currImage) { return (memory[currImage].getBuffer(vBuffer[currImage])); }
		VkBuffer &getIndexBuffer(uint32_t currImage) { return (memory[currImage].getBuffer(iBuffer[currImage])); }
		VkBuffer &getModelMatrixBuffer(uint32_t currImage) { return (memory[currImage].getBuffer(modelMatrixBuffer[currImage])); }

	public:
		uint32_t imageCount;

		glm::vec3 location = {0, 0, 0};
		glm::vec3 rotation = {0, 0, 0};
		glm::vec3 scale = { 1, 1, 1 };

		ctm::VkCore &core;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSet;

		std::vector<ctm::VkVertex> vertices = { 
			{{1, 1, 1}, {1, 1, 1}},
			{{-1, 1, 1}, {1, 1, 1}},
			{{-1, -1, 1}, {1, 1, 1}},
			{{1, -1, 1}, {1, 1, 1}},

			{{1, 1, -1}, {1, 1, 1}},
			{{1, -1, -1}, {1, 1, 1}},
			{{-1, -1, -1}, {1, 1, 1}},
			{{-1, 1, -1}, {1, 1, 1}},
		};
		std::vector<uint32_t> indices = {
			0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4, 5, 4, 0, 0, 3, 5, 0, 4, 7, 7, 1, 0, 1, 7, 6, 6, 2, 1, 5, 3, 2, 2, 6, 5
		};

		std::vector<ctm::VkMemoryBlock> memory;
		std::vector<ctm::VkMemoryBlock::BlockID> vBuffer;
		std::vector<ctm::VkMemoryBlock::BlockID> iBuffer;
		std::vector<ctm::VkMemoryBlock::BlockID> modelMatrixBuffer;

		void createDescriptorPool();
	};
}