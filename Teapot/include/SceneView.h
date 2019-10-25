#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "ctm/VkRasterizer.h"

namespace teapot
{
	struct SceneView : public ctm::VkRasterizer
	{
		VkSampler imageSampler;
		VkDescriptorSetLayout descriptorSetLayout;
		std::vector<VkFence> fences;
		std::vector<VkSemaphore> signalSemaphores;
		std::vector<VkDescriptorSet> descriptorSets;

		static void init(SceneView &scene, ctm::VkCore &core, VkDescriptorPool &descriptorPool, VkExtent2D extent, uint32_t imageCount);
		static void render(SceneView &scene, ctm::VkCore &core, uint32_t imageIdx);
		static void destroy(SceneView &scene, ctm::VkCore &core);
	};
}