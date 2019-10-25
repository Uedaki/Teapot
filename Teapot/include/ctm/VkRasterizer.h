#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "ctm/VkCore.h"

namespace ctm
{
	struct VkRasterizer
	{
		uint32_t imageCount = 0;
		VkExtent2D extent = {};
		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkCommandPool commandPool = VK_NULL_HANDLE;
		std::vector<VkImage> images;
		std::vector<VkDeviceMemory> imageMemories;
		std::vector<VkImageView> imageViews;
		std::vector<VkFramebuffer> frameBuffers;
		std::vector<VkCommandBuffer> commandBuffers;

		static void init(VkRasterizer &rast, VkCore &core, VkExtent2D extent, uint32_t imageCount);
		static void destroy(VkRasterizer &rast, VkCore &core);
	};
}