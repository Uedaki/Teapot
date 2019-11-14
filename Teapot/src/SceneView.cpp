#include "SceneView.h"

#include <stdexcept>

#include "ctm/TransformMatrix.h"

teapot::SceneView::SceneView(ctm::VkCore &vCore)
	: vCore(vCore)
{}

void teapot::SceneView::init(teapot::Mesh &mesh, VkDescriptorPool &targetDescriptorPool, uint32_t width, uint32_t height, uint32_t imageCount)
{
	if (vRasterizer.extent.width != 0 && vRasterizer.extent.height != 0)
		destroy();

	ctm::VkRasterizer::init(vRasterizer, vCore, { width, height }, imageCount);

	descriptorSets.resize(vRasterizer.imageCount);
	semaphores.resize(vRasterizer.imageCount);
	fences.resize(vRasterizer.imageCount);
	outDescriptorSets.resize(vRasterizer.imageCount);

	createImageSampler();
	createDescriptorPool();
	createOutputDescriptorLayout();

	for (uint32_t i = 0; i < vRasterizer.imageCount; i++)
	{
		createDescriptorSet(descriptorSets[i], mesh);
		recordCommandBuffer(vRasterizer.commandBuffers[i], mesh, vRasterizer.images[i], vRasterizer.frameBuffers[i], descriptorSets[i]);
		createSyncObj(semaphores[i], fences[i]);
		createOutputDescriptorSet(outDescriptorSets[i], targetDescriptorPool, vRasterizer.imageViews[i]);
	}
}

void teapot::SceneView::destroy()
{
	for (uint32_t i = 0; i < vRasterizer.imageCount; i++)
	{
		vkDestroySemaphore(vCore.device, semaphores[i], vCore.allocator);
		vkDestroyFence(vCore.device, fences[i], vCore.allocator);
	}

	vkDestroyDescriptorSetLayout(vCore.device, outDescriptorSetLayout, vCore.allocator);

	vkDestroyDescriptorPool(vCore.device, descriptorPool, vCore.allocator);
	vkDestroySampler(vCore.device, imageSampler, vCore.allocator);

	ctm::VkRasterizer::destroy(vRasterizer, vCore);
}

void teapot::SceneView::createImageSampler()
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
	if (vkCreateSampler(vCore.device, &samplerInfo, vCore.allocator, &imageSampler) != VK_SUCCESS)
		throw std::runtime_error("Failed to create VkSampler");
}

void teapot::SceneView::createDescriptorPool()
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = vRasterizer.imageCount;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = vRasterizer.imageCount;
	if (vkCreateDescriptorPool(vCore.device, &poolInfo, vCore.allocator, &descriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create VkDescriptorPool");
}

void teapot::SceneView::createOutputDescriptorLayout()
{
	VkDescriptorSetLayoutBinding binding = {};
	binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding.descriptorCount = 1;
	binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	binding.pImmutableSamplers = &imageSampler;
	VkDescriptorSetLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = 1;
	info.pBindings = &binding;
	if (vkCreateDescriptorSetLayout(vCore.device, &info, vCore.allocator, &outDescriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout");
}

void teapot::SceneView::createDescriptorSet(VkDescriptorSet &descriptorSet, teapot::Mesh &mesh)
{
	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = descriptorPool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &vRasterizer.descriptorSetLayout;
	if (vkAllocateDescriptorSets(vCore.device, &alloc_info, &descriptorSet) != VK_SUCCESS)
		throw std::runtime_error("Failed to create VkDescriptorSet");

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = mesh.transformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(ctm::TransformMatrix);

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;
	vkUpdateDescriptorSets(vCore.device, 1, &descriptorWrite, 0, nullptr);
}

void teapot::SceneView::createOutputDescriptorSet(VkDescriptorSet &outDescriptorSet, VkDescriptorPool &targetDescriptorPool, VkImageView &imageView)
{
	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = targetDescriptorPool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &outDescriptorSetLayout;
	if (vkAllocateDescriptorSets(vCore.device, &alloc_info, &outDescriptorSet) != VK_SUCCESS)
		throw std::runtime_error("Failed to create VkDescriptorSet");

	VkDescriptorImageInfo descImage = {};
	descImage.sampler = imageSampler;
	descImage.imageView = imageView;
	descImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkWriteDescriptorSet writeDesc = {};
	writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDesc.dstSet = outDescriptorSet;
	writeDesc.descriptorCount = 1;
	writeDesc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDesc.pImageInfo = &descImage;
	vkUpdateDescriptorSets(vCore.device, 1, &writeDesc, 0, nullptr);
}

void teapot::SceneView::createSyncObj(VkSemaphore &semaphore, VkFence &fence)
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(vCore.device, &semaphoreInfo, vCore.allocator, &semaphore))
		throw std::runtime_error("Failed to create VkSemaphore");

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	if (vkCreateFence(vCore.device, &fenceInfo, vCore.allocator, &fence) != VK_SUCCESS)
		throw std::runtime_error("Failed to create VkFence");
}

void teapot::SceneView::render()
{
	currentImage = (currentImage + 1) % vRasterizer.imageCount;

	vkWaitForFences(vCore.device, 1, &fences[currentImage], VK_TRUE, UINT64_MAX);
	vkResetFences(vCore.device, 1, &fences[currentImage]);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = 0;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vRasterizer.commandBuffers[currentImage];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &semaphores[currentImage];
	if (vkQueueSubmit(vCore.queue.graphics, 1, &submitInfo, fences[currentImage]) != VK_SUCCESS)
		throw std::runtime_error("failed to submit draw command buffer!");
}

void teapot::SceneView::recordCommandBuffer(VkCommandBuffer &commandBuffer, teapot::Mesh &mesh, VkImage &image, VkFramebuffer &frame, VkDescriptorSet &descriptorSet)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin recording command buffer");

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = vRasterizer.renderPass;
	renderPassInfo.framebuffer = frame;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = vRasterizer.extent;

	VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	VkDeviceSize p = 0;
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vRasterizer.pipeline);
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vBuffer, &p);
	vkCmdBindIndexBuffer(commandBuffer, mesh.iBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vRasterizer.pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
	vkCmdDrawIndexed(commandBuffer, (uint32_t)mesh.indices.size(), 1, 0, 0, 0);

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

bool teapot::SceneView::needToBeResized(uint32_t newWidth, uint32_t newHeight) const
{
	if (vRasterizer.extent.width != newWidth || vRasterizer.extent.height != newHeight)
		return (true);
	return (false);
}

void teapot::SceneView::SceneGui::drawGui()
{

}