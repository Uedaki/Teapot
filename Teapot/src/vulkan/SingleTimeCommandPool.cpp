#include "vulkan/SingleTimeCommandPool.h"

#include "Application.h"

void teapot::vk::SingleTimeCommandPool::init(VkQueue queue, uint32_t queueFamily)
{
	this->queue = &queue;

	Application &app = Application::get();
	vk::Context &vulkan = app.getVulkan();

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamily;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(vulkan.device, &poolInfo, vulkan.allocator, &commandPool));

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	VK_CHECK_RESULT(vkAllocateCommandBuffers(vulkan.device, &allocInfo, &commandBuffer));

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VK_CHECK_RESULT(vkCreateFence(vulkan.device, &fenceInfo, vulkan.allocator, &fence));
}

void teapot::vk::SingleTimeCommandPool::destroy()
{
	Application &app = Application::get();
	vk::Context &vulkan = app.getVulkan();

	vkWaitForFences(vulkan.device, 1, &fence, true, UINT64_MAX);
	
	vkDestroyFence(vulkan.device, fence, vulkan.allocator);
	vkFreeCommandBuffers(vulkan.device, commandPool, 1, &commandBuffer);
	vkDestroyCommandPool(vulkan.device, commandPool, vulkan.allocator);
}

VkCommandBuffer &teapot::vk::SingleTimeCommandPool::startRecording()
{
	Application &app = Application::get();
	vk::Context &vulkan = app.getVulkan();

	vkWaitForFences(vulkan.device, 1, &fence, true, UINT64_MAX);
	vkResetFences(vulkan.device, 1, &fence);

	vkResetCommandPool(vulkan.device, commandPool, 0);

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &begin_info);

	return (commandBuffer);
}

void teapot::vk::SingleTimeCommandPool::finishAndSubmit()
{
	Application &app = Application::get();
	vk::Context &vulkan = app.getVulkan();

	VkSubmitInfo end_info = {};
	end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	end_info.commandBufferCount = 1;
	end_info.pCommandBuffers = &commandBuffer;
	vkEndCommandBuffer(commandBuffer);

	vkQueueSubmit(*queue, 1, &end_info, fence);
}