#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "imgui/ImguiModule.h"
#include "Mesh.h"
#include "PipelinePool.h"
#include "Rasterizer.h"
#include "RasterizerData.h"
#include "RasterizerImage.h"

namespace teapot
{
	enum class EditMode : uint8_t
	{
		NONE,
		FACE,
		EDGE,
		VERTEX
	};

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

		void render(teapot::Mesh &mesh);

		bool needToBeResized(uint32_t newWidth, uint32_t newHeight) const;

		void changeMode(EditMode mode);

		VkSemaphore &getCurrentSignalSemaphore() { return (semaphores[currentImage]); };
		VkDescriptorSetLayout getDescriptorSetLayout() { return (pipelinePool.getDescriptorSetLayout()); };
		VkDescriptorSet &getOutputDescriptorSet() { return (outDescriptorSets[currentImage]); };

	private:
		ctm::VkCore &vCore;

		PipelinePool pipelinePool;
		Pipeline mainPipeline;
		Pipeline secondaryPipeline;

		teapot::RasterizerImage rasterizerImg = {};

		uint32_t currentImage = 0;

		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

		std::vector<VkBuffer> buffers;
		std::vector<VkDeviceMemory> bufferMemories;

		VkDescriptorSetLayout outDescriptorSetLayout = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> outDescriptorSets;

		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<VkFence> fences;
		std::vector<VkSemaphore> semaphores;

		void createCommandPool();

		void createOutputDescriptorLayout();
		void createOutputDescriptorSet(VkDescriptorSet &outDescriptorSet, VkDescriptorPool &targetDescriptorPool, VkImageView &imageView);
		void createSyncObj(VkSemaphore &semaphore, VkFence &fence);

		void recordCommandBuffer(VkCommandBuffer &commandBuffer, teapot::Mesh &mesh, VkImage &image, VkFramebuffer &frame, uint32_t imageIdx);
	};
}