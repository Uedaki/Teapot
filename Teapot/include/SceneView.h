#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "imgui/ImguiModule.h"
#include "Mesh.h"
#include "Rasterizer.h"

namespace teapot
{
	class SceneView
	{
		class SceneGui : public teapot::ImguiModule
		{
			void drawGui() override;
		};

	public:
		SceneView(ctm::VkCore &vCore);
		~SceneView() = default;

		void init(teapot::Mesh &mesh, VkDescriptorPool &targetDescriptorPool, uint32_t width, uint32_t height, uint32_t imageCount);
		void destroy();

		void render();

		bool needToBeResized(uint32_t newWidth, uint32_t newHeight) const;

		VkSemaphore &getCurrentSignalSemaphore() { return (semaphores[currentImage]); };
		VkDescriptorSetLayout &getDescriptorSetLayout() { return (rasterizer.descriptorSetLayout); };
		VkDescriptorSet &getOutputDescriptorSet() { return (outDescriptorSets[currentImage]); };

	private:
		ctm::VkCore &vCore;
		teapot::Rasterizer rasterizer = {};

		uint32_t currentImage = 0;

		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

		std::vector<VkFence> fences;
		std::vector<VkSemaphore> semaphores;

		std::vector<VkBuffer> buffers;
		std::vector<VkDeviceMemory> bufferMemories;

		VkDescriptorSetLayout outDescriptorSetLayout = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> outDescriptorSets;

		SceneGui gui;

		void createOutputDescriptorLayout();
		void createOutputDescriptorSet(VkDescriptorSet &outDescriptorSet, VkDescriptorPool &targetDescriptorPool, VkImageView &imageView);
		void createSyncObj(VkSemaphore &semaphore, VkFence &fence);

		void recordCommandBuffer(VkCommandBuffer &commandBuffer, teapot::Mesh &mesh, VkImage &image, VkFramebuffer &frame, uint32_t imageIdx);
	};
}