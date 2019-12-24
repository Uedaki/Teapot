#pragma once

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <vector>

#include "DisplayMode.h"
#include "vulkan/Pipeline.h"

namespace teapot
{
	namespace vk
	{
		class SceneEditor
		{
		public:
			struct SceneView
			{
				glm::vec3 pos = glm::vec3(glm::pi<float>());
				glm::vec3 dir = glm::vec3(glm::pi<float>());
				glm::vec3 up = glm::vec3(glm::pi<float>());
				glm::mat4 view;
				glm::mat4 proj;

				VkBuffer cameraBuffer = VK_NULL_HANDLE;
				VkDeviceMemory cameraBufferMemory = VK_NULL_HANDLE;

				std::vector<VkImage> images;
				std::vector<VkDeviceMemory> imageMemories;
				std::vector<VkImageView> imageViews;
				std::vector<VkFramebuffer> frameBuffers;
				
				std::vector<VkDescriptorSet> outDescriptorSet;

				VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
				std::vector<VkDescriptorSet> descriptorSet;
			};

			void init();
			void destroy();

			void renderViews(VkCommandBuffer &commandBuffer);
			void pushTransform(SceneView &view);

			void updateExtent(uint32_t width, uint32_t height);
			void changeOverlay(DisplayMode newMode);
			void deleteOverlay();

			VkDescriptorSet &getDescriptorSet();

			inline DisplayMode getCurrentDisplayMode() const { return (mode); }
			inline SceneView &getSceneView() { return (sceneView); }

		private:
			VkExtent2D extent = {0, 0};
			DisplayMode mode = DisplayMode::FACE;

			VkRenderPass renderPass;
			VkDescriptorSetLayout descriptorSetLayout;
			Pipeline mainPipeline;
			Pipeline overlayPipeline;

			VkSampler outImageSampler;
			VkDescriptorSetLayout outDescriptorSetLayout;

			SceneView sceneView;

			void createRenderPass();
			void createDescriptorSetLayout();
			void createOutputImageSampler();
			void createOutputDescriptorSetLayout();

			void createSceneView(SceneView &view);
			void createCameraBuffer(SceneView &view);
			void createImage(VkImage &image);
			void allocateImage(VkDeviceMemory &memory, VkImage &image);
			void createImageView(VkImageView &imageView, VkImage &image);
			void createFrameBuffer(VkFramebuffer &frame, VkImageView &imageView, VkRenderPass &renderPass);
			void createOutputDescriptorSet(VkDescriptorSet &outDescriptorSet, VkImageView &imageView);
			void createDescriptorPool(SceneView &view);
			void createDescriptorSet(teapot::vk::SceneEditor::SceneView &view);
			void destroySceneView(SceneView &view);
		};
	}
}