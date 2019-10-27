#include "ImguiWrapper.h"

#include <glfw/glfw3.h>

#include <stdexcept>

#include "imgui_impl_glfw.h"

#include "ctm/VkCore.h"
#include "SceneView.h"

namespace
{
	void uploadFont(teapot::ImguiWrapper &wrapper, ctm::VkCore &core)
	{
		VkCommandPool command_pool = wrapper.wd.Frames[wrapper.wd.FrameIndex].CommandPool;
		VkCommandBuffer command_buffer = wrapper.wd.Frames[wrapper.wd.FrameIndex].CommandBuffer;

		vkResetCommandPool(core.device, command_pool, 0);
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(command_buffer, &begin_info);

		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

		VkSubmitInfo end_info = {};
		end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		end_info.commandBufferCount = 1;
		end_info.pCommandBuffers = &command_buffer;
		vkEndCommandBuffer(command_buffer);

		vkQueueSubmit(core.queue.present, 1, &end_info, VK_NULL_HANDLE);

		vkDeviceWaitIdle(core.device);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}

void teapot::ImguiWrapper::init(teapot::ImguiWrapper &wrapper, GLFWwindow *win, ctm::VkCore &core)
{
	int w, h;
	glfwGetFramebufferSize(win, &w, &h);

	VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
	poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
	poolInfo.pPoolSizes = poolSizes;
	if (vkCreateDescriptorPool(core.device, &poolInfo, core.allocator, &wrapper.descriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor pool");

	wrapper.wd.Surface = core.surface;
	wrapper.wd.SurfaceFormat = core.config.surfaceFormat;
	wrapper.wd.PresentMode = core.config.presentMode;

	ImGui_ImplVulkanH_CreateWindow(core.instance, core.physicalDevice, core.device, &wrapper.wd, core.queue.presentIdx, core.allocator, w, h, 2);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(win, true);
	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = core.instance;
	initInfo.PhysicalDevice = core.physicalDevice;
	initInfo.Device = core.device;
	initInfo.QueueFamily = core.queue.presentIdx;
	initInfo.Queue = core.queue.present;
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = wrapper.descriptorPool;
	initInfo.Allocator = core.allocator;
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = wrapper.wd.ImageCount;
	ImGui_ImplVulkan_Init(&initInfo, wrapper.wd.RenderPass);

	uploadFont(wrapper, core);
}

void teapot::ImguiWrapper::destroy(teapot::ImguiWrapper &wrapper, ctm::VkCore &vCore)
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	ImGui_ImplVulkanH_DestroyWindow(vCore.instance, vCore.device, &wrapper.wd, vCore.allocator);
	vkDestroyDescriptorPool(vCore.device, wrapper.descriptorPool, vCore.allocator);
}

void teapot::ImguiWrapper::newFrame(teapot::ImguiWrapper &wrapper)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void teapot::ImguiWrapper::rebuildSwapChain(teapot::ImguiWrapper &wrapper, GLFWwindow *win, ctm::VkCore &vCore)
{
	int w;
	int h;
	glfwGetWindowSize(win, &w, &h);
	ImGui_ImplVulkan_SetMinImageCount(2);
	ImGui_ImplVulkanH_CreateWindow(vCore.instance, vCore.physicalDevice, vCore.device, &wrapper.wd, vCore.queue.presentIdx, vCore.allocator, w, h, wrapper.wd.ImageCount);
}

void teapot::ImguiWrapper::render(teapot::ImguiWrapper &wrapper, VkSemaphore sem, ctm::VkCore &vCore)
{
	ImGui::Render();

	VkSemaphore image_acquired_semaphore = wrapper.wd.FrameSemaphores[wrapper.wd.SemaphoreIndex].ImageAcquiredSemaphore;
	VkSemaphore render_complete_semaphore = wrapper.wd.FrameSemaphores[wrapper.wd.SemaphoreIndex].RenderCompleteSemaphore;
	vkAcquireNextImageKHR(vCore.device, wrapper.wd.Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wrapper.wd.FrameIndex);

	ImGui_ImplVulkanH_Frame *fd = &wrapper.wd.Frames[wrapper.wd.FrameIndex];
	{
		vkWaitForFences(vCore.device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);

		vkResetFences(vCore.device, 1, &fd->Fence);
	}
	{
		vkResetCommandPool(vCore.device, fd->CommandPool, 0);
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(fd->CommandBuffer, &info);
	}
	{
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = wrapper.wd.RenderPass;
		info.framebuffer = fd->Framebuffer;
		info.renderArea.extent.width = wrapper.wd.Width;
		info.renderArea.extent.height = wrapper.wd.Height;
		info.clearValueCount = 1;
		info.pClearValues = &wrapper.wd.ClearValue;
		vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), fd->CommandBuffer);

	vkCmdEndRenderPass(fd->CommandBuffer);
	{
		VkSubmitInfo info = {};
		VkSemaphore waitSem[2] = { image_acquired_semaphore, sem };
		VkPipelineStageFlags waitStage[2] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT };
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.waitSemaphoreCount = 2;
		info.pWaitSemaphores = waitSem;
		info.pWaitDstStageMask = waitStage;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &fd->CommandBuffer;
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = &render_complete_semaphore;

		vkEndCommandBuffer(fd->CommandBuffer);
		vkQueueSubmit(vCore.queue.graphics, 1, &info, fd->Fence);
	}

	VkPresentInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.waitSemaphoreCount = 1;
	info.pWaitSemaphores = &render_complete_semaphore;
	info.swapchainCount = 1;
	info.pSwapchains = &wrapper.wd.Swapchain;
	info.pImageIndices = &wrapper.wd.FrameIndex;
	vkQueuePresentKHR(vCore.queue.graphics, &info);
	wrapper.wd.SemaphoreIndex = (wrapper.wd.SemaphoreIndex + 1) % wrapper.wd.ImageCount; // Now we can use the next set of semaphores
}