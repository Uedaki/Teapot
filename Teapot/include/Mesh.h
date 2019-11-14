#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "ctm/VkCore.h"
#include "ctm/VkVertex.h"

namespace teapot
{
	class Mesh
	{
	public:
		Mesh(ctm::VkCore &core);
		~Mesh();

		void init();
		void destroy();

	public:
		ctm::VkCore &core;

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

		VkBuffer vBuffer;
		VkDeviceMemory vBufferMemory;

		VkBuffer iBuffer;
		VkDeviceMemory iBufferMemory;

		VkBuffer transformBuffer;
		VkDeviceMemory transformBufferMemory;
	};
}