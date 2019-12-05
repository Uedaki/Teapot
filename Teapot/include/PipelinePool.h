#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "ctm/VkCore.h"

namespace teapot
{
	class Pipeline
	{
	public:
		Pipeline() {}
		Pipeline(const Pipeline &ref);
		Pipeline &operator=(const Pipeline &ref);
		operator bool() const { return (index != -1); }

		void recreate(VkPolygonMode mode, const std::string &vert, const std::string &frag);
		void destroy();

		VkPipeline get();
	private:
		friend class PipelinePool;

		PipelinePool *pool = nullptr;
		uint32_t index = -1;

		Pipeline(PipelinePool *pool, uint32_t index);
	};

	class PipelinePool
	{
	public:
		PipelinePool(ctm::VkCore &core) : core(core) {}
		~PipelinePool() = default;

		void init(VkExtent2D extent);
		void destroy();

		Pipeline createPipeline(VkPolygonMode mode, const std::string &vert, const std::string &frag);

		VkRenderPass getRenderPass();
		VkPipelineLayout getPipelineLayout();
		VkDescriptorSetLayout getDescriptorSetLayout();

	private:
		friend class Pipeline;

		ctm::VkCore &core;
		VkExtent2D extent;

		uint32_t imageCount = 0;
		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

		std::vector<VkPipeline> pipelines;

		VkPipeline changePipeline(VkPolygonMode mode, const std::string &vert, const std::string &frag);
		void createDescriptorSetLayout();
		void createRenderPass();
		void createPipelineLayout();
	};
}