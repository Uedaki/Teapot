#include "SceneView.h"

#include <stdexcept>

namespace
{
	void createImageSampler(teapot::SceneView &scene, ctm::VkCore &core)
	{
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
		if (vkCreateSampler(core.device, &samplerInfo, core.allocator, &scene.imageSampler) != VK_SUCCESS)
			throw std::runtime_error("failed to create texture sampler!");
	}

	void createDescriptorSetLayout(teapot::SceneView &scene, ctm::VkCore &core)
	{
		VkDescriptorSetLayoutBinding binding = {};
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		binding.pImmutableSamplers = &scene.imageSampler;
		VkDescriptorSetLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = 1;
		info.pBindings = &binding;
		if (vkCreateDescriptorSetLayout(core.device, &info, core.allocator, &scene.descriptorSetLayout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create descriptor set layout");
	}

	void recordCommandBuffer(ctm::VkRasterizer &rast, VkCommandBuffer &commandBuffer, VkImage &image, VkFramebuffer &frame)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to begin recording command buffer");

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = rast.renderPass;
		renderPassInfo.framebuffer = frame;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = rast.extent;

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, rast.pipeline);
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
							 0, 0, nullptr, 0, nullptr, 1, &barrier);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to record command buffer!");
	}

	void createDescriptorSet(teapot::SceneView &scene, ctm::VkCore &core, VkDescriptorPool &descriptorPool, uint32_t i)
	{
		VkDescriptorSetAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = descriptorPool;
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &scene.descriptorSetLayout;
		if (vkAllocateDescriptorSets(core.device, &alloc_info, &scene.descriptorSets[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create VkDescriptorSet");

		VkDescriptorImageInfo desc_image[1] = {};
		desc_image[0].sampler = scene.imageSampler;
		desc_image[0].imageView = scene.imageViews[i];
		desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkWriteDescriptorSet write_desc[1] = {};
		write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_desc[0].dstSet = scene.descriptorSets[i];
		write_desc[0].descriptorCount = 1;
		write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write_desc[0].pImageInfo = desc_image;
		vkUpdateDescriptorSets(core.device, 1, write_desc, 0, nullptr);
	}
}

void teapot::SceneView::init(teapot::SceneView &scene, ctm::VkCore &core, VkDescriptorPool &descriptorPool, VkExtent2D extent, uint32_t imageCount)
{
	ctm::VkRasterizer::init(scene, core, extent, imageCount);

	createImageSampler(scene, core);
	createDescriptorSetLayout(scene, core);
	scene.fences.resize(imageCount);
	scene.signalSemaphores.resize(imageCount);
	scene.descriptorSets.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; i++)
	{
		recordCommandBuffer(scene, scene.commandBuffers[i], scene.images[i], scene.frameBuffers[i]);
		createDescriptorSet(scene, core, descriptorPool, i);

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		if (vkCreateFence(core.device, &fenceInfo, core.allocator, &scene.fences[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create fence");

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		if (vkCreateSemaphore(core.device, &semaphoreInfo, core.allocator, &scene.signalSemaphores[i]))
			throw std::runtime_error("Failed to create semaphore");
	}
}

void teapot::SceneView::render(teapot::SceneView &scene, ctm::VkCore &core, uint32_t imageIdx)
{
	vkWaitForFences(core.device, 1, &scene.fences[imageIdx], VK_TRUE, UINT64_MAX);
	vkResetFences(core.device, 1, &scene.fences[imageIdx]);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = 0;
	submitInfo.pWaitDstStageMask = nullptr;
	
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &scene.commandBuffers[imageIdx];

	VkSemaphore signalSemaphores[] = { scene.signalSemaphores[imageIdx] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(core.queue.graphics, 1, &submitInfo, scene.fences[imageIdx]) != VK_SUCCESS)
		throw std::runtime_error("failed to submit draw command buffer!");
}

void teapot::SceneView::destroy(teapot::SceneView &scene, ctm::VkCore &core)
{
	for (uint32_t i = 0; i < scene.imageCount; i++)
	{
		vkDestroySemaphore(core.device, scene.signalSemaphores[i], core.allocator);
		vkDestroyFence(core.device, scene.fences[i], core.allocator);
	}

	vkDestroyDescriptorSetLayout(core.device, scene.descriptorSetLayout, core.allocator);
	vkDestroySampler(core.device, scene.imageSampler, core.allocator);
	ctm::VkRasterizer::destroy(scene, core);
}