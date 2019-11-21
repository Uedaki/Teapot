#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "ctm/VkCore.h"

namespace teapot
{
	struct Rasterizer
	{
		uint32_t imageCount = 0;
		VkExtent2D extent = {};
		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkSampler imageSampler = VK_NULL_HANDLE;
		VkCommandPool commandPool = VK_NULL_HANDLE;
		std::vector<VkImage> images;
		std::vector<VkDeviceMemory> imageMemories;
		std::vector<VkImageView> imageViews;
		std::vector<VkFramebuffer> frameBuffers;
		std::vector<VkCommandBuffer> commandBuffers;

		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

		static void init(Rasterizer &rast, ctm::VkCore &core, VkExtent2D extent, uint32_t imageCount);
		static void destroy(Rasterizer &rast, ctm::VkCore &core);
	};
}