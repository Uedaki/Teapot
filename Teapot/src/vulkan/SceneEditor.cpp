#include "vulkan/SceneEditor.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Application.h"
#include "Log.h"
#include "vulkan/Utils.h"

void teapot::vk::SceneEditor::init()
{
	createRenderPass();
	createDescriptorSetLayout();

	createOutputImageSampler();
	createOutputDescriptorSetLayout();

	LOG_MSG("Scene editor loaded");
}

void teapot::vk::SceneEditor::destroy()
{
	vk::Context &vulkan = Application::get().getVulkan();

	if (extent.width != 0)
	{
		destroySceneView(sceneView);

		mainPipeline.destroy();
		if (overlayPipeline)
			overlayPipeline.destroy();
	}

	vkDestroyDescriptorSetLayout(vulkan.device, outDescriptorSetLayout, vulkan.allocator);
	vkDestroySampler(vulkan.device, outImageSampler, vulkan.allocator);

	vkDestroyRenderPass(vulkan.device, renderPass, vulkan.allocator);
}

void teapot::vk::SceneEditor::updateExtent(uint32_t width, uint32_t height)
{
	if (extent.width == width && extent.height == height)
		return;
	else if (extent.width != 0)
	{
		destroySceneView(sceneView);

		mainPipeline.destroy();
		if (overlayPipeline)
			overlayPipeline.destroy();
	}

	extent.width = width;
	extent.height = height;

	mainPipeline.init(renderPass, descriptorSetLayout,
					  VK_POLYGON_MODE_FILL, extent,
					  "shader/rasterizer.vert.spv", "shader/rasterizer.frag.spv");
	createSceneView(sceneView);
}

void teapot::vk::SceneEditor::changeOverlay(DisplayMode newMode)
{
	mode = newMode;
	if (mode == DisplayMode::FACE)
	{
		overlayPipeline.init(renderPass, descriptorSetLayout,
							 VK_POLYGON_MODE_FILL, extent,
							 "shader/rasterizer.vert.spv", "shader/rasterizerFace.frag.spv");
	}
	else if (mode == DisplayMode::EDGE)
	{
		overlayPipeline.init(renderPass, descriptorSetLayout,
							 VK_POLYGON_MODE_LINE, extent,
							 "shader/rasterizer.vert.spv", "shader/rasterizerLine.frag.spv");
	}
	else if (mode == DisplayMode::VERTEX)
	{
		overlayPipeline.init(renderPass, descriptorSetLayout,
							 VK_POLYGON_MODE_POINT, extent,
							 "shader/rasterizer.vert.spv", "shader/rasterizerPoint.frag.spv");
	}
}

void teapot::vk::SceneEditor::deleteOverlay()
{
	mode = DisplayMode::NONE;
	overlayPipeline.destroy();
}

VkDescriptorSet &teapot::vk::SceneEditor::getDescriptorSet()
{
	return (sceneView.outDescriptorSet[Application::get().getVulkan().swapchainInfo.currImg]);
}

void teapot::vk::SceneEditor::renderViews(VkCommandBuffer &commandBuffer)
{
	vk::Context &vulkan = Application::get().getVulkan();

	VkClearValue clearColor = { 0.27f, 0.28f, 0.26f, 1.0f };
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = sceneView.frameBuffers[vulkan.swapchainInfo.currImg];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = extent;
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mainPipeline.get());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mainPipeline.getLayout(), 0, 1, &sceneView.descriptorSet[vulkan.swapchainInfo.currImg], 0, nullptr);
	Application::get().getCollection().mesh.draw(commandBuffer, mainPipeline.getLayout());

	if (overlayPipeline)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, overlayPipeline.get());

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, overlayPipeline.getLayout(), 0, 1, &sceneView.descriptorSet[vulkan.swapchainInfo.currImg], 0, nullptr);
		Application::get().getCollection().mesh.draw(commandBuffer, overlayPipeline.getLayout());
	}

	vkCmdEndRenderPass(commandBuffer);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = sceneView.images[vulkan.swapchainInfo.currImg];
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
						 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void teapot::vk::SceneEditor::createRenderPass()
{
	vk::Context &vulkan = Application::get().getVulkan();

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
	if (vkCreateRenderPass(vulkan.device, &renderPassInfo, vulkan.allocator, &renderPass) != VK_SUCCESS)
		throw std::runtime_error("Failed to create VkRenderPass");
}

void teapot::vk::SceneEditor::createDescriptorSetLayout()
{
	vk::Context &vulkan = Application::get().getVulkan();

	VkDescriptorSetLayoutBinding binding = {};
	binding.binding = 0;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	binding.descriptorCount = 1;
	binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	binding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = 1;
	info.pBindings = &binding;
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(vulkan.device, &info, vulkan.allocator, &descriptorSetLayout));
}

void teapot::vk::SceneEditor::createOutputImageSampler()
{
	vk::Context &vulkan = Application::get().getVulkan();

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	VK_CHECK_RESULT(vkCreateSampler(vulkan.device, &samplerInfo, vulkan.allocator, &outImageSampler))
}

void teapot::vk::SceneEditor::createOutputDescriptorSetLayout()
{
	vk::Context &vulkan = Application::get().getVulkan();

	VkDescriptorSetLayoutBinding binding = {};
	binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding.descriptorCount = 1;
	binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	binding.pImmutableSamplers = &outImageSampler;
	VkDescriptorSetLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = 1;
	info.pBindings = &binding;
	if (vkCreateDescriptorSetLayout(vulkan.device, &info, vulkan.allocator, &outDescriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout");
}

void teapot::vk::SceneEditor::createSceneView(teapot::vk::SceneEditor::SceneView &view)
{
	vk::Context &vulkan = Application::get().getVulkan();

	view.view = glm::lookAt(glm::vec3(5, 5, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	view.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(extent.width) / extent.height, 0.1f, 100.0f);
	view.proj[1][1] *= -1;

	createCameraBuffer(view);

	view.images.resize(vulkan.swapchainInfo.imgCount);
	view.imageMemories.resize(vulkan.swapchainInfo.imgCount);
	view.imageViews.resize(vulkan.swapchainInfo.imgCount);
	view.frameBuffers.resize(vulkan.swapchainInfo.imgCount);
	view.outDescriptorSet.resize(vulkan.swapchainInfo.imgCount);
	for (uint32_t i = 0; i < vulkan.swapchainInfo.imgCount; i++)
	{
		createImage(view.images[i]);
		allocateImage(view.imageMemories[i], view.images[i]);
		createImageView(view.imageViews[i], view.images[i]);
		createFrameBuffer(view.frameBuffers[i], view.imageViews[i], renderPass);
		createOutputDescriptorSet(view.outDescriptorSet[i], view.imageViews[i]);
	}

	createDescriptorPool(view);
	createDescriptorSet(view);
}

#include <iostream>

void teapot::vk::SceneEditor::createCameraBuffer(teapot::vk::SceneEditor::SceneView &view)
{
	vk::Context &vulkan = Application::get().getVulkan();

	glm::mat4 mat[2];
	mat[0] = view.view;
	mat[1] = view.proj;

	void *data;
	Utils::createBuffer(view.cameraBuffer, view.cameraBufferMemory, 2 * sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	vkMapMemory(vulkan.device, view.cameraBufferMemory, 0, static_cast<uint32_t>(2 * sizeof(glm::mat4)), 0, &data);
	memcpy(data, mat, static_cast<uint32_t>(2 * sizeof(glm::mat4)));
	vkUnmapMemory(vulkan.device, view.cameraBufferMemory);
}

void teapot::vk::SceneEditor::createDescriptorPool(teapot::vk::SceneEditor::SceneView &view)
{
	vk::Context &vulkan = Application::get().getVulkan();

	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = vulkan.swapchainInfo.imgCount;

	std::vector<VkDescriptorPoolSize> poolSizes(vulkan.swapchainInfo.imgCount, poolSize);

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = vulkan.swapchainInfo.imgCount;
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = vulkan.swapchainInfo.imgCount;
	if (vkCreateDescriptorPool(vulkan.device, &poolInfo, vulkan.allocator, &view.descriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create VkDescriptorPool");
}

void teapot::vk::SceneEditor::createImage(VkImage &image)
{
	vk::Context &vulkan = Application::get().getVulkan();

	uint32_t familyIndice = vulkan.queue.graphicsFamily;
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.extent.width = extent.width;
	imageInfo.extent.height = extent.height;
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
	VK_CHECK_RESULT(vkCreateImage(vulkan.device, &imageInfo, vulkan.allocator, &image));
}

void teapot::vk::SceneEditor::allocateImage(VkDeviceMemory &memory, VkImage &image)
{
	vk::Context &vulkan = Application::get().getVulkan();

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(vulkan.device, image, &memRequirements);

	VkMemoryAllocateInfo memoryInfo = {};
	memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryInfo.allocationSize = memRequirements.size;
	memoryInfo.memoryTypeIndex = vk::Utils::findMemoryType(memRequirements.memoryTypeBits,
														   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(vulkan.device, &memoryInfo, vulkan.allocator, &memory));
	VK_CHECK_RESULT(vkBindImageMemory(vulkan.device, image, memory, 0));
}

void teapot::vk::SceneEditor::createImageView(VkImageView &imageView, VkImage &image)
{
	vk::Context &vulkan = Application::get().getVulkan();

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
	VK_CHECK_RESULT(vkCreateImageView(vulkan.device, &imageViewInfo, vulkan.allocator, &imageView));
}

void teapot::vk::SceneEditor::createFrameBuffer(VkFramebuffer &frame, VkImageView &imageView, VkRenderPass &renderPass)
{
	vk::Context &vulkan = Application::get().getVulkan();

	VkFramebufferCreateInfo frameInfo = {};
	frameInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameInfo.renderPass = renderPass;
	frameInfo.attachmentCount = 1;
	frameInfo.pAttachments = &imageView;
	frameInfo.width = extent.width;
	frameInfo.height = extent.height;
	frameInfo.layers = 1;
	VK_CHECK_RESULT(vkCreateFramebuffer(vulkan.device, &frameInfo, vulkan.allocator, &frame));
}

void teapot::vk::SceneEditor::createDescriptorSet(teapot::vk::SceneEditor::SceneView &view)
{
	vk::Context &vulkan = Application::get().getVulkan();

	std::vector<VkDescriptorSetLayout> layouts(vulkan.swapchainInfo.imgCount, descriptorSetLayout);

	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = view.descriptorPool;
	alloc_info.descriptorSetCount = vulkan.swapchainInfo.imgCount;
	alloc_info.pSetLayouts = layouts.data();

	view.descriptorSet.resize(vulkan.swapchainInfo.imgCount);
	if (vkAllocateDescriptorSets(vulkan.device, &alloc_info, view.descriptorSet.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to create VkDescriptorSet");

	VkWriteDescriptorSet descriptorWriteTemplate = {};
	descriptorWriteTemplate.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWriteTemplate.dstBinding = 0;
	descriptorWriteTemplate.dstArrayElement = 0;
	descriptorWriteTemplate.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWriteTemplate.descriptorCount = 1;
	std::vector<VkWriteDescriptorSet> descriptorWrites(vulkan.swapchainInfo.imgCount, descriptorWriteTemplate);

	VkDescriptorBufferInfo cameraInfoTemplate = {};
	cameraInfoTemplate.offset = 0;
	cameraInfoTemplate.range = 2 * sizeof(glm::mat4);
	std::vector<VkDescriptorBufferInfo> cameraInfos(vulkan.swapchainInfo.imgCount, cameraInfoTemplate);

	for (uint32_t i = 0; i < vulkan.swapchainInfo.imgCount; i++)
	{
		cameraInfos[i].buffer = view.cameraBuffer;

		descriptorWrites[i].dstSet = view.descriptorSet[i];
		descriptorWrites[i].pBufferInfo = &cameraInfos[i];
	}

	vkUpdateDescriptorSets(vulkan.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void teapot::vk::SceneEditor::createOutputDescriptorSet(VkDescriptorSet &outDescriptorSet, VkImageView &imageView)
{
	vk::Context &vulkan = Application::get().getVulkan();

	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = Application::get().getGui().getDescriptorPool();
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &outDescriptorSetLayout;
	if (vkAllocateDescriptorSets(vulkan.device, &alloc_info, &outDescriptorSet) != VK_SUCCESS)
		throw std::runtime_error("Failed to create VkDescriptorSet");

	VkDescriptorImageInfo descImage = {};
	descImage.sampler = outImageSampler;
	descImage.imageView = imageView;
	descImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkWriteDescriptorSet writeDesc = {};
	writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDesc.dstSet = outDescriptorSet;
	writeDesc.descriptorCount = 1;
	writeDesc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDesc.pImageInfo = &descImage;
	vkUpdateDescriptorSets(vulkan.device, 1, &writeDesc, 0, nullptr);
}

void teapot::vk::SceneEditor::destroySceneView(teapot::vk::SceneEditor::SceneView &view)
{
	vk::Context &vulkan = Application::get().getVulkan();

	for (uint32_t i = 0; i < vulkan.swapchainInfo.imgCount; i++)
	{
		vkDestroyFramebuffer(vulkan.device, view.frameBuffers[i], vulkan.allocator);
		vkDestroyImageView(vulkan.device, view.imageViews[i], vulkan.allocator);
		vkDestroyImage(vulkan.device, view.images[i], vulkan.allocator);
		vkFreeMemory(vulkan.device, view.imageMemories[i], vulkan.allocator);
	}

	vkFreeDescriptorSets(vulkan.device, Application::get().getGui().getDescriptorPool(),
						 static_cast<uint32_t>(view.outDescriptorSet.size()), view.outDescriptorSet.data());

	view.images.clear();
	view.imageMemories.clear();
	view.imageViews.clear();
	view.frameBuffers.clear();
	view.outDescriptorSet.clear();
}