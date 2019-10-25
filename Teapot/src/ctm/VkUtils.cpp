#include "ctm/VkUtils.h"

#include <stdexcept>

#include "ctm/VkCore.h"

uint32_t ctm::VkUtils::findMemoryType(ctm::VkCore &core, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(core.physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return (i);
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}