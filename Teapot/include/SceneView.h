#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "ctm/VkRasterizer.h"
#include "Mesh.h"
#include "imgui/ImguiModule.h"

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
		VkDescriptorSet &getOutputDescriptorSet() { return (outDescriptorSets[currentImage]); };

	private:
		ctm::VkCore &vCore;
		ctm::VkRasterizer vRasterizer = {};

		uint32_t currentImage = 0;

		VkSampler imageSampler = VK_NULL_HANDLE;
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

		std::vector<VkDescriptorSet> descriptorSets;
		std::vector<VkFence> fences;
		std::vector<VkSemaphore> semaphores;

		VkDescriptorSetLayout outDescriptorSetLayout = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> outDescriptorSets;

		SceneGui gui;

		void createImageSampler();
		void createDescriptorPool();
		void createOutputDescriptorLayout();
		void createDescriptorSet(VkDescriptorSet &descriptorSet, teapot::Mesh &mesh);
		void createOutputDescriptorSet(VkDescriptorSet &outDescriptorSet, VkDescriptorPool &targetDescriptorPool, VkImageView &imageView);
		void createSyncObj(VkSemaphore &semaphore, VkFence &fence);

		void recordCommandBuffer(VkCommandBuffer &commandBuffer, teapot::Mesh &mesh, VkImage &image, VkFramebuffer &frame, VkDescriptorSet &descriptorSet);
	};
}