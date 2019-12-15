#pragma once

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include <vector>

#include "vulkan/Vertex.h"

namespace teapot
{
	namespace vk
	{
		struct SceneViewDescriptors
		{
			VkDescriptorPool descriptorPool;
			std::vector<VkDescriptorSet> descriptorSet;
		};

		class Mesh
		{
		public:
			void init();
			void draw(VkCommandBuffer commandBuffer, VkPipelineLayout layout);
			void destroy();

			void updateTransform(glm::vec3 &location, glm::vec3 &rotation, glm::vec3 &scale);

		private:
			glm::vec3 location = { 0, 0, 0 };
			glm::vec3 rotation = { 0, 0, 0 };
			glm::vec3 scale = { 1, 1, 1 };
			glm::mat4 transform = glm::mat4(1);

			std::vector<Vertex> vertices = {
			{{1, 1, 1}, {1, 0, 0}},
			{{-1, 1, 1}, {0, 1, 0}},
			{{-1, -1, 1}, {1, 0, 0}},
			{{1, -1, 1}, {0, 0, 1}},

			{{1, 1, -1}, {1, 0, 0}},
			{{1, -1, -1}, {0, 1, 0}},
			{{-1, -1, -1}, {1, 0, 0}},
			{{-1, 1, -1}, {0, 0, 1}}
			};
			std::vector<Indice> indices = {
				{0, 1, 2},
				{2, 3, 0}, 
				{4, 5, 6},
				{6, 7, 4},
				{5, 4, 0},
				{0, 3, 5},
				{0, 4, 7},
				{7, 1, 0},
				{1, 7, 6},
				{6, 2, 1},
				{5, 3, 2},
				{2, 6, 5}
			};
			
			std::vector<VkBuffer> vertexBuffers;
			std::vector<VkDeviceMemory> vertexBufferMemories;

			std::vector<VkBuffer> indiceBuffers;
			std::vector<VkDeviceMemory> indiceBufferMemories;

			SceneViewDescriptors descriptors;
		};
	}
}