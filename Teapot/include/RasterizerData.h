#pragma once

#include <vulkan/vulkan.h>

namespace ctm { struct VkCore; }

namespace teapot
{
	struct RasterizerData
	{
		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

		void init(ctm::VkCore &core);
		void destroy(ctm::VkCore &core);
	};
}