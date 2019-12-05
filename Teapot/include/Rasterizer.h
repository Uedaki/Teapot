#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "RasterizerData.h"
#include "ctm/VkCore.h"

namespace teapot
{
	struct Rasterizer
	{
		VkPolygonMode mode;
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkCommandPool commandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> commandBuffers;

		void init(RasterizerData &data, ctm::VkCore &core, VkPolygonMode mode, VkExtent2D extent, uint32_t imageCount);
		void destroy(ctm::VkCore &core);
	};
}