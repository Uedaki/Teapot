#pragma once

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include <vector>

#include "DisplayMode.h"
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

			void select(DisplayMode mode, uint32_t v1, uint32_t v2, uint32_t v3);
			void selectFace(uint32_t v1, uint32_t v2, uint32_t v3);
			void selectEdge(uint32_t v1, uint32_t v2);
			void selectVertex(uint32_t index);
			void unselect(DisplayMode mode);

			inline const std::vector<Vertex> &getVertices() const { return (vertices); };
			inline const std::vector<Indice> &getIndices() const { return (indices); }
			inline const glm::mat4 &getTransform() const { return (transform); }

		private:
			glm::vec3 location = { 0, 0, 0 };
			glm::vec3 rotation = { 0, 0, 0 };
			glm::vec3 scale = { 1, 1, 1 };
			glm::mat4 transform = glm::mat4(1);

			std::vector<Vertex> vertices = {
				{{1, 1, 1}, {0, 0, 1}, {0.80, 0.80, 0.80}, {1, 1}, {0, 0, 0}},
				{{-1, 1, 1}, {0, 0, 1}, {0.80, 0.80, 0.80}, {-1, 1}, {0, 0, 0}},
				{{-1, -1, 1}, {0, 0, 1}, {0.80, 0.80, 0.80}, {-1, -1}, {0, 0, 0}},
				{{1, -1, 1}, {0, 0, 1}, {0.80, 0.80, 0.80}, {1, -1}, {0, 0, 0}},

				{{1, 1, -1}, {1, 0, 0}, {0.80, 0.80, 0.80}, {1, 1}, {0, 0, 0}},
				{{1, -1, -1}, {0, 1, 0}, {0.80, 0.80, 0.80}, {1, 1}, {0, 0, 0}},
				{{-1, -1, -1}, {1, 0, 0}, {0.80, 0.80, 0.80}, {1, 1}, {0, 0, 0}},
				{{-1, 1, -1}, {0, 0, 1}, {0.80, 0.80, 0.80}, {1, 1}, {0, 0, 0}}
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

			VkBuffer vertexBuffer;
			VkDeviceMemory vertexBufferMemory;

			VkBuffer indiceBuffer;
			VkDeviceMemory indiceBufferMemory;

			VkBuffer edgeIndiceBuffer;
			VkDeviceMemory edgeIndiceBufferMemory;

			SceneViewDescriptors descriptors;
		};
	}
}