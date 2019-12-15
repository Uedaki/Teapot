#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace teapot
{
	namespace vk
	{
		class Pipeline
		{
		public:
			void init(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout, VkPolygonMode mode, VkExtent2D extent, const std::string &vert, const std::string &frag);
			void destroy();

			operator bool() const;

			inline VkPipeline &get() { return (pipeline); }
			inline VkPipelineLayout getLayout() { return (layout); }

		private:
			VkPipeline pipeline = VK_NULL_HANDLE;
			VkPipelineLayout layout = VK_NULL_HANDLE;
		};
	}
}