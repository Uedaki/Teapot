#include "RasterizerData.h"

#include <vector>

#include "ctm/VkCore.h"
#include "profiler/Profiler.h"

namespace
{
	void createDescriptorSetLayout(teapot::RasterizerData &data, ctm::VkCore &core)
	{
		VkDescriptorSetLayoutBinding binding[2];
		binding[0].binding = 0;
		binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		binding[0].descriptorCount = 1;
		binding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		binding[0].pImmutableSamplers = nullptr;
		binding[1].binding = 1;
		binding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		binding[1].descriptorCount = 1;
		binding[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		binding[1].pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = 2;
		info.pBindings = binding;
		if (vkCreateDescriptorSetLayout(core.device, &info, core.allocator, &data.descriptorSetLayout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create descriptor set layout");
	}

	void createRenderPass(teapot::RasterizerData &rast, ctm::VkCore &core)
	{
		VkAttachmentDescription colorAttachement = {};
		colorAttachement.format = VK_FORMAT_R8G8B8A8_UNORM;
		colorAttachement.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachement.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachement.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachement.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachement.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachement.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachement.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;// VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorReference;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachement;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		if (vkCreateRenderPass(core.device, &renderPassInfo, core.allocator, &rast.renderPass) != VK_SUCCESS)
			throw std::runtime_error("Failed to create VkRenderPass");
	}

	void createPipelineLayout(teapot::RasterizerData &rast, ctm::VkCore &core)
	{
		std::vector<VkDescriptorSetLayout> layouts(2, rast.descriptorSetLayout);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		if (vkCreatePipelineLayout(core.device, &pipelineLayoutInfo, core.allocator, &rast.pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create pipeline layout!");
	}
}

void teapot::RasterizerData::init(ctm::VkCore &core)
{
	PROFILE_FUNCTION("blop");

	createDescriptorSetLayout(*this, core);
	createRenderPass(*this, core);
	createPipelineLayout(*this, core);
}

void teapot::RasterizerData::destroy(ctm::VkCore &core)
{
	PROFILE_FUNCTION("blop");

	vkDestroyDescriptorSetLayout(core.device, descriptorSetLayout, core.allocator);
	vkDestroyPipelineLayout(core.device, pipelineLayout, core.allocator);
	vkDestroyRenderPass(core.device, renderPass, core.allocator);
}