#include "gui/Manager.h"

#include "imgui_impl_glfw.h"

#include "Application.h"
#include "log.h"
#include "Profiler.h"
#include "vulkan/Context.h"

void teapot::gui::Manager::init()
{
	PROFILE_FUNCTION("GUI");

	Application &app = Application::get();
	vk::Context &vulkan = app.getVulkan();

	int w, h;
	glfwGetFramebufferSize(app.getWindow(), &w, &h);

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
	VK_CHECK_RESULT(vkCreateDescriptorPool(vulkan.device, &poolInfo, vulkan.allocator, &descriptorPool));

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(app.getWindow(), true);

	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = vulkan.instance;
	initInfo.PhysicalDevice = vulkan.physicalDevice;
	initInfo.Device = vulkan.device;
	initInfo.QueueFamily = vulkan.queue.graphicsFamily;
	initInfo.Queue = vulkan.queue.graphics;
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = descriptorPool;
	initInfo.Allocator = vulkan.allocator;
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = vulkan.swapchainInfo.imgCount;
	ImGui_ImplVulkan_Init(&initInfo, vulkan.renderPass);

	uploadFont();

	LOG_MSG("GUI manager loaded");
}

void teapot::gui::Manager::destroy()
{
	PROFILE_FUNCTION("GUI");

	Application &app = Application::get();
	vk::Context &vulkan = app.getVulkan();

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	vkDestroyDescriptorPool(vulkan.device, descriptorPool, vulkan.allocator);
}

void teapot::gui::Manager::drawWidgets()
{
	PROFILE_FUNCTION("GUI");

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	for (auto &widget : widgets)
	{
		widget->draw();
	}

	ImGui::Render();
}

void teapot::gui::Manager::render(VkCommandBuffer &commandBuffer)
{
	PROFILE_FUNCTION("GUI");

	Application &app = Application::get();
	vk::Context &vulkan = app.getVulkan();

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = vulkan.renderPass;
	renderPassInfo.framebuffer = vulkan.swapchainInfo.frames[vulkan.swapchainInfo.currImg];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = vulkan.config.extent;

	VkClearValue clearColor = { 0.39f, 0.40f, 0.38f, 1.0f };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	VkDeviceSize p = 0;
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

	vkCmdEndRenderPass(commandBuffer);
}

void teapot::gui::Manager::uploadFont()
{
	vk::Context &vulkan = Application::get().getVulkan();
	vk::Command& command = Application::get().getMainCommand();

	VkCommandBuffer &commandBuffer = command.recordNextBuffer();
	ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
	command.submit();
	vkDeviceWaitIdle(vulkan.device);
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}