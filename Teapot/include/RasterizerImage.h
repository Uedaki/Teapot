#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace ctm { struct VkCore; }
namespace teapot { struct RasterizerData; }

namespace teapot
{
	struct RasterizerImage
	{
		uint32_t imageCount = 0;
		VkExtent2D extent = {};

		VkSampler imageSampler = VK_NULL_HANDLE;

		std::vector<VkImage> images;
		std::vector<VkDeviceMemory> imageMemories;
		std::vector<VkImageView> imageViews;
		std::vector<VkFramebuffer> frameBuffers;

		void init(ctm::VkCore &core, VkRenderPass renderPass, VkExtent2D extent, uint32_t newImageCount);
		void destroy(ctm::VkCore &core);
	};
}