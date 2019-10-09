#include "WrapperImgui.h"

#include "imgui_impl_glfw.h"

#include "VulkanContext.h"

void WrapperImgui::init(GLFWwindow *win, VulkanContext &vulkan)
{
	int w, h;
	glfwGetFramebufferSize(win, &w, &h);

	wd.Surface = vulkan.getSurface();

	VkBool32 res;
	vkGetPhysicalDeviceSurfaceSupportKHR(vulkan.getPhysicalDevice(), vulkan.getQueueFamily(), wd.Surface, &res);

	const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
	const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

	VulkanContext::SwapChainSupportDetails swapChainSupport = vulkan.querySwapChainSupport(vulkan.getPhysicalDevice());
	wd.SurfaceFormat = vulkan.chooseSwapSurfaceFormat(swapChainSupport.formats);

	VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
	wd.PresentMode = vulkan.chooseSwapPresentMode(swapChainSupport.presentModes);
	
	ImGui_ImplVulkanH_CreateWindow(vulkan.getInstance(), vulkan.getPhysicalDevice(), vulkan.getDevice(), &wd, vulkan.getQueueFamily(), vulkan.getVulkanAllocator(), w, h, 2);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(win, true);
	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = vulkan.getInstance();
	initInfo.PhysicalDevice = vulkan.getPhysicalDevice();
	initInfo.Device = vulkan.getDevice();
	initInfo.QueueFamily = vulkan.getQueueFamily();
	initInfo.Queue = vulkan.getGraphicsQueue();
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = vulkan.getDescriptorPool();
	initInfo.Allocator = vulkan.getVulkanAllocator();
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = wd.ImageCount;
	ImGui_ImplVulkan_Init(&initInfo, wd.RenderPass);

	uploadFont(vulkan);
}

void WrapperImgui::newFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void WrapperImgui::render(VulkanContext &vulkan)
{
	ImGui::Render();

	VkSemaphore image_acquired_semaphore = wd.FrameSemaphores[wd.SemaphoreIndex].ImageAcquiredSemaphore;
	VkSemaphore render_complete_semaphore = wd.FrameSemaphores[wd.SemaphoreIndex].RenderCompleteSemaphore;
	vkAcquireNextImageKHR(vulkan.getDevice(), wd.Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd.FrameIndex);

	ImGui_ImplVulkanH_Frame *fd = &wd.Frames[wd.FrameIndex];
	{
		vkWaitForFences(vulkan.getDevice(), 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking

		vkResetFences(vulkan.getDevice(), 1, &fd->Fence);
	}
	{
		vkResetCommandPool(vulkan.getDevice(), fd->CommandPool, 0);
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(fd->CommandBuffer, &info);
	}
	{
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = wd.RenderPass;
		info.framebuffer = fd->Framebuffer;
		info.renderArea.extent.width = wd.Width;
		info.renderArea.extent.height = wd.Height;
		info.clearValueCount = 1;
		info.pClearValues = &wd.ClearValue;
		vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	// Record Imgui Draw Data and draw funcs into command buffer
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), fd->CommandBuffer);

	// Submit command buffer
	vkCmdEndRenderPass(fd->CommandBuffer);
	{
		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &image_acquired_semaphore;
		info.pWaitDstStageMask = &wait_stage;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &fd->CommandBuffer;
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = &render_complete_semaphore;

		vkEndCommandBuffer(fd->CommandBuffer);
		vkQueueSubmit(vulkan.getGraphicsQueue(), 1, &info, fd->Fence);
	}

	VkPresentInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.waitSemaphoreCount = 1;
	info.pWaitSemaphores = &render_complete_semaphore;
	info.swapchainCount = 1;
	info.pSwapchains = &wd.Swapchain;
	info.pImageIndices = &wd.FrameIndex;
	vkQueuePresentKHR(vulkan.getGraphicsQueue(), &info);
	wd.SemaphoreIndex = (wd.SemaphoreIndex + 1) % wd.ImageCount; // Now we can use the next set of semaphores
}

void WrapperImgui::destroy()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void WrapperImgui::uploadFont(VulkanContext &vulkan)
{
	{
		// Use any command queue
		VkCommandPool command_pool = wd.Frames[wd.FrameIndex].CommandPool;
		VkCommandBuffer command_buffer = wd.Frames[wd.FrameIndex].CommandBuffer;

		vkResetCommandPool(vulkan.getDevice(), command_pool, 0);
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
		
		vkQueueSubmit(vulkan.getGraphicsQueue(), 1, &end_info, VK_NULL_HANDLE);
		
		vkDeviceWaitIdle(vulkan.getDevice());
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}