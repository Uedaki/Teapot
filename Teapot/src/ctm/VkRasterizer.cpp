#include "ctm/VkRasterizer.h"

#include <fstream>
#include <stdexcept>
#include <vector>

#include "ctm/VkUtils.h"
#include "ctm/VkVertex.h"

namespace
{
	void createRenderPass(ctm::VkRasterizer &rast, ctm::VkCore &core)
	{
		VkAttachmentDescription colorAttachement = {};
		colorAttachement.format = VK_FORMAT_R8G8B8A8_UNORM;
		colorAttachement.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachement.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachement.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachement.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachement.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachement.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachement.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;// VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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

	std::vector<char> readFile(const std::string &filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	VkShaderModule createShaderModule(const std::vector<char> &code, ctm::VkCore &core)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(core.device, &createInfo, core.allocator, &shaderModule) != VK_SUCCESS)
			throw std::runtime_error("failed to create shader module!");

		return shaderModule;
	}

	void createPipeline(ctm::VkRasterizer &rast, ctm::VkCore &core)
	{
		auto vertShaderCode = readFile("shader/rasterizer.vert.spv");
		auto fragShaderCode = readFile("shader/rasterizer.frag.spv");

		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode, core);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode, core);

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
		viewport.width = static_cast<float>(rast.extent.width);
		viewport.height = static_cast<float>(rast.extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = rast.extent;

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
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		if (vkCreatePipelineLayout(core.device, &pipelineLayoutInfo, core.allocator, &rast.pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create pipeline layout!");

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
		pipelineInfo.layout = rast.pipelineLayout;
		pipelineInfo.renderPass = rast.renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		if (vkCreateGraphicsPipelines(core.device, VK_NULL_HANDLE, 1, &pipelineInfo, core.allocator, &rast.pipeline) != VK_SUCCESS)
			throw std::runtime_error("failed to create graphics pipeline!");

		vkDestroyShaderModule(core.device, fragShaderModule, core.allocator);
		vkDestroyShaderModule(core.device, vertShaderModule, core.allocator);
	}

	void createImage(ctm::VkRasterizer &rast, ctm::VkCore &core, VkImage &image)
	{
		uint32_t familyIndice = core.queue.graphicsIdx;
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageInfo.extent.width = rast.extent.width;
		imageInfo.extent.height = rast.extent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.queueFamilyIndexCount = 1;
		imageInfo.pQueueFamilyIndices = &familyIndice;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if (vkCreateImage(core.device, &imageInfo, core.allocator, &image) != VK_SUCCESS)
			throw std::runtime_error("Failed to create VkImage");
	}

	void allocateImage(ctm::VkCore &core, VkDeviceMemory &memory, VkImage &image)
	{
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(core.device, image, &memRequirements);

		VkMemoryAllocateInfo memoryInfo = {};
		memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryInfo.allocationSize = memRequirements.size;
		memoryInfo.memoryTypeIndex = ctm::VkUtils::findMemoryType(core, memRequirements.memoryTypeBits, 
																  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		if (vkAllocateMemory(core.device, &memoryInfo, core.allocator, &memory) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate memory");
		if (vkBindImageMemory(core.device, image, memory, 0) != VK_SUCCESS)
			throw std::runtime_error("Failed to bind image memory");
	}

	void createImageView(ctm::VkCore &core, VkImageView &imageView, VkImage &image)
	{
		VkImageViewCreateInfo imageViewInfo = {};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.image = image;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;
		if (vkCreateImageView(core.device, &imageViewInfo, core.allocator, &imageView) != VK_SUCCESS)
			throw std::runtime_error("Failed to create VkImage");
	}

	void createFrameBuffer(ctm::VkCore &core, ctm::VkRasterizer &rast, VkFramebuffer &frame, VkImageView &imageView)
	{
		VkFramebufferCreateInfo frameInfo = {};
		frameInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameInfo.renderPass = rast.renderPass;
		frameInfo.attachmentCount = 1;
		frameInfo.pAttachments = &imageView;
		frameInfo.width = rast.extent.width;
		frameInfo.height = rast.extent.height;
		frameInfo.layers = 1;
		if (vkCreateFramebuffer(core.device, &frameInfo, core.allocator, &frame) != VK_SUCCESS)
			throw std::runtime_error("Failed to create VkFrameBuffer");
	}

	void createCommandPool(ctm::VkRasterizer &rast, ctm::VkCore &core)
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = core.queue.graphicsIdx;
		if (vkCreateCommandPool(core.device, &poolInfo, core.allocator, &rast.commandPool) != VK_SUCCESS)
			throw std::runtime_error("Failed to create VkCommandPool");

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = rast.commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(rast.commandBuffers.size());

		if (vkAllocateCommandBuffers(core.device, &allocInfo, rast.commandBuffers.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate VkCommandBuffers");
	}
}

void ctm::VkRasterizer::init(ctm::VkRasterizer &rast, ctm::VkCore &core, VkExtent2D extent, uint32_t imageCount)
{
	rast.imageCount = imageCount;
	rast.extent = extent;

	rast.images.resize(imageCount);
	rast.imageMemories.resize(imageCount);
	rast.imageViews.resize(imageCount);
	rast.frameBuffers.resize(imageCount);
	rast.commandBuffers.resize(imageCount);

	createRenderPass(rast, core);
	createPipeline(rast, core);
	createCommandPool(rast, core);
	for (uint32_t i = 0; i < imageCount; i++)
	{
		createImage(rast, core, rast.images[i]);
		allocateImage(core, rast.imageMemories[i], rast.images[i]);
		createImageView(core, rast.imageViews[i], rast.images[i]);
		createFrameBuffer(core, rast, rast.frameBuffers[i], rast.imageViews[i]);
	}
}

void ctm::VkRasterizer::destroy(ctm::VkRasterizer &rast, ctm::VkCore &core)
{
	vkFreeCommandBuffers(core.device, rast.commandPool, static_cast<uint32_t>(rast.commandBuffers.size()), rast.commandBuffers.data());

	vkDestroyCommandPool(core.device, rast.commandPool, core.allocator);
	vkDestroyPipeline(core.device, rast.pipeline, core.allocator);
	vkDestroyPipelineLayout(core.device, rast.pipelineLayout, core.allocator);
	vkDestroyRenderPass(core.device, rast.renderPass, core.allocator);

	for (uint32_t i = 0; i < rast.imageCount; i++)
	{
		vkDestroyFramebuffer(core.device, rast.frameBuffers[i], core.allocator);
		vkDestroyImageView(core.device, rast.imageViews[i], core.allocator);
		vkFreeMemory(core.device, rast.imageMemories[i], core.allocator);
		vkDestroyImage(core.device, rast.images[i], core.allocator);
	}
}