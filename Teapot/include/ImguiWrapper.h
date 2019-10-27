#pragma once

#include <vulkan/vulkan.h>

#include "imgui.h"
#include "imgui/ctm_imgui_impl_vulkan.h"

struct GLFWwindow;
namespace ctm { struct VkCore; }
namespace teapot { struct SceneView; }

namespace teapot
{
	struct ImguiWrapper
	{
		VkDescriptorPool descriptorPool;
		ImGui_ImplVulkanH_Window wd = {};

		static void init(teapot::ImguiWrapper &wrapper, GLFWwindow *win, ctm::VkCore &core);
		static void destroy(teapot::ImguiWrapper &wrapper, ctm::VkCore &vCore);
		static void newFrame(teapot::ImguiWrapper &wrapper);
		static void rebuildSwapChain(teapot::ImguiWrapper &wrapper, GLFWwindow *win, ctm::VkCore &vCore);
		static void render(teapot::ImguiWrapper &wrapper, VkSemaphore sem, ctm::VkCore &vCore);
	};
}