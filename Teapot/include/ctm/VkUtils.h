#pragma once

#include <vulkan/vulkan.h>

#include <stdint.h>

namespace ctm { struct VkCore; }

namespace ctm
{
	class VkUtils
	{
	public:
		static uint32_t findMemoryType(ctm::VkCore &core, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	};
}