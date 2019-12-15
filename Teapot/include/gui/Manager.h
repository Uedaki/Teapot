#pragma once

#include <vulkan/vulkan.h>

#include <functional>
#include <vector>

#include "imgui.h"
#include "imgui/ctm_imgui_impl_vulkan.h"

#include "gui/Widget.h"

namespace teapot
{
	namespace gui
	{
		class Manager
		{
		public:
			void init();
			void destroy();
			
			template <typename T, typename ...Args>
			void addWidget(Args... args)
			{
				widgets.emplace_back(std::make_unique<T>(args...));
			}

			void drawWidgets();
			void render(VkCommandBuffer &cmd);

			inline uint32_t getImageCount() const { return (wd.ImageCount); }
			inline VkDescriptorPool &getDescriptorPool() { return (descriptorPool); }

		private:
			ImGui_ImplVulkanH_Window wd = {};

			VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

			std::vector<std::unique_ptr<Widget>> widgets;

			void uploadFont();
		};
	}
}