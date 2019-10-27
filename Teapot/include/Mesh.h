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


	public:
		ctm::VkCore &core;

		std::vector<ctm::VkVertex> vertices ={ 
			{{-1, -1, -1}, {1, 1, 1}},
			{{1, -1, -1}, {1, 1, 1}},
			{{1, 1, -1}, {1, 1, 1}},
			{{-1, 1, -1}, {1, 1, 1}},
			{{-1, -1, 1}, {1, 1, 1}},
			{{1, -1, 1}, {1, 1, 1}},
			{{1, 1, 1}, {1, 1, 1}},
			{{-1, 1, 1}, {1, 1, 1}},
		};
		std::vector<uint32_t> indices = {
			0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 5, 0, 4, 6, 6, 2, 0, 7, 6, 2, 2, 3, 7, 7, 5, 1, 1, 3, 7, 1, 0, 4, 4, 5, 1
		};

		VkBuffer vBuffer;
		VkDeviceMemory vBufferMemory;

		VkBuffer iBuffer;
		VkDeviceMemory iBufferMemory;
	};
}