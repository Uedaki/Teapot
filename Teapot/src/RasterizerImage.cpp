#include "RasterizerImage.h"

#include "ctm/VkCore.h"
#include "ctm/VkUtils.h"
#include "profiler/Profiler.h"
#include "RasterizerData.h"

namespace
{
	void createImageSampler(teapot::RasterizerImage &img, ctm::VkCore &core)
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		if (vkCreateSampler(core.device, &samplerInfo, core.allocator, &img.imageSampler) != VK_SUCCESS)
			throw std::runtime_error("Failed to create VkSampler");
	}

	void createImage(teapot::RasterizerImage &data, ctm::VkCore &core, VkImage &image)
	{
		uint32_t familyIndice = core.queue.graphicsIdx;
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageInfo.extent.width = data.extent.width;
		imageInfo.extent.height = data.extent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.queueFamilyIndexCount = 1;
		imageInfo.pQueueFamilyIndices = &familyIndice;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if (vkCreateImage(core.device, &imageInfo, core.allocator, &image) != VK_SUCCESS)
			throw std::runtime_error("Failed to create VkImage");
	}

	void allocateImage(ctm::VkCore &core, VkDeviceMemory &memory, VkImage &image)
	{
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(core.device, image, &memRequirements);

		VkMemoryAllocateInfo memoryInfo = {};
		memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryInfo.allocationSize = memRequirements.size;
		memoryInfo.memoryTypeIndex = ctm::VkUtils::findMemoryType(core, memRequirements.memoryTypeBits,
																  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		if (vkAllocateMemory(core.device, &memoryInfo, core.allocator, &memory) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate memory");
		if (vkBindImageMemory(core.device, image, memory, 0) != VK_SUCCESS)
			throw std::runtime_error("Failed to bind image memory");
	}

	void createImageView(ctm::VkCore &core, VkImageView &imageView, VkImage &image)
	{
		VkImageViewCreateInfo imageViewInfo = {};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.image = image;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;
		if (vkCreateImageView(core.device, &imageViewInfo, core.allocator, &imageView) != VK_SUCCESS)
			throw std::runtime_error("Failed to create VkImage");
	}

	void createFrameBuffer(ctm::VkCore &core, VkRenderPass &renderPass, VkExtent2D &extent, VkFramebuffer &frame, VkImageView &imageView)
	{
		VkFramebufferCreateInfo frameInfo = {};
		frameInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameInfo.renderPass = renderPass;
		frameInfo.attachmentCount = 1;
		frameInfo.pAttachments = &imageView;
		frameInfo.width = extent.width;
		frameInfo.height = extent.height;
		frameInfo.layers = 1;
		if (vkCreateFramebuffer(core.device, &frameInfo, core.allocator, &frame) != VK_SUCCESS)
			throw std::runtime_error("Failed to create VkFrameBuffer");
	}
}

void teapot::RasterizerImage::init(ctm::VkCore &core, VkRenderPass renderPass, VkExtent2D newExtent, uint32_t newImageCount)
{
	PROFILE_FUNCTION("blop");

	extent = newExtent;
	imageCount = newImageCount;

	images.resize(imageCount);
	imageMemories.resize(imageCount);
	imageViews.resize(imageCount);
	frameBuffers.resize(imageCount);

	createImageSampler(*this, core);

	for (uint32_t i = 0; i < imageCount; i++)
	{
		createImage(*this, core, images[i]);
		allocateImage(core, imageMemories[i], images[i]);
		createImageView(core, imageViews[i], images[i]);
		createFrameBuffer(core, renderPass, extent, frameBuffers[i], imageViews[i]);
	}
}

void teapot::RasterizerImage::destroy(ctm::VkCore &core)
{
	PROFILE_FUNCTION("blop");

	vkDestroySampler(core.device, imageSampler, core.allocator);

	for (uint32_t i = 0; i < frameBuffers.size(); i++)
	{
		vkDestroyFramebuffer(core.device, frameBuffers[i], core.allocator);
		vkDestroyImageView(core.device, imageViews[i], core.allocator);
		vkFreeMemory(core.device, imageMemories[i], core.allocator);
		vkDestroyImage(core.device, images[i], core.allocator);
	}
}