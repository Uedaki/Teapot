#include "SceneView.h"

#include <glm/gtx/transform.hpp>

#include <stdexcept>

#include "ctm/VkUtils.h"
#include "ctm/TransformMatrix.h"
#include "profiler/Profiler.h"

teapot::SceneView::SceneView(ctm::VkCore &vCore)
	: vCore(vCore), pipelinePool(vCore)
{}

void teapot::SceneView::init(teapot::Mesh &mesh, VkDescriptorPool &targetDescriptorPool, uint32_t width, uint32_t height, uint32_t imageCount)
{
	PROFILE_FUNCTION("blop");

	if (rasterizerImg.extent.width != 0 && rasterizerImg.extent.height != 0)
		destroy();

	pipelinePool.init({width, height});
	mainPipeline = pipelinePool.createPipeline(VK_POLYGON_MODE_FILL, "shader/rasterizer.vert.spv", "shader/rasterizer.frag.spv");

	rasterizerImg.init(vCore, pipelinePool.getRenderPass(), { width, height }, imageCount);

	buffers.resize(rasterizerImg.imageCount);
	bufferMemories.resize(rasterizerImg.imageCount);

	semaphores.resize(rasterizerImg.imageCount);
	fences.resize(rasterizerImg.imageCount);
	
	commandBuffers.resize(rasterizerImg.imageCount);

	outDescriptorSets.resize(rasterizerImg.imageCount);

	createOutputDescriptorLayout();
	createCommandPool();
	mesh.allocDescriptorSet(pipelinePool.getDescriptorSetLayout());
	for (uint32_t i = 0; i < rasterizerImg.imageCount; i++)
	{
		ctm::VkUtils::createBuffer(vCore, 2 * sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffers[i], bufferMemories[i]);
		void *data;
		vkMapMemory(vCore.device, bufferMemories[i], 0, 2 * sizeof(glm::mat4), 0, &data);

		glm::mat4 mat[2];
		mat[0] = glm::lookAt(glm::vec3(5, 5, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));;
		mat[1] = glm::perspective(glm::radians(45.0f), static_cast<float>(rasterizerImg.extent.width) / rasterizerImg.extent.height, 0.1f, 100.0f);
		mat[1][1][1] *= -1;

		memcpy(data, mat, 2 * sizeof(glm::mat4));
		vkUnmapMemory(vCore.device, bufferMemories[i]);

		mesh.updateDescriptorSet(buffers[i], i);

		recordCommandBuffer(commandBuffers[i], mesh, rasterizerImg.images[i], rasterizerImg.frameBuffers[i], i);
		createSyncObj(semaphores[i], fences[i]);
		createOutputDescriptorSet(outDescriptorSets[i], targetDescriptorPool, rasterizerImg.imageViews[i]);
	}
}

void teapot::SceneView::destroy()
{
	PROFILE_FUNCTION("blop");

	for (uint32_t i = 0; i < rasterizerImg.imageCount; i++)
	{
		vkDestroyBuffer(vCore.device, buffers[i], vCore.allocator);
		vkFreeMemory(vCore.device, bufferMemories[i], vCore.allocator);

		vkDestroySemaphore(vCore.device, semaphores[i], vCore.allocator);
		vkDestroyFence(vCore.device, fences[i], vCore.allocator);
	}

	vkDestroyDescriptorSetLayout(vCore.device, outDescriptorSetLayout, vCore.allocator);
	vkDestroyCommandPool(vCore.device, commandPool, vCore.allocator);
	rasterizerImg.destroy(vCore);
	pipelinePool.destroy();
}

void teapot::SceneView::createCommandPool()
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = vCore.queue.graphicsIdx;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (vkCreateCommandPool(vCore.device, &poolInfo, vCore.allocator, &commandPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create VkCommandPool");

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

	if (vkAllocateCommandBuffers(vCore.device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate VkCommandBuffers");
}

void teapot::SceneView::recordCommandBuffer(VkCommandBuffer &commandBuffer, teapot::Mesh &mesh, VkImage &image, VkFramebuffer &frame, uint32_t imageIdx)
{
	PROFILE_FUNCTION("blop");

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin recording command buffer");

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = pipelinePool.getRenderPass();
	renderPassInfo.framebuffer = frame;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = rasterizerImg.extent;

	VkClearValue clearColor = { 0.27f, 0.28f, 0.26f, 1.0f };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	VkDeviceSize p = 0;
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mainPipeline.get());

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.getVertexBuffer(imageIdx), &p);
	vkCmdBindIndexBuffer(commandBuffer, mesh.getIndexBuffer(imageIdx), 0, VK_INDEX_TYPE_UINT32);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinePool.getPipelineLayout(), 0, 1, &mesh.descriptorSet[imageIdx], 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, (uint32_t)mesh.indices.size(), 1, 0, 0, 0);

	if (secondaryPipeline)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, secondaryPipeline.get());

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.getVertexBuffer(imageIdx), &p);
		vkCmdBindIndexBuffer(commandBuffer, mesh.getIndexBuffer(imageIdx), 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinePool.getPipelineLayout(), 0, 1, &mesh.descriptorSet[imageIdx], 0, nullptr);

		vkCmdDrawIndexed(commandBuffer, (uint32_t)mesh.indices.size(), 1, 0, 0, 0);
	}
	vkCmdEndRenderPass(commandBuffer);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
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

void teapot::SceneView::createOutputDescriptorLayout()
{
	VkDescriptorSetLayoutBinding binding = {};
	binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding.descriptorCount = 1;
	binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	binding.pImmutableSamplers = &rasterizerImg.imageSampler;
	VkDescriptorSetLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = 1;
	info.pBindings = &binding;
	if (vkCreateDescriptorSetLayout(vCore.device, &info, vCore.allocator, &outDescriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout");
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
	descImage.sampler = rasterizerImg.imageSampler;
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

void teapot::SceneView::render(teapot::Mesh &mesh)
{
	currentImage = (currentImage + 1) % rasterizerImg.imageCount;

	recordCommandBuffer(commandBuffers[currentImage], mesh, rasterizerImg.images[currentImage], rasterizerImg.frameBuffers[currentImage], currentImage);

	vkWaitForFences(vCore.device, 1, &fences[currentImage], VK_TRUE, UINT64_MAX);
	vkResetFences(vCore.device, 1, &fences[currentImage]);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = 0;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentImage];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &semaphores[currentImage];
	if (vkQueueSubmit(vCore.queue.graphics, 1, &submitInfo, fences[currentImage]) != VK_SUCCESS)
		throw std::runtime_error("failed to submit draw command buffer!");
}

bool teapot::SceneView::needToBeResized(uint32_t newWidth, uint32_t newHeight) const
{
	if (rasterizerImg.extent.width != newWidth || rasterizerImg.extent.height != newHeight)
		return (true);
	return (false);
}

void teapot::SceneView::SceneGui::drawGui()
{

}

void teapot::SceneView::changeMode(teapot::EditMode mode)
{
	VkPolygonMode polyMode;
	std::string vert;
	std::string frag;

	switch (mode)
	{
	case teapot::EditMode::FACE:
		polyMode = VK_POLYGON_MODE_FILL;
		vert = "shader/rasterizer.vert.spv";
		frag = "shader/rasterizer.frag.spv";
		break;
	case teapot::EditMode::EDGE:
		polyMode = VK_POLYGON_MODE_LINE;
		vert = "shader/rasterizer.vert.spv";
		frag = "shader/rasterizerLine.frag.spv";
		break;
	case teapot::EditMode::VERTEX:
		polyMode = VK_POLYGON_MODE_POINT;
		vert = "shader/rasterizer.vert.spv";
		frag = "shader/rasterizerPoint.frag.spv";
		break;
	}

	if (secondaryPipeline)
	{
		if (mode == EditMode::NONE)
		{
			secondaryPipeline.destroy();
			return;
		}
		secondaryPipeline.recreate(polyMode, vert, frag);
	}
	else if (mode != EditMode::NONE)
	{
		secondaryPipeline = pipelinePool.createPipeline(polyMode, vert, frag);
	}
}