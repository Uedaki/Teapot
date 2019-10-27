#pragma once

#include <Vulkan/vulkan.h>

#include <glm/glm.hpp>

#include <array>

namespace ctm
{
	struct VkVertex
	{
		glm::vec3 pos;
		glm::vec3 color;

		static VkVertexInputBindingDescription getBindingDescription();
		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
	};
}