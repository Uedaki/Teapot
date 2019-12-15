#include "vulkan/Command.h"

#include "Application.h"
#include "Profiler.h"
#include "vulkan/Context.h"

void teapot::vk::Command::init()
{
	Application &app = Application::get();
	vk::Context &vulkan = app.getVulkan();

	imageCount = vulkan.swapchainInfo.imgCount;  // app.getGui().getImageCount();

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = vulkan.queue.graphicsFamily;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(vulkan.device, &poolInfo, vulkan.allocator, &commandPool));

	commandBuffers.resize(imageCount);
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
	VK_CHECK_RESULT(vkAllocateCommandBuffers(vulkan.device, &allocInfo, commandBuffers.data()));

	createSyncObjects();
}

void teapot::vk::Command::destroy()
{
	Application &app = Application::get();
	vk::Context &vulkan = app.getVulkan();

	if (vkWaitForFences(vulkan.device, static_cast<uint32_t>(fences.size()), fences.data(), true, UINT64_MAX) != VK_SUCCESS)
		vkDeviceWaitIdle(vulkan.device);

	for (uint32_t i = 0; i < imageCount; i++)
	{
		vkDestroyFence(vulkan.device, fences[i], vulkan.allocator);
		vkDestroySemaphore(vulkan.device, submitSemaphores[i], vulkan.allocator);
		vkDestroySemaphore(vulkan.device, presentSemaphores[i], vulkan.allocator);
	}

	vkFreeCommandBuffers(vulkan.device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	vkDestroyCommandPool(vulkan.device, commandPool, vulkan.allocator);

	commandBuffers.clear();
}

void teapot::vk::Command::requestNextImage()
{
	vk::Context &vulkan = Application::get().getVulkan();

	VkResult result;
	uint32_t swapchainImageIdx = 0;
	result = vkAcquireNextImageKHR(vulkan.device, vulkan.swapchainInfo.swapchain, UINT64_MAX, submitSemaphores[currImage], VK_NULL_HANDLE, &swapchainImageIdx);
	vulkan.swapchainInfo.currImg = swapchainImageIdx;
}

VkCommandBuffer &teapot::vk::Command::recordNextBuffer()
{
	PROFILE_FUNCTION("Vulkan");

	Application &app = Application::get();
	vk::Context &vulkan = app.getVulkan();
	VkCommandBuffer &cmd = commandBuffers[currImage];

	vkWaitForFences(vulkan.device, 1, &fences[currImage], true, UINT64_MAX);
	vkResetFences(vulkan.device, 1, &fences[currImage]);

	//	vkResetCommandPool(vulkan.device, commandPool, 0);
	vkResetCommandBuffer(cmd, 0);

	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmd, &info);
	return (cmd);
}

void teapot::vk::Command::submitAndPresent()
{
	PROFILE_FUNCTION("Vulkan");

	Application &app = Application::get();
	vk::Context &vulkan = app.getVulkan();
	VkCommandBuffer &cmd = commandBuffers[currImage];

	vkEndCommandBuffer(cmd);

	VkSubmitInfo submitInfo = {};
	VkPipelineStageFlags waitStage[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &submitSemaphores[currImage];
	submitInfo.pWaitDstStageMask = waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currImage];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &presentSemaphores[currImage];
	vkQueueSubmit(vulkan.queue.graphics, 1, &submitInfo, fences[currImage]);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &presentSemaphores[currImage];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &vulkan.swapchainInfo.swapchain;
	presentInfo.pImageIndices = &vulkan.swapchainInfo.currImg;
	vkQueuePresentKHR(vulkan.queue.present, &presentInfo);

	currImage = (currImage + 1) % imageCount;
}

void teapot::vk::Command::createSyncObjects()
{
	Application &app = Application::get();
	vk::Context &vulkan = app.getVulkan();

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	fences.resize(imageCount);
	submitSemaphores.resize(imageCount);
	presentSemaphores.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; i++)
	{
		VK_CHECK_RESULT(vkCreateFence(vulkan.device, &fenceInfo, vulkan.allocator, &fences[i]));
		VK_CHECK_RESULT(vkCreateSemaphore(vulkan.device, &semaphoreInfo, vulkan.allocator, &submitSemaphores[i]))
		VK_CHECK_RESULT(vkCreateSemaphore(vulkan.device, &semaphoreInfo, vulkan.allocator, &presentSemaphores[i]))
	}
}