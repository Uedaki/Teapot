#include "PipelinePool.h"

#include <vector>

#include "ctm/VkUtils.h"
#include "ctm/VkVertex.h"

teapot::Pipeline::Pipeline(teapot::PipelinePool *pool, uint32_t index)
	: pool(pool), index(index)
{}

teapot::Pipeline::Pipeline(const teapot::Pipeline &ref)
	: pool(ref.pool), index(ref.index)
{}

teapot::Pipeline &teapot::Pipeline::operator=(const teapot::Pipeline &ref)
{
	if (&ref != this)
	{
		pool = ref.pool;
		index = ref.index;
	}
	return (*this);
}

void teapot::Pipeline::recreate(VkPolygonMode mode, const std::string &vert, const std::string &frag)
{
	vkDestroyPipeline(pool->core.device, pool->pipelines[index], pool->core.allocator);
	pool->pipelines[index] = pool->changePipeline(mode, vert, frag);
}

void teapot::Pipeline::destroy()
{
	pool->pipelines.erase(pool->pipelines.begin() + index);
	index = -1;
}

VkPipeline teapot::Pipeline::get()
{
	return (pool->pipelines[index]);
}

void teapot::PipelinePool::init(VkExtent2D newExtent)
{
	extent = newExtent;

	createDescriptorSetLayout();
	createPipelineLayout();
	createRenderPass();
}

void teapot::PipelinePool::destroy()
{
	for (auto &pipeline : pipelines)
	{
		vkDestroyPipeline(core.device, pipeline, core.allocator);
	}

	vkDestroyDescriptorSetLayout(core.device, descriptorSetLayout, core.allocator);
	vkDestroyPipelineLayout(core.device, pipelineLayout, core.allocator);
	vkDestroyRenderPass(core.device, renderPass, core.allocator);
}

teapot::Pipeline teapot::PipelinePool::createPipeline(VkPolygonMode mode, const std::string &vert, const std::string &frag)
{
	pipelines.push_back(changePipeline(mode, vert, frag));

	return (Pipeline(this, pipelines.size() - 1));
}

VkPipeline teapot::PipelinePool::changePipeline(VkPolygonMode mode, const std::string &vert, const std::string &frag)
{
	VkPipeline pipeline;

	VkShaderModule vertShaderModule = ctm::VkUtils::createShaderModule(core, vert);
	VkShaderModule fragShaderModule = ctm::VkUtils::createShaderModule(core, frag);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	auto attrDesc = ctm::VkVertex::getAttributeDescriptions();
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &ctm::VkVertex::getBindingDescription();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrDesc.size());
	vertexInputInfo.pVertexAttributeDescriptions = attrDesc.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = mode;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	if (vkCreateGraphicsPipelines(core.device, VK_NULL_HANDLE, 1, &pipelineInfo, core.allocator, &pipeline) != VK_SUCCESS)
		throw std::runtime_error("failed to create graphics pipeline!");

	vkDestroyShaderModule(core.device, fragShaderModule, core.allocator);
	vkDestroyShaderModule(core.device, vertShaderModule, core.allocator);

	return (pipeline);
}

void teapot::PipelinePool::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding binding[2] = {};
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
	if (vkCreateDescriptorSetLayout(core.device, &info, core.allocator, &descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout");
}

void teapot::PipelinePool::createRenderPass()
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
	if (vkCreateRenderPass(core.device, &renderPassInfo, core.allocator, &renderPass) != VK_SUCCESS)
		throw std::runtime_error("Failed to create VkRenderPass");
}

void teapot::PipelinePool::createPipelineLayout()
{
	std::vector<VkDescriptorSetLayout> layouts(2, descriptorSetLayout);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 2;
	pipelineLayoutInfo.pSetLayouts = layouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	if (vkCreatePipelineLayout(core.device, &pipelineLayoutInfo, core.allocator, &pipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipeline layout!");
}

VkRenderPass teapot::PipelinePool::getRenderPass()
{
	return (renderPass);
}

VkPipelineLayout teapot::PipelinePool::getPipelineLayout()
{
	return (pipelineLayout);
}

VkDescriptorSetLayout teapot::PipelinePool::getDescriptorSetLayout()
{
	return (descriptorSetLayout);
}